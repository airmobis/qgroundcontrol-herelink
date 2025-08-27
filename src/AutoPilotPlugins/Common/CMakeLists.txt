cmake_minimum_required(VERSION 3.10)

project(SrcAutoPilotPluginsCommon LANGUAGES CXX)

add_library(AutoPilotPluginsCommon
    src/ESP8266Component.cc
    src/MotorComponent.cc
    src/SyslinkComponent.cc
    src/ESP8266ComponentController.cc
    src/RadioComponentController.cc
    src/SyslinkComponentController.cc
)

target_include_directories(AutoPilotPluginsCommon PUBLIC include)

add_custom_target(CommonAutoPilotPluginQml SOURCES
    res/ESP8266Component.qml
    res/ESP8266ComponentSummary.qml
    res/MotorComponent.qml
    res/RadioComponent.qml
    res/SetupPage.qml
    res/SyslinkComponent.qml
)
