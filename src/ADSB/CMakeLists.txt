add_library(ADSB
    src/ADSBVehicle.cc
    src/ADSBVehicleManager.cc
)

target_link_libraries(ADSB PUBLIC qgc)
target_include_directories(ADSB PUBLIC include)
