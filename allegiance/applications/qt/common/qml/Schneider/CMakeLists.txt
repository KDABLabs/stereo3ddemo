qt_add_qml_module(
    KDAB_QT_Common_Schneider_QML
    URI
    Schneider
    VERSION
    1.0
    SOURCES
    scene_controller.h
    scene_controller.cpp
    camera_controller.h
    camera_controller.cpp
    cursor_controller.h
    cursor_controller.cpp
    style.h
    QML_FILES
    "qml/ButtonX.qml"
    "qml/CheckBoxX.qml"
    "qml/CursorMenu.qml"
    "qml/CameraMenu.qml"
    "qml/MainMenu.qml"
    "qml/PageBase.qml"
    "qml/SliderValue.qml"
    "qml/NonLinearSliderValue.qml"
    "qml/SectionTitle.qml"
    "qml/Logos.qml"
    "qml/CustomSlider.qml"
    RESOURCES
    "qml/checked.svg"
    "qml/unchecked.svg"
    "qml/kdab.png"
    "qml/schneider_white.svg"
    RESOURCE_PREFIX
    "/qt/qml" # Set to default prefix for elements to be in a know import path
    STATIC
    OUTPUT_TARGETS
)

target_link_libraries(
    KDAB_QT_Common_Schneider_QML
    PRIVATE Qt6::Gui shared
)

target_compile_definitions(KDAB_QT_Common_Schneider_QML PUBLIC QT_NO_KEYWORDS)
