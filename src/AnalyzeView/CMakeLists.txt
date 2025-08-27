cmake_minimum_required(VERSION 3.10)

project(SrcAnalyzeView LANGUAGES CXX)

if (BUILD_TESTING)
    set(EXTRA_SRC LogDownloadTest.cc)
endif()

add_library(AnalyzeView
    src/ExifParser.cc
    src/GeoTagController.cc
    src/LogDownloadController.cc
    src/MavlinkConsoleController.cc
    src/MAVLinkInspectorController.cc
    src/PX4LogParser.cc
    src/ULogParser.cc
    ${EXTRA_SRC}
)

add_custom_target(AnalyzeViewQml SOURCES
    res/AnalyzePage.qml
    res/AnalyzeView.qml
    res/GeoTagPage.qml
    res/LogDownloadPage.qml
    res/MavlinkConsolePage.qml
    res/MAVLinkInspectorPage.qml
    res/VibrationPage.qml
)

find_package(Qt5Charts REQUIRED)
find_package(Qt5Location REQUIRED)
find_package(Qt5SerialPort REQUIRED)
find_package(Qt5TextToSpeech REQUIRED)
find_package(Qt5Widgets REQUIRED)

target_link_libraries(AnalyzeView
  PRIVATE
    qgc

  PUBLIC
    Qt5::Charts
    Qt5::Location
    Qt5::SerialPort
    Qt5::TextToSpeech
    Qt5::Widgets
)

target_include_directories(AnalyzeView INTERFACE include)
