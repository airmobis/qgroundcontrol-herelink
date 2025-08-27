if (BUILD_TESTING)
    set(EXTRA_SRC
        MockLink.cc
        MockLink.h
        MockLinkFTP.cc
        MockLinkFTP.h
        MockLinkMissionItemHandler.cc
        MockLinkMissionItemHandler.h
    )
endif()

add_library(comm
     #src/BluetoothLink.cc
     src/LinkConfiguration.cc
     src/LinkInterface.cc
     src/LinkManager.cc
     src/LogReplayLink.cc
     src/MavlinkMessagesTimer.cc
     src/MAVLinkProtocol.cc
     src/QGCMAVLink.cc
     src/QGCSerialPortInfo.cc
     src/SerialLink.cc
     src/TCPLink.cc
     src/UdpIODevice.cc
     src/UDPLink.cc
     ${EXTRA_SRC}
)

find_package(Qt5Location REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5TextToSpeech REQUIRED)
find_package(Qt5Widgets REQUIRED)

add_subdirectory(../../libs/mavlink mavlink)

target_link_libraries(comm
    PUBLIC
        qgc
        Qt5::Location
        Qt5::SerialPort
        Qt5::Test
        Qt5::TextToSpeech
        Qt5::Widgets
        qmdnsengine
        mavlink
)

target_include_directories(comm PUBLIC include)
