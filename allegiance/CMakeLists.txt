project(allegiance VERSION ${PROJECT_VERSION})

add_subdirectory(renderer)

# For now let it remain like this
if(BUILD_QT_UI)
    if(BUILD_QT3D_RENDERER)
        set(POSTFIX "KDAB_SMC_OpenGL")
        set(ALLEGIANCE_RENDERER "Qt3D")
        set(QT_TARGET_LIB ${PROJECT_NAME}_${ALLEGIANCE_RENDERER}_qt)
        add_subdirectory(bridge ${CMAKE_CURRENT_BINARY_DIR}/bridge_qt3d)
        add_subdirectory(qt ${CMAKE_CURRENT_BINARY_DIR}/qt_qt3d)
    endif()
    if(BUILD_SERENITY_RENDERER)
        set(POSTFIX "KDAB_SMC_Vulkan_Qt")
        set(ALLEGIANCE_RENDERER "Serenity")
        set(QT_TARGET_LIB ${PROJECT_NAME}_${ALLEGIANCE_RENDERER}_qt)
        add_subdirectory(bridge ${CMAKE_CURRENT_BINARY_DIR}/bridge_serenity)
        add_subdirectory(qt ${CMAKE_CURRENT_BINARY_DIR}/qt_serenity)
    endif()
endif()

#add_executable(
#    ${PROJECT_NAME} MACOSX_BUNDLE
#)
#
#if (WIN32)
#    set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE $<CONFIG:Release>)
#
#    if (MSVC AND NOT MSVC_VERSION VERSION_LESS 142)
#        target_link_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/INCREMENTAL>)
#        target_compile_options(${PROJECT_NAME} PRIVATE $<$<CONFIG:Debug>:/ZI>)
#
#    endif()
#endif()
#
#if(ALLEGIANCE_UI STREQUAL "Qt")
#    set(ALLEGIANCE_SOURCES
#        ../assets/resources.qrc
#        src/entry_main_qt.cpp
#        "include/qt/app.h"
#
#        include/qt/stereo_camera.h
#        include/window.h
#        include/sliders.h
#        include/camera_control.h
#        include/vertical_label.h
#        include/cursor.h
#        include/cursor.cpp
#        include/ui/style.h
#        include/ui/camera_controller.h
#    )
#    target_link_libraries(${PROJECT_NAME} PUBLIC Shared)
#elseif(ALLEGIANCE_UI STREQUAL "Flutter")
#    set(ALLEGIANCE_SOURCES src/entry_main_flutter.cpp)
#endif()
#
#if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
#    find_package(assimp QUIET)
#    if(NOT assimp_FOUND)
#        FetchContent_Declare(
#            assimp
#            GIT_REPOSITORY https://github.com/assimp/assimp.git
#            GIT_TAG master
#        )
#
#        set(BUILD_SHARED_LIBS OFF)
#        FetchContent_MakeAvailable(assimp)
#    endif()
#
#    list(
#        APPEND
#        ALLEGIANCE_SOURCES
#
#        include/serenity_impl.h
#        include/serenity_stereo_graph.h
#        src/serenity_stereo_graph.cpp
#        include/serenity/cursor.h
#        include/serenity/mesh_loader.h
#        include/serenity/serenity_qt.h
#        include/picking_application_layer.h
#        src/picking_application_layer.cpp
#        include/window_extent_watcher.h
#        src/window_extent_watcher.cpp
#        src/serenity/mesh_loader.cpp
#      )
#elseif(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
#    set(ALLEGIANCE_QT3D TRUE)
#
#
#    list(
#        APPEND
#        ALLEGIANCE_SOURCES
#        include/qt3d/qt3d_impl.h
#        include/qt3d/qt3d_shaders.h
#        include/qt3d/qt3d_materials.h
#        include/qt3d/qt3d_cursor.h
#        include/qt3d/stereo_image_mesh.h
#        include/qt3d/stereo_image_material.h
#        src/stereo_image_mesh.cpp
#        src/stereo_image_material.cpp
#    )
#endif()
#
#if(ALLEGIANCE_RENDERER STREQUAL "Serenity" AND ALLEGIANCE_UI STREQUAL "Qt")
#    list(APPEND ALLEGIANCE_SOURCES include/serenity/serenity_qt.h include/serenity_impl_qt.h)
#endif()
#
##target_sources(${PROJECT_NAME} PRIVATE ${ALLEGIANCE_SOURCES})
#
#if(ALLEGIANCE_UI STREQUAL "Qt")
#    target_link_libraries(
#        ${PROJECT_NAME}
#        PRIVATE Qt6::Core
#                Qt6::Gui
#                Qt6::Widgets
#                Qt6::Quick
#                glm::glm
#                Qt6::QuickWidgets
#                Qt6::QuickControls2
#    )
#endif()
#if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
#    target_link_libraries(${PROJECT_NAME} PRIVATE KDGpu::KDGpu Serenity::Core Serenity::Gui Serenity::Logic assimp::assimp)
#elseif(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
#    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::3DExtras)
#endif()
#
#set(files_to_copy
#    cottage.obj
#    cottage.mtl
#    cottage_diffuse.png
#    terrain.obj
#    terrain.png
#    motorbike.fbx
#    motorbike.mtl
#    motorbike.obj
#    fbx/showroom2303.fbx
#    13_3840x2160_sbs.jpg
#    cursor_billboard.png
#)
#
#foreach(file ${files_to_copy})
#    set(src "${CMAKE_SOURCE_DIR}/assets/${file}")
#    set(dest "${SCENE_DIR}/${file}")
#
#    add_custom_command(
#        TARGET ${PROJECT_NAME}
#        POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${src}" "${dest}"
#    )
#endforeach()
#
#install(
#    DIRECTORY ${SCENE_DIR}
#    DESTINATION ${CMAKE_INSTALL_BINDIR}
#    COMPONENT applications
#)
#
#if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
#    add_dependencies(${PROJECT_NAME} Shaders)
#endif()
#
#target_include_directories(
#    ${PROJECT_NAME}
#    PRIVATE "include"
#)
#set_target_properties(
#    ${PROJECT_NAME}
#    PROPERTIES CXX_STANDARD 20
#               AUTOMOC ON
#               AUTORCC ON
#)
#target_precompile_headers(${PROJECT_NAME} PRIVATE "include/pch.h")
#if(ALLEGIANCE_UI STREQUAL "Qt")
#    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_KEYWORDS)
#    qt_add_qml_module(${PROJECT_NAME}
#        URI Schneider
#        NO_RESOURCE_TARGET_PATH
#        RESOURCES ./resources/checked.svg ./resources/unchecked.svg
#        QML_FILES
#        resources/camera_control.qml
#        resources/MainMenu.qml
#        resources/PageBase.qml
#        resources/SliderValue.qml
#        resources/CheckBoxX.qml
#        resources/ButtonX.qml )
#elseif(ALLEGIANCE_UI STREQUAL "Flutter")
#    target_compile_definitions(${PROJECT_NAME} PRIVATE FLUTTER_UI_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/flutter")
#endif()
#if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
#    target_compile_definitions(${PROJECT_NAME} PRIVATE ALLEGIANCE_SERENITY=1)
#elseif(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
#
#    target_compile_definitions(${PROJECT_NAME} PRIVATE ALLEGIANCE_SERENITY=0)
#endif()
#if(ALLEGIANCE_UI STREQUAL "Flutter")
#    # Flutter overlay doesn't render in stereo mode
#    target_compile_definitions(${PROJECT_NAME} PRIVATE RENDER_MODE_LEFT_ONLY)
#endif()
#
#install(
#    TARGETS ${PROJECT_NAME}
#    BUNDLE DESTINATION .
#    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications
#)
#
#
#if(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
#    qt_finalize_target(${PROJECT_NAME})
#    qt_generate_deploy_qml_app_script(
#            TARGET ${PROJECT_NAME}
#            OUTPUT_SCRIPT deploy_script
#            MACOS_BUNDLE_POST_BUILD
#            NO_UNSUPPORTED_PLATFORM_ERROR
#            DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
#    )
#    install(SCRIPT ${deploy_script} COMPONENT applications)
#endif()
#
#if(ALLEGIANCE_UI STREQUAL "Flutter")
#    add_custom_command(
#        TARGET ${PROJECT_NAME}
#        POST_BUILD
#        COMMAND ${FLUTTER_EXE} build bundle
#        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/flutter/ui"
#    )
#
#    get_filename_component(FLUTTER_SDK_PATH ${FLUTTER_EXE} PATH)
#    file(GLOB_RECURSE ICUDTL "${FLUTTER_SDK_PATH}/icudtl.dat")
#    if(ICUDTL)
#        add_custom_command(
#            TARGET ${PROJECT_NAME}
#            POST_BUILD
#            COMMAND ${CMAKE_COMMAND} -E copy ${ICUDTL} "${CMAKE_CURRENT_SOURCE_DIR}/flutter/"
#        )
#    endif()
#endif()
#
