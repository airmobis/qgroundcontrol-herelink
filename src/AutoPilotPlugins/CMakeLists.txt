cmake_minimum_required(VERSION 3.10)

project(SrcAutoPilotPlugins LANGUAGES CXX)

add_subdirectory(APM)
add_subdirectory(Common)
add_subdirectory(PX4)

add_library(AutoPilotPlugins
    src/AutoPilotPlugin.cc
)

target_link_libraries(AutoPilotPlugins
    PRIVATE
        qgc

    PUBLIC
        SrcAutoPilotPluginsAPM
        SrcAutoPilotPluginsCommon
        SrcAutoPilotPluginsPX4
)

target_include_directories(AutoPilotPlugins
    INTERFACE
        ${CMAKE_CURRENT_SOURCE_DIR}

    PUBLIC
        APM
        Common
        PX4
 )
