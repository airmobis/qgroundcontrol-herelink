#pragma once

#include <QObject>
#include <QNetworkAccessManager>
#include <QSettings>
#include <QGeoCoordinate>

class GeoWork : public QObject {
    Q_OBJECT

    // Read-only values presented to QML
    Q_PROPERTY(QString projectId   READ projectId   NOTIFY projectIdChanged)
    Q_PROPERTY(QString stateId     READ stateId     NOTIFY stateIdChanged)

    // User-configurable, persisted settings
    Q_PROPERTY(QString deviceName  READ deviceName  WRITE setDeviceName  NOTIFY deviceNameChanged)
    Q_PROPERTY(QString bearerToken READ bearerToken WRITE setBearerToken NOTIFY bearerTokenChanged)

    // 0 = NoToken (grey), 1 = Valid (green), 2 = Invalid (red)
    Q_PROPERTY(int tokenStatus     READ tokenStatus                      NOTIFY tokenStatusChanged)

public:
    explicit GeoWork(QObject* parent = nullptr);

    // Getters for QML
    QString projectId()   const { return _projectId; }
    QString stateId()     const { return _stateId; }
    QString deviceName()  const { return _deviceName; }
    QString bearerToken() const { return _bearerToken; }
    int     tokenStatus() const { return _tokenStatus; }

public slots:
    // --- Main flow used by QML ---
    // 1) Check session; if active task, POST project-marker-state {name}; cache stateId
    void checkActiveTaskAndFetchState(const QString& stateName);

    // 2) Create a marker using current GPS from active vehicle
    void createMarker();

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

private:
    QByteArray authHeader() const;

    // Returns true if we could read a valid lat/lon from QGC's active vehicle
    bool _getActiveVehicleCoordinate(double& latOut, double& lonOut, double& altOut) const;

    QNetworkAccessManager _nam;

    // Cached runtime state
    QString _projectId;
    QString _stateId;

    // Persisted user settings
    QString _deviceName;     // e.g., "BLUE001"
    QString _bearerToken;    // normalized to start with "Bearer "
    int     _tokenStatus = 0; // 0=no token, 1=valid, 2=invalid

    // Organization/App for QSettings (Android/desktop-safe)
    QSettings _settings{ "Airmobis", "QGroundControl" };
};
