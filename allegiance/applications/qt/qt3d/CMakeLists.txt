project(KDAB_Qt_Qt3D_OpenGL)

find_package(
    Qt6
    6.8.2
    COMPONENTS Gui
               Widgets
               3DExtras
               3DCore
               3DRender
               3DInput
               Qml
               Quick
               QuickWidgets
               QuickControls2
    REQUIRED
    CONFIG
)

qt_standard_project_setup()
qt_add_executable(${PROJECT_NAME} main.cpp)

# Fake QML Module to have qmlimportscanner correctly deploy the Qt QML plugins
# when packaging (we don't need to deploy our own modules since those are static)
qt_add_qml_module(
    ${PROJECT_NAME}
    URI
    imports
    QML_FILES
    "imports.qml"
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC Qt6::Widgets KDAB::QtCommon KDAB::Qt3DRenderer
)

set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 20
               AUTOMOC ON
               AUTORCC ON
               RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications
)

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
