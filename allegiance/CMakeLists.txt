project(${POSTFIX} VERSION ${PROJECT_VERSION})

add_executable(
    ${PROJECT_NAME} MACOSX_BUNDLE
)

if(ALLEGIANCE_UI STREQUAL "Qt")
    set(ALLEGIANCE_SOURCES
        ../shared/resources.qrc
        src/entry_main_qt.cpp
        include/app.h
        include/window.h
        include/sliders.h
        include/camera_control.h
        include/stereo_camera.h
        include/vertical_label.h
        include/spacemouse.h
        include/spacemouse.cpp
        include/cursor.h
        include/cursor.cpp
    )
elseif(ALLEGIANCE_UI STREQUAL "Flutter")
    set(ALLEGIANCE_SOURCES src/entry_main_flutter.cpp)
endif()

if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
    list(
        APPEND
        ALLEGIANCE_SOURCES
        include/serenity_impl.h
        include/serenity_stereo_graph.h
        src/serenity_stereo_graph.cpp
        include/serenity/cursor.h
        include/picking_application_layer.h
        src/picking_application_layer.cpp
        include/window_extent_watcher.h
        src/window_extent_watcher.cpp
    )
elseif(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
    list(
        APPEND
        ALLEGIANCE_SOURCES
        include/qt3d_impl.h
        include/qt3d_shaders.h
        include/qt3d_materials.h
        include/qt3d_cursor.h
        include/stereo_image_mesh.h
        include/stereo_image_material.h
        src/stereo_image_mesh.cpp
        src/stereo_image_material.cpp
    )
endif()

if(ALLEGIANCE_RENDERER STREQUAL "Serenity" AND ALLEGIANCE_UI STREQUAL "Qt")
    list(APPEND ALLEGIANCE_SOURCES include/serenity/serenity_qt.h include/serenity_impl_qt.h)
endif()

target_sources(${PROJECT_NAME} PRIVATE ${ALLEGIANCE_SOURCES})

if(ALLEGIANCE_UI STREQUAL "Qt")
    target_link_libraries(
        ${PROJECT_NAME}
        PRIVATE Qt6::Core
                Qt6::Gui
                Qt6::Widgets
                Qt6::Quick
                glm::glm
    )
endif()
if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
    target_link_libraries(${PROJECT_NAME} PRIVATE KDGpu::KDGpu Serenity::Core Serenity::Gui Serenity::Logic)
elseif(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::3DExtras)
endif()

if(${WITH_NAVLIB})
    set(TDxWare_SDK_DIR "${CMAKE_SOURCE_DIR}/3rdparty/3DxWare_SDK_v4-0-3_r19962/")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DWITH_NAVLIB) # -DTRACE_NAVLIB

    find_package(TDxWare_SDK 4.0 REQUIRED COMPONENTS 3DxWare::Navlib)
    target_include_directories(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:3DxWare::Navlib,INTERFACE_INCLUDE_DIRECTORIES>)

    #target_link_options(${PROJECT_NAME} PRIVATE "/ignore:4099")
    target_link_libraries(${PROJECT_NAME} PRIVATE $<TARGET_PROPERTY:3DxWare::Navlib,INTERFACE_LINK_LIBRARIES>)
endif()

if(${WITH_SPNAV})
    target_compile_definitions(${PROJECT_NAME} PRIVATE -DWITH_SPNAV)
    target_link_libraries(${PROJECT_NAME} PRIVATE spnav)
endif()

set(files_to_copy
    cottage.obj
    cottage_diffuse.png
    terrain.obj
    terrain.png
    motorbike.fbx
    motorbike.mtl
    motorbike.obj
    fbx/showroom2303.fbx
    13_3840x2160_sbs.jpg
    cursor_billboard.png
)

foreach(file ${files_to_copy})
    set(src "${CMAKE_SOURCE_DIR}/shared/${file}")
    set(dest "${SCENE_DIR}/${file}")

    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different "${src}" "${dest}"
    )
endforeach()

install(
    DIRECTORY ${SCENE_DIR}
    DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT applications
)

if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
    add_dependencies(${PROJECT_NAME} Shaders)
endif()

target_include_directories(
    ${PROJECT_NAME}
    PRIVATE "include"
)
set_target_properties(
    ${PROJECT_NAME}
    PROPERTIES CXX_STANDARD 20
               AUTOMOC ON
               AUTORCC ON
)
target_precompile_headers(${PROJECT_NAME} PRIVATE "include/pch.h")
execute_process(
    COMMAND git log -1 --format=%cd --date=format:%Y-%m-%d
    OUTPUT_VARIABLE GIT_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
target_compile_definitions(${PROJECT_NAME} PRIVATE ALLEGIANCE_BUILD_STR="v${PROJECT_VERSION} - ${GIT_COMMIT_DATE}")
if(ALLEGIANCE_UI STREQUAL "Qt")
    target_compile_definitions(${PROJECT_NAME} PRIVATE QT_NO_KEYWORDS)
elseif(ALLEGIANCE_UI STREQUAL "Flutter")
    target_compile_definitions(${PROJECT_NAME} PRIVATE FLUTTER_UI_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/flutter")
endif()
if(ALLEGIANCE_RENDERER STREQUAL "Serenity")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALLEGIANCE_SERENITY=1)
elseif(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
    target_compile_definitions(${PROJECT_NAME} PRIVATE ALLEGIANCE_SERENITY=0)
endif()
if(ALLEGIANCE_UI STREQUAL "Flutter")
    # Flutter overlay doesn't render in stereo mode
    target_compile_definitions(${PROJECT_NAME} PRIVATE RENDER_MODE_LEFT_ONLY)
endif()

install(
    TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT applications
)

if(ALLEGIANCE_RENDERER STREQUAL "Qt3D")
    qt_generate_deploy_app_script(TARGET ${PROJECT_NAME} OUTPUT_SCRIPT deploy_script)

    #    qt_generate_deploy_qml_app_script(
    #        TARGET ${PROJECT_NAME}
    #        OUTPUT_SCRIPT deploy_script
    #        MACOS_BUNDLE_POST_BUILD
    #        NO_UNSUPPORTED_PLATFORM_ERROR
    #        DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
    #    )
    install(SCRIPT ${deploy_script} COMPONENT applications)
endif()

if(ALLEGIANCE_UI STREQUAL "Flutter")
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND ${FLUTTER_EXE} build bundle
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/flutter/ui"
    )

    get_filename_component(FLUTTER_SDK_PATH ${FLUTTER_EXE} PATH)
    file(GLOB_RECURSE ICUDTL "${FLUTTER_SDK_PATH}/icudtl.dat")
    if(ICUDTL)
        add_custom_command(
            TARGET ${PROJECT_NAME}
            POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy ${ICUDTL} "${CMAKE_CURRENT_SOURCE_DIR}/flutter/"
        )
    endif()
endif()
