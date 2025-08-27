if (BUILD_TESTING)
    set(EXTRA_SRC AudioOutputTest.cc)
endif()

add_library(Audio
    src/AudioOutput.cc
    ${EXTRA_SRC}
)

find_package(Qt5Core REQUIRED)
find_package(Qt5Qml REQUIRED)
find_package(Qt5TextToSpeech REQUIRED)
find_package(Qt5Widgets REQUIRED)

target_link_libraries(Audio
    PRIVATE
        qgc

    PUBLIC
        Qt5::Core
        Qt5::Qml
        Qt5::TextToSpeech
        Qt5::Widgets
)

target_include_directories(Audio PUBLIC include)

if(BUILD_TESTING)
    add_qgc_test(AudioOutputTest)
endif()
