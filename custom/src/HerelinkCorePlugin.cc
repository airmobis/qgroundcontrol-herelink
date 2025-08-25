#include "HerelinkCorePlugin.h"

#include "AppSettings.h"
#include "AutoConnectSettings.h"
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"
#include "JoystickManager.h"
#include "MultiVehicleManager.h"
#include "QGCApplication.h"
#include "QGCToolbox.h"
#include "VideoSettings.h"

QGC_LOGGING_CATEGORY(HerelinkCorePluginLog, "HerelinkCorePluginLog")

HerelinkCorePlugin::HerelinkCorePlugin(QGCApplication* app, QGCToolbox* toolbox)
    : QGCCorePlugin { app, toolbox } {
}

void HerelinkCorePlugin::setToolbox(QGCToolbox* toolbox) {
    QGCCorePlugin::setToolbox(toolbox);

    _herelinkOptions = new HerelinkOptions(this, nullptr);

    auto multiVehicleManager = qgcApp()->toolbox()->multiVehicleManager();
    connect(multiVehicleManager, &MultiVehicleManager::activeVehicleChanged, this, &HerelinkCorePlugin::_activeVehicleChanged);
}

bool HerelinkCorePlugin::overrideSettingsGroupVisibility(QString name) {
    // Hide all AutoConnect settings
    return name != AutoConnectSettings::name;
}

// We return `true` in every branch, so that we always show this setting in the UI.
bool HerelinkCorePlugin::adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData) {
    if (settingsGroup == AppSettings::settingsGroup) {
        // Default herelink fontsize of 10, nice starting point.
        if (metaData.name() == AppSettings::appFontPointSizeName) {
            metaData.setRawDefaultValue(10);
            return true;
        }

        // Default dark palette.
        if (metaData.name() == AppSettings::indoorPaletteName) {
            metaData.setRawDefaultValue(1);
            return true;
        }
    }

    if (settingsGroup == AutoConnectSettings::settingsGroup) {
        // We have to adjust the Herelink UDP autoconnect settings for the AirLink
        if (metaData.name() == AutoConnectSettings::udpListenPortName) {
            metaData.setRawDefaultValue(14551);
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostIPName) {
            metaData.setRawDefaultValue(QStringLiteral("127.0.0.1"));
        } else if (metaData.name() == AutoConnectSettings::udpTargetHostPortName) {
            metaData.setRawDefaultValue(15552);
        } else {
            // Disable all the other autoconnect types
            const char* disabledAndHiddenSettings[] {
                AutoConnectSettings::autoConnectPixhawkName,
                AutoConnectSettings::autoConnectSiKRadioName,
                AutoConnectSettings::autoConnectPX4FlowName,
                AutoConnectSettings::autoConnectRTKGPSName,
                AutoConnectSettings::autoConnectLibrePilotName,
                AutoConnectSettings::autoConnectNmeaPortName,
                AutoConnectSettings::autoConnectZeroConfName,
            };

            for (const char* const disabledAndHiddenSetting : disabledAndHiddenSettings) {
                if (disabledAndHiddenSetting == metaData.name()) {
                    metaData.setRawDefaultValue(false);
                }
            }
        }
    } else if (settingsGroup == VideoSettings::settingsGroup) {
        if (metaData.name() == VideoSettings::rtspTimeoutName) {
            metaData.setRawDefaultValue(60);
        } else if (metaData.name() == VideoSettings::videoSourceName) {
            metaData.setRawDefaultValue(VideoSettings::videoSourceHerelinkAirUnit);
        }
    } else if (settingsGroup == AppSettings::settingsGroup) {
        if (metaData.name() == AppSettings::androidSaveToSDCardName) {
            metaData.setRawDefaultValue(true);
        }
    }

    return true;
}

void HerelinkCorePlugin::_activeVehicleChanged(Vehicle* activeVehicle) {
    if (activeVehicle == nullptr) {
        return;
    }

    QString herelinkButtonsJoystickName { "gpio-keys" };

    JoystickManager& joystickManager { *qgcApp()->toolbox()->joystickManager() };
    if (joystickManager.activeJoystickName() != herelinkButtonsJoystickName
        && !joystickManager.setActiveJoystickName(herelinkButtonsJoystickName)) {
        qgcApp()->showAppMessage("Warning: Herelink buttton setup failed. Buttons will not work.");
        return;
    }

    activeVehicle->setJoystickEnabled(true);
}

namespace {
    struct data {
        QString factGroupName, factName;
        QString icon;
        QString text {};
        bool    showUnits { true };
    };

    void setData(InstrumentValueData& ivd, const data& d) {
        ivd.setFact(d.factGroupName, d.factName);
        ivd.setIcon(d.icon);
        ivd.setText(d.text.isNull() ? ivd.fact()->shortDescription() : d.text);
        ivd.setShowUnits(d.showUnits);
    }

    template <std::size_t N>
    void setDataBatched(HorizontalFactValueGrid& hfvg, std::size_t index, const data (&arr)[N]) {
        QmlObjectListModel& qolm { *hfvg.columns()->value<QmlObjectListModel*>(index) };

        for (std::size_t i { 0 }; i < N; ++i) {
            setData(*qolm.value<InstrumentValueData*>(i), arr[i]);
        }
    }
}

// Same as in the original, only we set font size to medium by default for Herelink
void HerelinkCorePlugin::factValueGridCreateDefaultSettings(const QString& defaultSettingsGroup) {
    HorizontalFactValueGrid factValueGrid { defaultSettingsGroup };

    const bool includeFWValues {
        factValueGrid.vehicleClass() == QGCMAVLink::VehicleClassFixedWing
        || factValueGrid.vehicleClass() == QGCMAVLink::VehicleClassVTOL
        || factValueGrid.vehicleClass() == QGCMAVLink::VehicleClassAirship
    };

    factValueGrid.setFontSize(FactValueGrid::MediumFontSize);

    // Append a column three times, plus one additional if `includeFWValues` is set.
    for (std::size_t i { 0 }; i < 3 + includeFWValues; ++i) {
        factValueGrid.appendColumn();
    }

    factValueGrid.appendRow();

    setDataBatched(factValueGrid, 0,
        {
            { "Vehicle", "AltitudeRelative", "arrow-thick-up.svg" },
            { "Vehicle", "DistanceToHome", "bookmark copy 3.svg" },
        });

    setDataBatched(factValueGrid, 1,
        {
            { "Vehicle", "ClimbRate", "arrow-simple-up.svg" },
            { "Vehicle", "GroundSpeed", "arrow-simple-right.svg" },
        });

    std::size_t endIndex { 2 };

    if (includeFWValues) {
        setDataBatched(factValueGrid, 2,
            { { "Vehicle", "AirSpeed", "AirSpd" },
                { "Vehicle", "ThrottlePct", "Thr" } });

        // Ensure we don't overwrite this entry.
        ++endIndex;
    }

    setDataBatched(factValueGrid, endIndex,
        { { "Vehicle", "FlightTime", "timer.svg", {}, false },
            { "Vehicle", "FlightDistance", "travel-walk.svg" } });
}
