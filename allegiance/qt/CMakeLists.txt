project(${POSTFIX})

find_package(
    Qt6
    COMPONENTS Widgets
               Quick
               QuickWidgets
               Gui
               QuickControls2
               Qml
               3DExtras
    REQUIRED
)
qt_standard_project_setup()

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION)
qt_add_qml_module(
    ${PROJECT_NAME}
    URI
    Schneider
    VERSION
    1.0
    QML_FILES
    "resources/ButtonX.qml"
    "resources/camera_control.qml"
    "resources/CheckBoxX.qml"
    "resources/MainMenu.qml"
    "resources/PageBase.qml"
    "resources/SliderValue.qml"
)

target_sources(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/assets/resources.qrc")

target_sources(
    ${PROJECT_NAME}
    PRIVATE "entry_main.cpp"
            "qml_nodes.h"
            "stereo_camera.h"
            "camera_control.h"
            "window_watcher.h"
            "window.h"
            "app.h"
            "ui/camera_controller.h"
            "ui/style.h"
            "resources/resources.qrc"
)
target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ${QT_TARGET_LIB}
            Qt6::Gui
            Qt6::Qml
            Qt6::Widgets
            Qt6::Quick
            Qt6::QuickWidgets
            Qt6::QuickControls2
)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 20
               AUTOMOC ON
               AUTORCC ON
               RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_compile_definitions(
    ${PROJECT_NAME} PRIVATE QT_NO_KEYWORDS ALLEGIANCE_BUILD_STR="v${PROJECT_VERSION} - ${GIT_COMMIT_DATE}"
)
install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications
)

qt_finalize_target(${PROJECT_NAME})
qt_generate_deploy_qml_app_script(
    TARGET
    ${PROJECT_NAME}
    OUTPUT_SCRIPT
    deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
    DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
)
install(
    SCRIPT ${deploy_script}
    COMPONENT applications
)
