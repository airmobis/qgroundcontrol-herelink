cmake_minimum_required(VERSION 3.10)

project(SrcAutoPilotPluginsGeneric LANGUAGES CXX)

add_library(AutoPilotPluginsGeneric
    src/GenericAutoPilotPlugin.cc
)

target_include_directories(AutoPilotPluginsGeneric PUBLIC include)
