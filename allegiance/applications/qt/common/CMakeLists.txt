project(KDAB_Qt_Common)

find_package(
    Qt6
    COMPONENTS Core
               Widgets
               Quick
               QuickWidgets
               Gui
               QuickControls2
               Qml
               3DExtras
    REQUIRED
)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 20)

# Lib
qt_add_library(
    ${PROJECT_NAME}
    STATIC
    stereo_camera.h
    util_qt.h
    camera_control.h
    main_window.h
    main_window.cpp
    renderer_initializer.h
    window_event_watcher.h
    ${CMAKE_SOURCE_DIR}/assets/resources.qrc
)

add_library(
    KDAB::QtCommon ALIAS ${PROJECT_NAME}
)

# Schneider QML module
add_subdirectory(qml/Schneider)

# Main Module for Root QML File
qt_add_qml_module(
    ${PROJECT_NAME}
    URI
    common
    QML_FILES
    "qml/camera_control.qml"
    RESOURCE_PREFIX
    "/qt/qml" # Set to default prefix for elements to be in a know import path
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC Qt6::Gui
           Qt6::Widgets
           Qt6::QuickWidgets
           Qt6::QuickControls2
           shared
           KDAB_QT_Common_Schneider_QML # To access the Singleton Types
           KDAB_QT_Common_Schneider_QMLplugin # To allow QML Imports to work since plugin is static
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../../
)
