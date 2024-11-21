project(KDAB_Serenity_Renderer)

include(Deps.cmake)
if(NOT DEFINED CompileShaderSet)
    include(CompileShader.cmake)
endif()

add_subdirectory(shaders)

# Lib
add_library(
    ${PROJECT_NAME} STATIC
)
add_library(
    KDAB::SerenityRenderer ALIAS ${PROJECT_NAME}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC ALLEGIANCE_SERENITY SHADER_DIR="serenity/shaders/")
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/../../
)

target_sources(
    ${PROJECT_NAME}
    PUBLIC "serenity_window.h"
           "serenity_renderer.h"
           "mesh_loader.h"
           "window_extent_watcher.h"
           "stereo_proxy_camera.h"
           "serenity_stereo_graph.h"
           "picking_application_layer.h"
           "cursor.h"
    PRIVATE "picking_application_layer.cpp"
            "serenity_stereo_graph.cpp"
            "window_extent_watcher.cpp"
            "mesh_loader.cpp"
            "serenity_renderer.cpp"
            "cursor.cpp"
)

# if(BUILD_SERENITY_RENDERER_QT)
#     target_sources(${PROJECT_NAME} PUBLIC "serenity_impl_qt.h" "serenity_qt.h")
# endif()

# if(BUILD_SERENITY_RENDERER_KDGUI)
#     target_sources(${PROJECT_NAME} PUBLIC "serenity_impl_kdgui.h" "serenity_kdgui.h")
# endif()

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC KDGpu::KDGpu
           Serenity::Core
           Serenity::Gui
           Serenity::Logic
           assimp::assimp
           shared
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)
add_dependencies(${PROJECT_NAME} shaders)
target_precompile_headers(${PROJECT_NAME} PUBLIC "pch.h")
