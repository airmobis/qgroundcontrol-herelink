message("Adding Custom Herelink Plugin")

#-- Version control
#   Major and minor versions are defined here (manually)

CUSTOM_QGC_VER_MAJOR = 4
CUSTOM_QGC_VER_MINOR = 4
CUSTOM_QGC_VER_PATCH = 5
CUSTOM_QGC_VER_FIRST_BUILD = 0

linux {
    QMAKE_CXXFLAGS_WARN_ON += -Wno-strict-aliasing
}

# Build number is automatic
# Uses the current branch. This way it works on any branch including build-server's PR branches
CUSTOM_QGC_VER_BUILD = $$system(git --git-dir ../.git rev-list $$GIT_BRANCH --first-parent --count)
win32 {
    CUSTOM_QGC_VER_BUILD = $$system("set /a $$CUSTOM_QGC_VER_BUILD - $$CUSTOM_QGC_VER_FIRST_BUILD")
} else {
    CUSTOM_QGC_VER_BUILD = $$system("echo $(($$CUSTOM_QGC_VER_BUILD - $$CUSTOM_QGC_VER_FIRST_BUILD))")
}
CUSTOM_QGC_VERSION = $${CUSTOM_QGC_VER_MAJOR}.$${CUSTOM_QGC_VER_MINOR}.$${CUSTOM_QGC_VER_PATCH}.$${CUSTOM_QGC_VER_BUILD}

DEFINES -= APP_VERSION_STR=\"\\\"$$APP_VERSION_STR\\\"\"
DEFINES += APP_VERSION_STR=\"\\\"$$CUSTOM_QGC_VERSION\\\"\"

message(Custom QGC Version: $${CUSTOM_QGC_VERSION})

# Branding

DEFINES += CUSTOMHEADER=\"\\\"HerelinkCorePlugin.h\\\"\"
DEFINES += CUSTOMCLASS=HerelinkCorePlugin

TARGET   = Herelink-QGroundControl
DEFINES += QGC_APPLICATION_NAME='"\\\"Herelink QGroundControl\\\""'

DEFINES += QGC_ORG_NAME=\"\\\"cubepilot.org\\\"\"
DEFINES += QGC_ORG_DOMAIN=\"\\\"org.cubepilot\\\"\"

QGC_APP_NAME        = "Herelink QGroundControl"
QGC_BINARY_NAME     = "Herelink-QGroundControl"
QGC_ORG_NAME        = "Cubepilot"
QGC_ORG_DOMAIN      = "org.cubepilot"
QGC_ANDROID_PACKAGE = "org.cubepilot.herelink_qgroundcontrol"
QGC_APP_DESCRIPTION = "Herelink QGroundControl"
QGC_APP_COPYRIGHT   = "Copyright (C) 2024 Cubepilot. All rights reserved."

# Remove code which the Herelink doesn't need
DEFINES += \
    QGC_GST_TAISYNC_DISABLED
    NO_SERIAL_LINK
    QGC_DISABLE_BLUETOOTH

# Enable Herelink AirUnit video config
DEFINES += \
    QGC_HERELINK_AIRUNIT_VIDEO

CONFIG += AndroidHomeApp

# Our own, custom resources
# Not yet used
RESOURCES += \
    $$PWD/custom.qrc
message(*** USING custom.qrc from $$PWD ***)

QML_IMPORT_PATH += \
   $$PWD/src

# Herelink specific custom sources
SOURCES += \
    $$PWD/src/HerelinkCorePlugin.cc \
    $$PWD/src/HerelinkOptions.cc \


HEADERS += \
    $$PWD/include/HerelinkCorePlugin.h \
    $$PWD/include/HerelinkOptions.h \


INCLUDEPATH += \
    $$PWD/include \

# Herelink specific custom sources, for HDMI switching
message("Including custom Herelink sources for source switching")

SOURCES += $$PWD/src/VideoStreamControl.cc
HEADERS += $$PWD/include/VideoStreamControl.h
INCLUDEPATH += $$PWD/herelink

# Custom versions of a Herelink build should only add changes below here to prevent conflicts
SOURCES += \
    $$PWD/src/GeoWork.cc \
    $$PWD/src/CustomPlugin.cc \
    $$PWD/src/GeoWork_qmlinit.cc

HEADERS += \
    $$PWD/include/GeoWork.h \
    $$PWD/include/CustomPlugin.h

QT += network
