cmake_minimum_required(VERSION 3.10)

project(SrcAutoPilotPluginsAPM LANGUAGES CXX)

add_library(AutoPilotPluginsAPM
    src/APMAirframeComponent.cc
    src/APMFlightModesComponentController.cc
    src/APMMotorComponent.cc
    src/APMSensorsComponent.cc
    src/APMAirframeComponentController.cc
    src/APMFollowComponent.cc
    src/APMPowerComponent.cc
    src/APMSensorsComponentController.cc
    src/APMAutoPilotPlugin.cc
    src/APMFollowComponentController.cc
    src/APMRadioComponent.cc
    src/APMSubFrameComponent.cc
    src/APMCameraComponent.cc
    src/APMHeliComponent.cc
    src/APMRemoteSupportComponent.cc
    src/APMSubMotorComponentController.cc
    src/APMFlightModesComponent.cc
    src/APMLightsComponent.cc
    src/APMSafetyComponent.cc
    src/APMTuningComponent.cc
)

target_include_directories(AutoPilotPluginsAPM PUBLIC include)

add_custom_target(APMAutoPilotPluginQml SOURCES
    res/APMAirframeComponent.qml
    res/APMAirframeComponentSummary.qml
    res/APMCameraComponent.qml
    res/APMCameraComponentSummary.qml
    res/APMCameraSubComponent.qml
    res/APMFlightModesComponent.qml
    res/APMFlightModesComponentSummary.qml
    res/APMHeliComponent.qml
    res/APMLightsComponent.qml
    res/APMLightsComponentSummary.qml
    res/APMNotSupported.qml
    res/APMPowerComponent.qml
    res/APMPowerComponentSummary.qml
    res/APMRadioComponentSummary.qml
    res/APMRemoteSupportComponent.qml
    res/APMSafetyComponent.qml
    res/APMSafetyComponentCopter.qml
    res/APMSafetyComponentPlane.qml
    res/APMSafetyComponentRover.qml
    res/APMSafetyComponentSub.qml
    res/APMSafetyComponentSummary.qml
    res/APMSafetyComponentSummaryCopter.qml
    res/APMSafetyComponentSummaryPlane.qml
    res/APMSafetyComponentSummaryRover.qml
    res/APMSafetyComponentSummarySub.qml
    res/APMSensorsComponent.qml
    res/APMSensorsComponentSummary.qml
    res/APMSubFrameComponent.qml
    res/APMSubFrameComponentSummary.qml
    res/APMSubMotorComponent.qml
    res/APMTuningComponentCopter.qml
    res/APMTuningComponentSub.qml
)
