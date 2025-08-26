#include "../include/GeoWork.h"

#include <QDateTime>
#include <QDebug>
#include <QFile>
#include <QGeoCoordinate>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QSet>
#include <QUrl>
#include <QUrlQuery>
#include <QVariant>
#include <QVector>

// ==== QGC headers to access active vehicle ====
#include "MultiVehicleManager.h"
#include "QGCApplication.h"
#include "QGCToolbox.h"
#include "Vehicle.h"

// ===== REST endpoints =====
static const char* kUrlSession        = "https://api.geowork.mobis1.com/vehicles-reporting/session";
static const char* kUrlState          = "https://api.geowork.mobis1.com/vehicles-reporting/project-marker-state";
static const char* kUrlMarker         = "https://api.geowork.mobis1.com/vehicles-reporting/markers/create";
static const char* kUrlReportLocation = "https://api.geowork.mobis1.com/vehicles-reporting/report-location";

// ---------------------------------------------------------

GeoWork::GeoWork(QObject* parent)
    : QObject { parent } {
    loadSettings();
}

QByteArray GeoWork::authHeader() const {
    if (m_bearerToken.isEmpty()) {
        return QByteArray();
    }
    QString t = m_bearerToken.trimmed();
    if (!t.startsWith(QStringLiteral("Bearer "))) {
        t = QStringLiteral("Bearer ") + t;
    }
    return t.toUtf8();
}

// Read active vehicle coordinate via QGC singletons
bool GeoWork::getActiveVehicleCoordinate(double& latOut, double& lonOut, double& altOut) const {
    latOut = lonOut = altOut = 0.0;

    if (!qgcApp() || !qgcApp()->toolbox()) {
        qWarning() << "[GeoWork] qgcApp/toolbox not ready";
        return false;
    }

    MultiVehicleManager* mvm { qgcApp()->toolbox()->multiVehicleManager() };
    if (!mvm) {
        qWarning() << "[GeoWork] MultiVehicleManager not available";
        return false;
    }

    Vehicle* vehicle { mvm->activeVehicle() };
    if (!vehicle) {
        qWarning() << "[GeoWork] No active vehicle";
        return false;
    }

    const QGeoCoordinate coord { vehicle->coordinate() };
    if (!coord.isValid()) {
        qWarning() << "[GeoWork] Active vehicle coordinate invalid";
        return false;
    }

    latOut = coord.latitude();
    lonOut = coord.longitude();
    altOut = coord.altitude(); // may be NaN if not provided

    return true;
}

// ======================= Settings =========================

void GeoWork::setDeviceName(const QString& name) {
    const QString n = name.trimmed();
    if (m_deviceName == n)
        return;

    m_deviceName = n;
    emit deviceNameChanged();
    saveSettings();
}

void GeoWork::setBearerToken(const QString& token) {
    QString t = token.trimmed();
    if (!t.startsWith(QStringLiteral("Bearer "))) {
        t = QStringLiteral("Bearer ") + t;
    }

    if (m_bearerToken == t)
        return;

    m_bearerToken = t;
    emit bearerTokenChanged();

    // Keep a basic presence-based status until validateToken() sets a final state
    TokenStatus newStatus { m_bearerToken.isEmpty() ? TokenStatus::None : m_tokenStatus };
    if (newStatus != m_tokenStatus) {
        m_tokenStatus = newStatus;
        emit tokenStatusChanged();
    }

    saveSettings();
}

bool GeoWork::setBearerTokenFromFile(const QString& fileUrl) {
    // Accept both file:// URL and plain path
    QString local { QUrl(fileUrl).isValid() ? QUrl(fileUrl).toLocalFile() : QString() };
    QFile   f { local.isEmpty() ? fileUrl : local };
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[GeoWork] Cannot open token file:" << fileUrl << f.errorString();
        return false;
    }

    QString txt { QString::fromUtf8(f.readAll()).trimmed() };
    f.close();

    // Strip UTF-8 BOM if present
    if (!txt.isEmpty() && txt.at(0) == QChar(0xFEFF)) {
        txt.remove(0, 1);
    }

    if (txt.isEmpty()) {
        qWarning() << "[GeoWork] Token file is empty:" << fileUrl;
        if (m_tokenStatus != TokenStatus::None) {
            m_tokenStatus = TokenStatus::None;
            emit tokenStatusChanged();
        }

        return false;
    }

    setBearerToken(txt); // normalizes + persists
    qInfo() << "[GeoWork] Token loaded from" << fileUrl << "len:" << txt.size();

    validateToken(); // set tokenStatus (0/1/2)

    return true;
}

void GeoWork::validateToken() {
    // TODO: Replace with a real server validation endpoint if available.
    TokenStatus newStatus { TokenStatus::None };

    if (m_bearerToken.isEmpty()) {
        newStatus = TokenStatus::None;
    } else if (m_bearerToken.startsWith(QStringLiteral("Bearer ")) && m_bearerToken.size() > 40) {
        newStatus = TokenStatus::Valid;
    } else {
        newStatus = TokenStatus::Invalid;
    }
    if (newStatus != m_tokenStatus) {
        m_tokenStatus = newStatus;
        emit tokenStatusChanged();
    }
    saveSettings();
}

void GeoWork::saveSettings() {
    m_settings.setValue(QStringLiteral("deviceName"), m_deviceName);
    m_settings.setValue(QStringLiteral("bearerToken"), m_bearerToken);
    m_settings.setValue(QStringLiteral("tokenStatus"), static_cast<int>(m_tokenStatus));
    m_settings.sync();
}

void GeoWork::loadSettings() {
    // First used for checking whether settings have correctly been parsed.
    // Afterward, w
    bool any { false };

    const QString     dn { m_settings.value(QStringLiteral("deviceName")).toString() };
    const QString     tk { m_settings.value(QStringLiteral("bearerToken")).toString() };
    const TokenStatus st { m_settings.value(QStringLiteral("tokenStatus"), 0).toUInt() };

    if (m_deviceName != dn) {
        m_deviceName = dn;
        emit deviceNameChanged();
        any = true;
    }

    if (m_bearerToken != tk) {
        m_bearerToken = tk;
        emit bearerTokenChanged();
        any = true;
    }

    if (m_tokenStatus != st) {
        m_tokenStatus = st;
        emit tokenStatusChanged();
        any = true;
    }

    if (any) {
        qInfo() << "[GeoWork] Settings loaded. deviceName=" << m_deviceName
                << " tokenLen=" << m_bearerToken.size()
                << " tokenStatus=" << static_cast<std::uint32_t>(m_tokenStatus);
    }
}

// ======================= Network ops ======================

void GeoWork::checkActiveTaskAndFetchState(const QString& stateName) {
    // Cache device name for later use
    if (m_deviceName != stateName) {
        m_deviceName = stateName.trimmed();
        emit deviceNameChanged();
        saveSettings();
    }

    if (m_bearerToken.isEmpty()) {
        qWarning() << "[GeoWork] No bearer token set. Open Geowork Settings and load a token file.";
        if (m_tokenStatus != TokenStatus::None) {
            m_tokenStatus = TokenStatus::None;
            emit tokenStatusChanged();
        }

        return;
    }

    // 1. Check the current session for an active task.
    QNetworkRequest req { QUrl(QString::fromUtf8(kUrlSession)) };
    req.setRawHeader("Authorization", authHeader());
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply* replySession { m_nam.get(req) };

    connect(replySession, &QNetworkReply::finished, this, [this, replySession]() {
        const QByteArray body = replySession->readAll();

        if (replySession->error() != QNetworkReply::NoError) {
            qWarning() << "[GeoWork] /session error:" << replySession->errorString()
                       << "payload:" << body;
            replySession->deleteLater();
            return;
        }

        QJsonParseError     jerr;
        const QJsonDocument doc = QJsonDocument::fromJson(body, &jerr);

        if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "[GeoWork] /session JSON parse error:" << jerr.errorString();
            replySession->deleteLater();

            return;
        }

        const QJsonObject root { doc.object() };
        const QJsonObject data { root.value(QStringLiteral("data")).toObject() };
        const QJsonObject activeTask { data.value(QStringLiteral("activeTask")).toObject() };

        if (activeTask.isEmpty()) {
            qInfo() << "[GeoWork] No active task.";
            m_projectId.clear();
            emit projectIdChanged();
        } else {
            const QString newProjectId = activeTask.value(QStringLiteral("projectId")).toString();
            if (m_projectId != newProjectId) {
                m_projectId = newProjectId;
                emit projectIdChanged();
            }

            qInfo() << "[GeoWork] Active task projectId:" << m_projectId;
        }

        replySession->deleteLater();

        // 2) Resolve state for this device/name
        QNetworkRequest reqState(QUrl(QString::fromUtf8(kUrlState)));
        reqState.setRawHeader("Authorization", authHeader());
        reqState.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

        QJsonObject payloadState;
        payloadState.insert(QStringLiteral("name"), m_deviceName);

        const QByteArray jsonBody { QJsonDocument(payloadState).toJson(QJsonDocument::Compact) };
        QNetworkReply*   replyState { m_nam.post(reqState, jsonBody) };

        connect(replyState, &QNetworkReply::finished, this, [this, replyState]() {
            const QByteArray body2 { replyState->readAll() };

            if (replyState->error() != QNetworkReply::NoError) {
                qWarning() << "[GeoWork] /project-marker-state error:" << replyState->errorString()
                           << "payload:" << body2;
                replyState->deleteLater();

                return;
            }

            QJsonParseError     jerr2;
            const QJsonDocument doc2 { QJsonDocument::fromJson(body2, &jerr2) };
            if (jerr2.error != QJsonParseError::NoError || !doc2.isObject()) {
                qWarning() << "[GeoWork] /project-marker-state JSON parse error:" << jerr2.errorString();
                replyState->deleteLater();

                return;
            }

            const QJsonObject root2 { doc2.object() };
            const QJsonObject data2 { root2.value(QStringLiteral("data")).toObject() };
            const QJsonObject state { data2.value(QStringLiteral("state")).toObject() };

            const QString newStateId { state.value(QStringLiteral("id")).toString() };

            if (m_stateId != newStateId) {
                m_stateId = newStateId;
                emit stateIdChanged();
            }

            if (m_stateId.isEmpty()) {
                qWarning() << "[GeoWork] state.id missing in response";
            } else {
                qInfo() << "[GeoWork] stateId:" << m_stateId;
            }

            replyState->deleteLater();
        });
    });
}

void GeoWork::createMarker() {
    if (m_bearerToken.isEmpty()) {
        qWarning() << "[GeoWork] createMarker(): No bearer token set.";
        return;
    }
    if (m_stateId.isEmpty()) {
        qWarning() << "[GeoWork] createMarker(): stateId is empty. Run checkActiveTaskAndFetchState() first.";
        return;
    }

    double lat = 0.0, lon = 0.0, alt = 0.0;
    if (!getActiveVehicleCoordinate(lat, lon, alt)) {
        qWarning() << "[GeoWork] createMarker(): No valid GPS from active vehicle.";
        return;
    }

    // Build GeoJSON point: [lon, lat] (note order)
    QJsonObject geom;
    geom.insert(QStringLiteral("type"), QStringLiteral("Point"));
    QJsonArray coords;
    coords.append(lon);
    coords.append(lat);
    geom.insert(QStringLiteral("coordinates"), coords);

    // Payload
    QJsonObject payload;
    payload.insert(QStringLiteral("stateId"), m_stateId);
    payload.insert(QStringLiteral("geometry"), geom);

    QNetworkRequest req(QUrl(QString::fromUtf8(kUrlMarker)));
    req.setRawHeader("Authorization", authHeader());
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const QByteArray body  = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QNetworkReply*   reply = m_nam.post(req, body);

    connect(reply, &QNetworkReply::finished, this, [reply, lat, lon]() {
        const QByteArray resp = reply->readAll();

        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "[GeoWork] /markers/create error:" << reply->errorString()
                       << "payload:" << resp;
            reply->deleteLater();
            return;
        }

        QJsonParseError     jerr {};
        const QJsonDocument doc = QJsonDocument::fromJson(resp, &jerr);
        if (jerr.error != QJsonParseError::NoError || !doc.isObject()) {
            qWarning() << "[GeoWork] /markers/create JSON parse error:" << jerr.errorString();
            reply->deleteLater();
            return;
        }

        const QJsonObject root     = doc.object();
        const QJsonObject data     = root.value(QStringLiteral("data")).toObject();
        const QString     markerId = data.value(QStringLiteral("id")).toString();

        if (!markerId.isEmpty()) {
            qInfo() << "[GeoWork] Marker created at" << lat << "," << lon << " id:" << markerId;
        } else {
            qInfo() << "[GeoWork] Marker created (no id field)";
        }

        reply->deleteLater();
    });
}

// Safely get a nested Fact rawValue: vehicle.<group>.<fact>.rawValue
// Returns {true, value} on success; {false, 0} if any link in the chain is missing.
static std::pair<bool, double> getFactRawDouble(QObject* groupObj, const char* factName) {
    if (!groupObj)
        return { false, 0.0 };
    QVariant factVar = groupObj->property(factName);
    QObject* factObj = factVar.value<QObject*>();
    if (!factObj)
        return { false, 0.0 };
    const QVariant raw = factObj->property("rawValue");
    if (!raw.isValid())
        return { false, 0.0 };
    return { true, raw.toDouble() };
}

// Convenience: pull a QObject* sub-object by name from vehicle (e.g., "gps", "battery", …)
static QObject* vehicleSubObject(QObject* v, const char* name) {
    if (!v)
        return nullptr;
    QVariant sub = v->property(name);
    return sub.value<QObject*>();
}

// ---- Helpers for robust fact lookup ----
static std::pair<bool, double> getAnyPressure(QObject* groupObj) {
    if (!groupObj)
        return { false, 0.0 };
    const char* names[] = { "absPressure", "absolutePressure", "baroPressure", "pressure", "staticPressure", "pressAbs", "ambPressure" };
    for (const char* n : names) {
        auto res = getFactRawDouble(groupObj, n);
        if (res.first)
            return res;
    }
    return { false, 0.0 };
}

static QVector<QObject*> collectBatteryGroups(QObject* vehicle) {
    QVector<QObject*> out;
    if (!vehicle)
        return out;
    const char* names[] = { "battery", "battery1", "battery2", "battery3" };
    for (const char* n : names) {
        QVariant v = vehicle->property(n);
        if (v.isValid()) {
            if (QObject* o = v.value<QObject*>())
                out.append(o);
        }
    }
    // Also try "batteries" list model, if present
    QVariant lstVar = vehicle->property("batteries");
    if (lstVar.isValid()) {
        if (QObject* listObj = lstVar.value<QObject*>()) {
            const auto children = listObj->findChildren<QObject*>(QString(), Qt::FindDirectChildrenOnly);
            for (QObject* c : children)
                if (c)
                    out.append(c);
        }
    }
    // Deduplicate and clamp to 3
    QSet<QObject*>    seen;
    QVector<QObject*> dedup;
    for (QObject* o : out)
        if (o && !seen.contains(o)) {
            seen.insert(o);
            dedup.append(o);
        }
    while (dedup.size() > 3)
        dedup.removeLast();
    return dedup;
}
void GeoWork::reportLocation() {
    // --- Guards ---
    if (m_bearerToken.isEmpty()) {
        qWarning() << "[GeoWork] reportLocation(): No bearer token set.";
        return;
    }
    if (!qgcApp() || !qgcApp()->toolbox()) {
        qWarning() << "[GeoWork] reportLocation(): App/toolbox not ready.";
        return;
    }
    MultiVehicleManager* mvm     = qgcApp()->toolbox()->multiVehicleManager();
    Vehicle*             vehicle = mvm ? mvm->activeVehicle() : nullptr;
    if (!vehicle) {
        qWarning() << "[GeoWork] reportLocation(): No active vehicle.";
        return;
    }

    // --- GPS position (required) ---
    const QGeoCoordinate coord = vehicle->coordinate();
    if (!coord.isValid()) {
        qWarning() << "[GeoWork] reportLocation(): Active vehicle coordinate invalid.";
        return;
    }
    const double lat = coord.latitude();
    const double lon = coord.longitude();
    const double alt = coord.altitude(); // may be NaN; fine in JSON as string

    // --- Timestamp with timezone, ISO 8601 (like Python astimezone().isoformat()) ---
    const QString timestamp = QDateTime::currentDateTime().toString(Qt::ISODateWithMs);

    // --- Pull additional telemetry via generic Fact access (defensive) ---
    // gps.count
    QObject* gpsObj        = vehicleSubObject(vehicle, "gps");
    int      gpsSatellites = 0;
    {
        auto [ok, v] = getFactRawDouble(gpsObj, "count");
        if (ok)
            gpsSatellites = static_cast<int>(v);
    }

    // groundSpeed (Vehicle usually exposes a Q_PROPERTY "groundSpeed" in m/s)
    double groundSpeed = vehicle->property("groundSpeed").toDouble();

    // heading (deg)
    double heading = vehicle->property("heading").toDouble();

    // heading (deg)
    // battery facts (supports up to 3 packs)
    QVector<QObject*> batGroups   = collectBatteryGroups(vehicle);
    double            batVoltage  = qQNaN();
    double            batCurrent  = qQNaN();
    double            batConsumed = qQNaN(); // QGC usually exposes "mahConsumed" (mAh)
    QList<double>     batVoltages, batCurrents, batMah;

    for (int i = 0; i < batGroups.size() && i < 3; ++i) {
        QObject* b     = batGroups[i];
        auto [okV, vV] = getFactRawDouble(b, "voltage");
        auto [okC, vC] = getFactRawDouble(b, "current");
        auto [okM, vM] = getFactRawDouble(b, "mahConsumed");
        if (!okM) {
            auto alt = getFactRawDouble(b, "mah_consumed");
            if (alt.first) {
                okM = true;
                vM  = alt.second;
            }
        }
        if (okV)
            batVoltages.append(vV);
        if (okC)
            batCurrents.append(vC);
        if (okM)
            batMah.append(vM);
    }
    if (!batVoltages.isEmpty())
        batVoltage = batVoltages.first();
    if (!batCurrents.isEmpty())
        batCurrent = batCurrents.first();
    if (!batMah.isEmpty())
        batConsumed = batMah.first();

    // absolute pressure (try several groups and names)
    double pressAbs = qQNaN();
    {
        QObject* envObj     = vehicleSubObject(vehicle, "environment");
        QObject* airObj     = vehicleSubObject(vehicle, "air");
        QObject* sensorsObj = vehicleSubObject(vehicle, "sensors");
        auto     p1         = getAnyPressure(envObj);
        auto     p2         = getAnyPressure(airObj);
        auto     p3         = getAnyPressure(sensorsObj);
        if (p1.first)
            pressAbs = p1.second;
        else if (p2.first)
            pressAbs = p2.second;
        else if (p3.first)
            pressAbs = p3.second;
    }

    // --- Build payload (same structure as your Python) ---
    QJsonArray meta;

    // satellites
    meta.append(QJsonObject {
        { "key", "satellites" },
        { "value", QString::number(gpsSatellites) } //,        { "unit",  "number" }
    });

    // ground speed (m/s)
    if (std::isfinite(groundSpeed)) {
        meta.append(QJsonObject {
            { "key", "ground_speed" },
            { "value", QString::number(groundSpeed, 'f', 2) },
            { "unit", "m/s" } });
    }

    // heading (deg)
    if (std::isfinite(heading)) {
        meta.append(QJsonObject {
            { "key", "heading" },
            { "value", QString::number(heading) },
            { "unit", "deg" } });
    }

    // battery voltage (V)
    if (std::isfinite(batVoltage)) {
        meta.append(QJsonObject {
            { "key", "bat_voltage" },
            { "value", QString::number(batVoltage) },
            { "unit", "V" } });
    }

    // battery current (A)
    if (std::isfinite(batCurrent)) {
        meta.append(QJsonObject {
            { "key", "bat_current" },
            { "value", QString::number(batCurrent) },
            { "unit", "A" } });
    }

    // total consumption – QGC exposes mAh; your Python key was "current_consumed"
    if (std::isfinite(batConsumed)) {
        meta.append(QJsonObject {
            { "key", "current_consumed" },
            { "value", QString::number(batConsumed) },
            { "unit", "mA" } // clarify unit; change to "A" / "Ah" if your backend expects that
        });
        // per-pack battery values (up to 3)
        for (int i = 0; i < batVoltages.size() && i < 3; ++i) {
            meta.append(QJsonObject {
                { "key", QString("bat%1_voltage").arg(i + 1) },
                { "value", QString::number(batVoltages[i]) },
                { "unit", "V" } });
        }
        for (int i = 0; i < batCurrents.size() && i < 3; ++i) {
            meta.append(QJsonObject {
                { "key", QString("bat%1_current").arg(i + 1) },
                { "value", QString::number(batCurrents[i]) },
                { "unit", "A" } });
        }
        for (int i = 0; i < batMah.size() && i < 3; ++i) {
            meta.append(QJsonObject {
                { "key", QString("bat%1_mah").arg(i + 1) },
                { "value", QString::number(batMah[i]) },
                { "unit", "mA" } });
        }
    }

    // absolute pressure (if available)
    if (std::isfinite(pressAbs)) {
        meta.append(QJsonObject {
            { "key", "press_abs" },
            { "value", QString::number(pressAbs, 'f', 2) },
            { "units", "Pa" } });
    }

    // altitude (from coordinate)
    if (std::isfinite(alt)) {
        meta.append(QJsonObject {
            { "key", "altitude" },
            { "value", QString::number(alt, 'f', 1) },
            { "units", "m" } });
    }

    QJsonObject loc {
        { "latitude", lat },
        { "longitude", lon },
        { "timestamp", timestamp },
        { "meta", meta }
    };

    QJsonObject payload {
        { "locations", QJsonArray { loc } }
    };

    // --- POST it ---
    QNetworkRequest req(QUrl(QString::fromUtf8(kUrlReportLocation)));
    req.setRawHeader("Authorization", authHeader()); // "Bearer …"
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    const QByteArray jsonBody = QJsonDocument(payload).toJson(QJsonDocument::Compact);
    QNetworkReply*   nrep     = m_nam.post(req, jsonBody);

    connect(nrep, &QNetworkReply::finished, this, [nrep]() {
        const QByteArray r = nrep->readAll();
        if (nrep->error() != QNetworkReply::NoError) {
            qWarning() << "[GeoWork] /report-location error:" << nrep->errorString()
                       << "payload:" << r;
            nrep->deleteLater();
            return;
        }
        qInfo() << "[GeoWork] /report-location OK";
        nrep->deleteLater();
    });
}
