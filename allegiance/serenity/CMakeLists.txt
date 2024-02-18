project("Shaders")

add_custom_target(${PROJECT_NAME} ALL)

CompileShaderSet(serenity_multiview multiview)
CompileShaderSet(serenity_multiview2 multiview.dif)
CompileShaderSet(serenity_bb billboard)
CompileShaderSet(serenity_color color)
CompileShaderSet(serenity_stereoimage stereoimage)
add_dependencies(
    ${PROJECT_NAME}
    serenity_bbShaders
    serenity_colorShaders
    serenity_multiviewShaders
    serenity_multiview2Shaders
    serenity_stereoimageShaders
)

set(COMP_SHADERS
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
)

add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${COMP_SHADERS} ${SCENE_DIR} DEPENDS serenity_bbShaders
            serenity_colorShaders serenity_multiviewShaders
)

set(SHADER_FILES
    "${SCENE_DIR}/multiview.dif.vert.spv"
    "${SCENE_DIR}/multiview.dif.frag.spv"
    "${SCENE_DIR}/multiview.vert.spv"
    "${SCENE_DIR}/multiview.frag.spv"
    "${SCENE_DIR}/color.vert.spv"
    "${SCENE_DIR}/color.frag.spv"
    "${SCENE_DIR}/billboard.vert.spv"
    "${SCENE_DIR}/billboard.frag.spv"
    "${SCENE_DIR}/stereoimage.vert.spv"
    "${SCENE_DIR}/stereoimage.frag.spv"
)
install(
    FILES ${SHADER_FILES}
    DESTINATION ${CMAKE_INSTALL_BINDIR}/scene
    COMPONENT applications
)
