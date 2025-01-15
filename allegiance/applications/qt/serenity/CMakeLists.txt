project(KDAB_Qt_Serenity_Vulkan)

find_package(
    Qt6
    COMPONENTS Gui
               Widgets
               Qml
               Quick
               QuickWidgets
               QuickControls2
    REQUIRED
)

qt_standard_project_setup()
qt_add_executable(
    ${PROJECT_NAME}
    main.cpp
    serenity_gui_application.cpp
    serenity_gui_application.h
    serenity_window_qt.h
    serenity_window_qt.cpp
    serenity_renderer_qt.h
    serenity_renderer_qt.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC Qt6::Widgets KDAB::QtCommon KDAB::SerenityRenderer
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
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT applications
)