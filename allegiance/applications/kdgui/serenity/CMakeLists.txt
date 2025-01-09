project(KDAB_KDGui_Serenity)

add_executable(
    ${PROJECT_NAME}
    main.cpp
    renderer_initializer.cpp
    renderer_initializer.h
    serenity_kdgui_window.cpp
    serenity_kdgui_window.h
    window_watcher.h
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC shared KDAB::SerenityRenderer KDUtils::KDGui
)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
