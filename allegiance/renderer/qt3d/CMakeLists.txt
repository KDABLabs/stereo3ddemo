project(qt3d_render)

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

target_compile_definitions(${PROJECT_NAME} PUBLIC ALLEGIANCE_QT3D)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CMAKE_SOURCE_DIR}/allegiance/qt
)

target_sources(
    ${PROJECT_NAME}
    PUBLIC "qt3d_impl.h"
           "qt3d_cursor.h"
           "qt3d_materials.h"
           "qt3d_shaders.h"
           "util_qt.h"
           "stereo_forward_renderer.h"
           "stereo_proxy_camera.h"
           "stereo_image_mesh.h"
           "stereo_image_material.h"
           "frame_action.h"
           "mesh_loader.h"
           "scene_mesh.h"
    PRIVATE "qt3d_impl.cpp"
            "mesh_loader.cpp"
            "scene_mesh.cpp"
            "qt3d_materials.cpp"
            "stereo_image_material.cpp"
            "stereo_image_mesh.cpp"
            "qt3d_cursor.cpp"
            "stereo_forward_renderer.cpp"
            "stereo_proxy_camera.cpp"
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
