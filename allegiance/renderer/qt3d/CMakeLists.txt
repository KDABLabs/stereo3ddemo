project(KDAB_Qt3D_Renderer)

#We need to add Qml here even we dont need it, due to a cpack Qt bug
find_package(
    Qt6
    COMPONENTS Core
               3DExtras
               3DCore
               3DRender
               3DInput
               Qml
    REQUIRED
)

add_library(
    ${PROJECT_NAME} STATIC
)
add_library(
    KDAB::Qt3DRenderer ALIAS ${PROJECT_NAME}
)

target_compile_definitions(${PROJECT_NAME} PUBLIC ALLEGIANCE_QT3D)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}
)

target_sources(
    ${PROJECT_NAME}
    PUBLIC qt3d_renderer.h
           qt3d_cursor.h
           qt3d_materials.h
           qt3d_shaders.h
           qt3d_focusarea.h
           util_qt.h
           stereo_forward_renderer.h
           stereo_proxy_camera.h
           stereo_image_mesh.h
           stereo_image_material.h
           frame_action.h
           mesh_loader.h
           scene_mesh.h
           frustum.h
           frustum_rect.h
           focus_plane_preview.h

    PRIVATE qt3d_renderer.cpp
            qt3d_cursor.cpp
            qt3d_materials.cpp
            qt3d_focusarea.cpp
            mesh_loader.cpp
            scene_mesh.cpp
            stereo_image_material.cpp
            stereo_image_mesh.cpp
            stereo_forward_renderer.cpp
            stereo_proxy_camera.cpp
            frustum.cpp
            frustum_rect.cpp
            focus_plane_preview.cpp
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC Qt6::Core
           Qt6::3DExtras
           Qt6::3DCore
           Qt6::3DRender
           Qt6::3DInput
           shared
           assimp::assimp
)

set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20 AUTOMOC ON)
