add_subdirectory(FactControls)

if (BUILD_TESTING)
    set(EXTRA_SRC
        FactSystemTestBase.cc
        FactSystemTestGeneric.cc
        FactSystemTestPX4.cc
        ParameterManagerTest.cc
    )
endif()

add_library(FactSystem
 src/Fact.cc
 src/FactGroup.cc
 src/FactMetaData.cc
 src/FactSystem.cc
 src/FactValueSliderListModel.cc
 src/ParameterManager.cc
 src/SettingsFact.cc

 res/FactSystemTest.qml

 ${EXTRA_SRC}
)

target_link_libraries(FactSystem
    PRIVATE
        qgc

    PUBLIC
        FactControls
)

target_include_directories(FactSystem
    PUBLIC
        include
)
