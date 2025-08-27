cmake_minimum_required(VERSION 3.10)

project(SrcFirmwarePluginAPM LANGUAGES CXX)

add_library(FirmwarePluginAPM
    src/APMFirmwarePlugin.cc
    src/APMParameterMetaData.cc
    src/ArduPlaneFirmwarePlugin.cc
    src/ArduSubFirmwarePlugin.cc
    src/APMFirmwarePluginFactory.cc
    src/ArduCopterFirmwarePlugin.cc
    src/ArduRoverFirmwarePlugin.cc
)

target_include_directories(FirmwarePluginAPM PUBLIC include)

add_custom_target(APMFirmwarePluginQml SOURCES
    res/APMSensorParams.qml
)
