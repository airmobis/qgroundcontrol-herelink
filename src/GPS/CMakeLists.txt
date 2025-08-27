

add_library(gps
 Drivers/src/ashtech.cpp
 Drivers/src/gps_helper.cpp
 Drivers/src/mtk.cpp
 Drivers/src/rtcm.cpp
 Drivers/src/sbf.cpp
 Drivers/src/ubx.cpp
 GPSManager.cc
 GPSProvider.cc
 RTCM/RTCMMavlink.cc
)

find_package(Qt5Core REQUIRED)
find_package(Qt5Location REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Svg REQUIRED)
find_package(Qt5TextToSpeech REQUIRED)

target_link_libraries(gps
 Qt5::Core
 Qt5::Location
 Qt5::SerialPort
 Qt5::Svg
 Qt5::TextToSpeech

 qgc
)

target_include_directories(gps INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

