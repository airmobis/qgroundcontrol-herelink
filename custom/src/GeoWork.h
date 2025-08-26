#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QGeoCoordinate>

class QQuickItem;

class GeoWork : public QObject {
    Q_OBJECT

    // Read-only values presented to QML
    Q_PROPERTY(QString projectId   READ projectId   NOTIFY projectIdChanged)
    Q_PROPERTY(QString stateId     READ stateId     NOTIFY stateIdChanged)

    // User-configurable, persisted settings
    Q_PROPERTY(QString deviceName  READ deviceName  WRITE setDeviceName  NOTIFY deviceNameChanged)
    Q_PROPERTY(QString bearerToken READ bearerToken WRITE setBearerToken NOTIFY bearerTokenChanged)

    // 0 = NoToken (grey), 1 = Valid (green), 2 = Invalid (red)
    Q_PROPERTY(int tokenStatus READ tokenStatus NOTIFY tokenStatusChanged)

public:
    explicit GeoWork(QObject* parent = nullptr);

    // Getters for QML
    QString projectId()   const { return m_projectId; }
    QString stateId()     const { return m_stateId; }
    QString deviceName()  const { return m_deviceName; }
    QString bearerToken() const { return m_bearerToken; }
    int     tokenStatus() const { return _tokenStatus; }

public slots:
    // --- Main flow used by QML ---
    // 1) Check session; if active task, POST project-marker-state {name}; cache stateId
    void checkActiveTaskAndFetchState(const QString& stateName);

    // 2) Create a marker using current GPS from active vehicle
    void createMarker();

    // ---- Minimal additions ----
    Q_INVOKABLE void setVideoItem(QObject* videoItem);   // bind the live video surface from QML
    Q_INVOKABLE void AddPhoto();
    Q_INVOKABLE void AddPhotoForMarker(const QString& markerId);
    Q_INVOKABLE void autoBindVideo();  // try to locate video item automatically                         // capture current frame and save to Downloads

    // --- Settings helpers (persisted via QSettings) ---
    void setDeviceName(const QString& name);
    void setBearerToken(const QString& token);            // accepts token with or without "Bearer "
    bool setBearerTokenFromFile(const QString& fileUrl);  // read token text from file URL or path

    // Token validation (heuristic placeholder; swap for real endpoint later)
    void validateToken();

    // Explicit persist/load if you want to call from elsewhere
    void saveSettings();
    void loadSettings();

    void reportLocation();

signals:
    void projectIdChanged();
    void stateIdChanged();
    void deviceNameChanged();
    void bearerTokenChanged();
    void tokenStatusChanged();
    void photoSaved(const QString& savedPath);
    void photoSaveFailed(const QString& reason);

private:
    QByteArray authHeader() const;

    // --- Minimal additions for frame capture ---
    void captureAndSave();
    void uploadPhotoToMarker(const QString& markerId, const QString& photoPath);

    QQuickItem* findVideoItemRecursive(QQuickItem* root) const; // internal helper
    QObject* m_videoItemObj {nullptr};

    // Returns true if we could read a valid lat/lon from QGC's active vehicle
    bool getActiveVehicleCoordinate(double& latOut, double& lonOut, double& altOut) const;

    QNetworkAccessManager m_nam;

    // Cached runtime state
    QString m_projectId;
    QString m_stateId;

    // Persisted user settings
    QString m_deviceName;  // e.g., "BLUE001"
    QString m_bearerToken; // normalized to start with "Bearer "

public:
    enum class TokenStatus {
        None,
        Valid,
        Invalid
    };
private:
    TokenStatus _tokenStatus {TokenStatus::None};

    // Organization/App for QSettings (Android/desktop-safe)
    QSettings _settings{ "Airmobis", "QGroundControl" };
};
