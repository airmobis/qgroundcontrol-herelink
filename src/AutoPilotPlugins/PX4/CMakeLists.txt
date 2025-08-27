cmake_minimum_required(VERSION 3.10)

project(SrcAutoPilotPluginsPX4 LANGUAGES CXX)

add_library(AutoPilotPluginsPX4
    src/ActuatorComponent.cc
    src/FlightModesComponent.cc
    src/PX4FlightBehavior.cc
    src/SensorsComponent.cc
    src/AirframeComponent.cc
    src/PowerComponent.cc
    src/PX4RadioComponent.cc
    src/SensorsComponentController.cc
    src/AirframeComponentAirframes.cc
    src/PowerComponentController.cc
    src/PX4SimpleFlightModesController.cc
    src/AirframeComponentController.cc
    src/PX4AirframeLoader.cc
    src/PX4TuningComponent.cc
    src/CameraComponent.cc
    src/PX4AutoPilotPlugin.cc
    src/SafetyComponent.cc
)

target_include_directories(AutoPilotPluginsPX4 PUBLIC include)

add_custom_target(PX4AutoPilotPluginQml SOURCES
  res/AirframeComponent.qml
  res/AirframeComponentSummary.qml
  res/BatteryParams.qml
  res/CameraComponent.qml
  res/CameraComponentSummary.qml
  res/FlightModesComponentSummary.qml
  res/PowerComponent.qml
  res/PowerComponentSummary.qml
  res/PX4FlightModes.qml
  res/PX4RadioComponentSummary.qml
  res/PX4SimpleFlightModes.qml
  res/PX4TuningComponentCopter.qml
  res/PX4TuningComponentPlane.qml
  res/PX4TuningComponentVTOL.qml
  res/SafetyComponent.qml
  res/SafetyComponentSummary.qml
  res/SensorsComponent.qml
  res/SensorsComponentSummaryFixedWing.qml
  res/SensorsComponentSummary.qml
  res/SensorsSetup.qml
)
