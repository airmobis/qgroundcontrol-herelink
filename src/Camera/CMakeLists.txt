add_library(Camera
    src/QGCCameraControl.cc
    src/QGCCameraIO.cc
    src/QGCCameraManager.cc
)

target_link_libraries(Camera
    PUBLIC
        qgc
)

target_include_directories(Camera PUBLIC include)
