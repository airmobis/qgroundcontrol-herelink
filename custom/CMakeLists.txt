# This is a really hacky way to get around the top-level CMakeLists.txt
# being incompatible with my CMake 4.1.0 -- I should probably do something
# about it, but I probably won't. Oh well!
# - Petr

cmake_minimum_required(VERSION 3.30)

project(ReallyDumbIdea VERSION 1.0 LANGUAGES CXX)

find_package(Qt5Widgets REQUIRED)
find_package(Qt5Network REQUIRED)
find_package(Qt5Positioning REQUIRED)

set(SOURCES
    _GeoWork
    CustomPlugin
    GeoWork
    GeoWork_qmlinit
    HerelinkCorePlugin
    HerelinkOptions
)

list(TRANSFORM SOURCES PREPEND src/)
list(TRANSFORM SOURCES APPEND .cc)

add_library(dummy ${SOURCES})
target_include_directories(dummy PRIVATE include)
target_compile_features(dummy PRIVATE cxx_std_17)
target_link_libraries(dummy PRIVATE
    Qt5::Widgets
    Qt5::Network
    Qt5::Positioning
)
