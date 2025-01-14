project(shaders)

add_custom_target(${PROJECT_NAME} ALL)

CompileShaderSet(serenity_multiview multiview)
CompileShaderSet(serenity_multiview2 multiview.dif)
CompileShaderSet(serenity_bb billboard)
CompileShaderSet(serenity_color color)
CompileShaderSet(serenity_stereoimage stereoimage)
CompileShaderSet(serenity_focusplane focus_plane)
CompileShaderSet(serenity_focusarea focus_area)
CompileShaderSet(serenity_frustumtris frustum_tris)
CompileShaderSet(serenity_frustumlines frustum_lines)
CompileShaderSet(serenity_frustumrect frustum_rect)

add_dependencies(
    ${PROJECT_NAME}
    serenity_bbShaders
    serenity_colorShaders
    serenity_multiviewShaders
    serenity_multiview2Shaders
    serenity_stereoimageShaders
    serenity_focusplaneShaders
    serenity_focusareaShaders
    serenity_frustumtrisShaders
    serenity_frustumlinesShaders
    serenity_frustumrectShaders
)

set(SHADER_FILES
    "${CMAKE_CURRENT_BINARY_DIR}/multiview.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/multiview.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/multiview.dif.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/multiview.dif.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/color.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/color.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/billboard.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/billboard.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/stereoimage.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/stereoimage.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/focus_plane.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/focus_plane.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/focus_area.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/focus_area.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/frustum_lines.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/frustum_lines.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/frustum_tris.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/frustum_tris.frag.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/frustum_rect.vert.spv"
    "${CMAKE_CURRENT_BINARY_DIR}/frustum_rect.frag.spv"
)

file(MAKE_DIRECTORY ${CMAKE_BINARY_DIR}/serenity/shaders)
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SHADER_FILES} ${CMAKE_BINARY_DIR}/serenity/shaders DEPENDS
            serenity_bbShaders serenity_colorShaders serenity_multiviewShaders
)

install(
    FILES ${SHADER_FILES}
    DESTINATION ${CMAKE_INSTALL_BINDIR}/serenity/shaders
    COMPONENT applications
)
