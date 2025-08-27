cmake_minimum_required(VERSION 3.10)

project(QGroundControlHerelinkCustom LANGUAGES C CXX)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/../src qgc)

add_library(dummy
    src/CustomPlugin.cc
    src/GeoWork.cc
    src/GeoWork_qmlinit.cc
    src/HerelinkCorePlugin.cc
    src/HerelinkOptions.cc
    src/VideoStreamControl.cc
)

target_compile_features(dummy PRIVATE cxx_std_17)
target_link_libraries(dummy PRIVATE qgc mavlink)
target_include_directories(dummy PRIVATE include)
