#pragma once

#include <QObject>

#include "MAVLinkProtocol.h"
#include "VideoSettings.h"
#include "SettingsManager.h"

Q_DECLARE_LOGGING_CATEGORY(VideoStreamControlLog)

class VideoStreamControl : public QObject
{
    Q_OBJECT
public:
    VideoStreamControl();
    ~VideoStreamControl();

    Q_PROPERTY(bool settingInProgress READ settingInProgress NOTIFY settingInProgressChanged)
    bool settingInProgress() { return _settingInProgress; }

signals:
    void settingInProgressChanged();
    void videoNeedsReset();

private slots:
    void _mavlinkMessageReceived(LinkInterface *link, mavlink_message_t message);
    void _settingInProgressTimeout();
    void _cameraIdChanged();

private:
    int _systemId;
    LinkInterface *_linkInterface;
    MAVLinkProtocol *_mavlinkProtocol;
    VideoSettings *_videoSettings;
    QTimer _settingInProgressTimer;
    uint32_t _cameraServiceUid;
    uint32_t _cameraCount;
    uint32_t _cameraIdSetting;
    bool _settingInProgress;

    void _handleHeartbeatInfo(LinkInterface* link, mavlink_message_t& message);
    void _setCameraId();
    void _setCameraIdLockUi(bool lockUi);
    void _startVideoStreaming();
    void _setSettingInProgress(bool inProgress);
};
