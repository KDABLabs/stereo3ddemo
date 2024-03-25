project(bridge)

set(AUTO_STEREO_SHEAR 1)
if(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
    set(RENDER_IMPL qt3d_render)
    set(AUTO_STEREO_SHEAR 0)
elseif(ALLEGIANCE_RENDERER STREQUAL "Serenity")
    set(RENDER_IMPL serenity_render)
else()
    message(FATAL_ERROR "No renderer implementation specified")
endif()

add_library(
    ${QT_TARGET_LIB} INTERFACE
)

target_include_directories(
    ${QT_TARGET_LIB}
    INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}
)
target_sources(${QT_TARGET_LIB} INTERFACE "renderer_interface_qt.h")
target_link_libraries(
    ${QT_TARGET_LIB}
    INTERFACE ${RENDER_IMPL}
)
target_compile_definitions(${QT_TARGET_LIB} INTERFACE AUTO_STEREO_SHEAR=${AUTO_STEREO_SHEAR})
