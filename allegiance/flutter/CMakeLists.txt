if(BUILD_FLUTTER_UI AND BUILD_SERENITY_RENDERER)
    message(STATUS "Flutter SDK: ${FLUTTER_EXE}")

    add_custom_target(
        FlutterAssetBuild
        COMMAND ${FLUTTER_EXE} build bundle
        WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/ui"
    )
    add_dependencies(KDAB_Serenity_Renderer FlutterAssetBuild)

    get_filename_component(FLUTTER_SDK_PATH ${FLUTTER_EXE} PATH)
    file(GLOB_RECURSE ICUDTL "${FLUTTER_SDK_PATH}/icudtl.dat")
    message(STATUS "flutter icudtl.dat file: ${ICUDTL}")

    if(ICUDTL)
        add_custom_target(
            FlutterICUDTLDeploy COMMAND ${CMAKE_COMMAND} -E copy ${ICUDTL} "${CMAKE_CURRENT_SOURCE_DIR}/ui"
        )
        add_dependencies(KDAB_Serenity_Renderer FlutterICUDTLDeploy)
    endif()

    target_compile_definitions(KDAB_Serenity_Renderer PUBLIC FLUTTER_UI_ASSET_DIR="${CMAKE_CURRENT_SOURCE_DIR}/ui")
endif()
