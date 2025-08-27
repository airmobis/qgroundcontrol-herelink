cmake_minimum_required(VERSION 3.10)

project(SrcFirmwarePluginPX4 LANGUAGES CXX)

add_library(FirmwarePluginPX4
    src/PX4FirmwarePlugin.cc
    src/PX4FirmwarePluginFactory.cc
    src/PX4ParameterMetaData.cc
)

target_include_directories(FirmwarePluginPX4 PUBLIC include)
