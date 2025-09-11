#include "HerelinkCorePlugin.h"

#include "AutoConnectSettings.h"
#include "VideoSettings.h"
#include "AppSettings.h"
#include "QGCApplication.h"
#include "QGCToolbox.h"
#include "MultiVehicleManager.h"
#include "JoystickManager.h"
#include "HorizontalFactValueGrid.h"
#include "InstrumentValueData.h"

#include <list>


QGC_LOGGING_CATEGORY(HerelinkCorePluginLog, "HerelinkCorePluginLog")

HerelinkCorePlugin::HerelinkCorePlugin(QGCApplication *app, QGCToolbox* toolbox)
    : QGCCorePlugin{ app, toolbox }
{

}

void HerelinkCorePlugin::setToolbox(QGCToolbox* toolbox)
{
    QGCCorePlugin::setToolbox(toolbox);

    _herelinkOptions = new HerelinkOptions(this, nullptr);

    connect(
        qgcApp()->toolbox()->multiVehicleManager(),
        &MultiVehicleManager::activeVehicleChanged,
        this,
        &HerelinkCorePlugin::_activeVehicleChanged
    );
}

bool HerelinkCorePlugin::overrideSettingsGroupVisibility(QString name)
{
    // Hide all AutoConnect settings.
    return name != AutoConnectSettings::name;
}

bool HerelinkCorePlugin::adjustSettingMetaData(const QString& settingsGroup, FactMetaData& metaData)
{
    if (settingsGroup == AppSettings::settingsGroup) {
        // Default herelink fontsize of 10; nice starting point.
        if (metaData.name() == AppSettings::appFontPointSizeName) {
            metaData.setRawDefaultValue(10);
            return true;
        }

        // Default to a dark palette.
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
            // Disable all other autoconnect types.
            const char* const disabledAndHiddenSettings[] {
                AutoConnectSettings::autoConnectPixhawkName,
                AutoConnectSettings::autoConnectSiKRadioName,
                AutoConnectSettings::autoConnectPX4FlowName,
                AutoConnectSettings::autoConnectRTKGPSName,
                AutoConnectSettings::autoConnectLibrePilotName,
                AutoConnectSettings::autoConnectNmeaPortName,
                AutoConnectSettings::autoConnectZeroConfName,
            };

            for (const char* const disabledAndHiddenSetting: disabledAndHiddenSettings) {
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

    return true; // Show all settings in ui
}

void HerelinkCorePlugin::_activeVehicleChanged(Vehicle* activeVehicle)
{
    if (activeVehicle == nullptr) {
        return;
    }

    QString herelinkButtonsJoystickName("gpio-keys");
    auto joystickManager = qgcApp()->toolbox()->joystickManager();

    if (joystickManager->activeJoystickName() != herelinkButtonsJoystickName) {
        if (!joystickManager->setActiveJoystickName(herelinkButtonsJoystickName)) {
            qgcApp()->showAppMessage("Warning: Herelink buttton setup failed. Buttons will not work.");

            return;
        }
    }

    activeVehicle->setJoystickEnabled(true);
}

namespace {
    struct data {
        QString factName, icon, text{};
        bool showUnits{true};
    };

    void setValue(InstrumentValueData& ivd, const data& d) {
        ivd.setFact("Vehicle", d.factName);

        if (!d.icon.isNull()) {
            ivd.setIcon(d.icon);
        }

        ivd.setText(d.text.isNull() ? ivd.fact()->shortDescription() : d.text);
        ivd.setShowUnits(d.showUnits);
    }

    template <std::size_t N>
    void setValues(QmlObjectListModel& qolm, const data (&arr)[N]) {
        for (std::size_t i{0}; i < N; ++i) {
            setValue(*qolm.value<InstrumentValueData*>(i), arr[i]);
        }
    }
}

// Same as original, only we set font size to medium by default for Herelink
void HerelinkCorePlugin::factValueGridCreateDefaultSettings(const QString& defaultSettingsGroup)
{
    HorizontalFactValueGrid factValueGrid { defaultSettingsGroup };

    const bool includeFWValues {
        factValueGrid.vehicleClass() == QGCMAVLink::VehicleClassFixedWing
        || factValueGrid.vehicleClass() == QGCMAVLink::VehicleClassVTOL
        || factValueGrid.vehicleClass() == QGCMAVLink::VehicleClassAirship
    };

    factValueGrid.setFontSize(FactValueGrid::MediumFontSize);

    for (std::size_t i{0}; i < 3 + includeFWValues; ++i) {
        factValueGrid.appendColumn();
    }

    factValueGrid.appendRow();

    setValues(*factValueGrid.columns()->value<QmlObjectListModel*>(0), {
        {"AltitudeRelative", "arrow-thick-up.svg"},
        {"DistanceToHome", "bookmark copy 3.svg"}
    });

    setValues(*factValueGrid.columns()->value<QmlObjectListModel*>(1), {
        {"ClimbRate", "arrow-simple-up.svg"},
        {"GroundSpeed", "arrow-simple-right.svg"}
    });

    int lastRow { 2 };

    if (includeFWValues) {
        setValues(*factValueGrid.columns()->value<QmlObjectListModel*>(1), {
            {"AirSpeed", {}, "AirSpd"},
            {"ThrottlePct", {}, "Thr"},
        });

        ++lastRow;
    }

    setValues(*factValueGrid.columns()->value<QmlObjectListModel*>(1), {
        {"FlightTime", "timer.svg", {}, false},
        {"FlightDistance", "travel-walk.svg"}
    });
}
