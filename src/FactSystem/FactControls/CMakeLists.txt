add_library(FactControls
    src/FactPanelController.cc
)

target_link_libraries(FactControls
    PUBLIC
        qgc
)

target_include_directories(FactControls
    PUBLIC
        include
)

add_custom_target(FactControlsQml SOURCES
    include/FactPanelController.h # FIXME Why is this here?
    res/AltitudeFactTextField.qml
    res/FactBitmask.qml
    res/FactCheckBox.qml
    res/FactComboBox.qml
    res/FactLabel.qml
    res/FactTextFieldGrid.qml
    res/FactTextField.qml
    res/FactTextFieldRow.qml
    res/FactTextFieldSlider.qml
    res/FactValueSlider.qml
)
