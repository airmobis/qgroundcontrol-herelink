add_subdirectory(APM)
add_subdirectory(PX4)

add_library(FirmwarePlugin
    src/CameraMetaData.cc
    src/FirmwarePlugin.cc
    src/FirmwarePluginManager.cc
)

target_link_libraries(FirmwarePlugin
    PRIVATE
        qgc

    PUBLIC
        FirmwarePluginAPM
        FirmwarePluginPX4
)

target_include_directories(FirmwarePlugin
    PUBLIC
        include
)
