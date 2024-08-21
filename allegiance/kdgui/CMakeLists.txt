project(${POSTFIX})

add_executable(${PROJECT_NAME}
                "entry_main.cpp"
                "window.h"
                "app.h"
                "window_watcher.h"
                "stereo_camera.h"
)

set(RENDER_IMPL serenity_render)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ${RENDER_IMPL}
)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 20
               RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)
target_compile_definitions(
    ${PROJECT_NAME} PRIVATE ALLEGIANCE_BUILD_STR="v${PROJECT_VERSION} - ${GIT_COMMIT_DATE}"
)
install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications
)
