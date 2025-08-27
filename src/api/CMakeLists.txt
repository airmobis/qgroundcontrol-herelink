cmake_minimum_required(VERSION 3.10)

project(SrcApi LANGUAGES CXX)

add_library(api
 src/QGCCorePlugin.cc
 src/QGCOptions.cc
 src/QGCSettings.cc
 src/QmlComponentInfo.cc
)

target_link_libraries(api PUBLIC qgc)
target_include_directories(api PUBLIC include)
