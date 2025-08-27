
set(EXTRA_SRC)
if(BUILD_TESTING)
    list(APPEND EXTRA_SRC
  LogDownloadTest.cc
  LogDownloadTest.h
 )
endif()

add_library(AnalyzeView
 ExifParser.cc
 ExifParser.h
 GeoTagController.cc
 GeoTagController.h
 LogDownloadController.cc
 LogDownloadController.h
 MavlinkConsoleController.cc
 MavlinkConsoleController.h
 MAVLinkInspectorController.cc
 MAVLinkInspectorController.h
 PX4LogParser.cc
 PX4LogParser.h
 ULogParser.cc
 ULogParser.h

 ${EXTRA_SRC}
)

add_custom_target(AnalyzeViewQml
 SOURCES
  AnalyzePage.qml
  AnalyzeView.qml
  GeoTagPage.qml
  LogDownloadPage.qml
  MavlinkConsolePage.qml
  MAVLinkInspectorPage.qml
  VibrationPage.qml
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

target_include_directories(AnalyzeView INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

