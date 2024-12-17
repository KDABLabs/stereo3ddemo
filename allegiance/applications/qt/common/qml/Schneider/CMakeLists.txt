qt_add_qml_module(
    KDAB_QT_Common_Schneider_QML
    URI
    Schneider
    VERSION
    1.0
    SOURCES
    camera_controller.h
    style.h
    QML_FILES
    "qml/ButtonX.qml"
    "qml/CheckBoxX.qml"
    "qml/CursorMenu.qml"
    "qml/MainMenu.qml"
    "qml/PageBase.qml"
    "qml/SliderValue.qml"
    "qml/SectionTitle.qml"
    RESOURCES
    "qml/checked.svg"
    "qml/unchecked.svg"
    RESOURCE_PREFIX
    "/qt/qml" # Set to default prefix for elements to be in a know import path
    STATIC
    OUTPUT_TARGETS
)

target_link_libraries(
    KDAB_QT_Common_Schneider_QML
    PRIVATE Qt6::Gui shared
)
