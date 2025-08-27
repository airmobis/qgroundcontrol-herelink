cmake_minimum_required(VERSION 3.10)

project(SrcADSB LANGUAGES CXX)

add_library(ADSB
    src/ADSBVehicle.cc
    src/ADSBVehicleManager.cc
)

target_link_libraries(ADSB PUBLIC qgc)
target_include_directories(ADSB PUBLIC include)
