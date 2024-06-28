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
    if(BUILD_SERENITY_RENDERER_QT)
        set(POSTFIX "KDAB_SMC_Vulkan_Qt")
        set(ALLEGIANCE_RENDERER "Serenity")
        set(QT_TARGET_LIB ${PROJECT_NAME}_${ALLEGIANCE_RENDERER}_qt)
        add_subdirectory(bridge ${CMAKE_CURRENT_BINARY_DIR}/bridge_serenity)
        add_subdirectory(qt ${CMAKE_CURRENT_BINARY_DIR}/qt_serenity)

        if(BUILD_FLUTTER_UI)
            message(STATUS "Flutter SDK: ${FLUTTER_EXE}")

            add_custom_target(FlutterAssetBuild
                COMMAND ${FLUTTER_EXE} build bundle
                WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/flutter/ui"
            )
            add_dependencies(serenity_render FlutterAssetBuild)

            get_filename_component(FLUTTER_SDK_PATH ${FLUTTER_EXE} PATH)
            file(GLOB_RECURSE ICUDTL "${FLUTTER_SDK_PATH}/icudtl.dat")
            message(STATUS "flutter icudtl.dat file: ${ICUDTL}")

            if(ICUDTL)
                add_custom_target(FlutterICUDTLDeploy
                    COMMAND ${CMAKE_COMMAND} -E copy ${ICUDTL} "${CMAKE_CURRENT_SOURCE_DIR}/flutter/ui"
                )
                add_dependencies(serenity_render FlutterICUDTLDeploy)
            endif()

            target_compile_definitions(
              serenity_render PUBLIC FLUTTER_UI_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/flutter/ui")
        endif()

    endif()
endif()

if(BUILD_SERENITY_RENDERER_KDGUI)
    set(POSTFIX "KDAB_SMC_Vulkan_KDGui")
    set(ALLEGIANCE_RENDERER "Serenity")
    add_subdirectory(kdgui ${CMAKE_CURRENT_BINARY_DIR}/kdgui_serenity)
endif()
