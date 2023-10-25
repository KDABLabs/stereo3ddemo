project(allegiance${POSTFIX})

find_package(glm QUIET)

if (NOT glm_FOUND)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG        master
    )
    FetchContent_MakeAvailable(glm)
endif()

add_executable(${PROJECT_NAME} MACOSX_BUNDLE)
target_sources(${PROJECT_NAME}
        PRIVATE
                "../shared/resources.qrc"
                "src/entry_main.cpp"
                "include/app.h"
                "include/window.h"
                "include/serenity/serenity.h"
                "include/serenity_impl.h"
                "include/sliders.h"
                "include/camera_control.h"
                "include/stereo_camera.h"
                "include/qt3d_impl.h"
                "include/vertical_label.h"
                "include/stereo_image_material.h"
                "include/stereo_image_mesh.h"
                "src/stereo_image_mesh.cpp"
                "src/stereo_image_material.cpp"
		"include/serenity_stereo_graph.h"
		"src/serenity_stereo_graph.cpp"
                "include/serenity/cursor.h"
		"include/qt3d_shaders.h"
		"include/qt3d_materials.h"
)

target_link_libraries(${PROJECT_NAME}
        PRIVATE
                Qt6::Core
                Qt6::Gui
                Qt6::Widgets
                Qt6::3DExtras
                Qt6::Quick
                glm::glm
)
if(SERENITY_BUILD)
    set(SCENE_DIR "${CMAKE_CURRENT_BINARY_DIR}/../scene")
    file(MAKE_DIRECTORY ${SCENE_DIR})

    add_subdirectory("serenity")
    add_dependencies(${PROJECT_NAME} Shaders)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../shared/cottage.obj"
            "${SCENE_DIR}/cottage.obj"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../shared/cottage_diffuse.png"
            "${SCENE_DIR}/cottage_diffuse.png"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../shared/cursor_billboard.png"
            "${SCENE_DIR}/cursor_billboard.png"
    )

    set(SCENE_FILES
        "${SCENE_DIR}/cottage.obj"
        "${SCENE_DIR}/cottage_diffuse.png"
        "${SCENE_DIR}/cursor_billboard.png"
    )
    install(FILES ${SCENE_FILES}
            DESTINATION ${CMAKE_INSTALL_BINDIR}/scene
            COMPONENT applications)
endif()
if(NOT FORCE_QT)
    target_link_libraries(${PROJECT_NAME}
            PRIVATE
                    KDGpu::KDGpu
                    Serenity::Core
                    Serenity::Gui
                    Serenity::Logic
    )
endif()
target_include_directories(${PROJECT_NAME}
    PRIVATE
        "include"
)
set_target_properties(${PROJECT_NAME}
    PROPERTIES
        CXX_STANDARD 20
        AUTOMOC ON
        AUTORCC ON
)
target_precompile_headers(${PROJECT_NAME}
    PRIVATE
        "include/pch.h"
)
target_compile_definitions(${PROJECT_NAME}
    PRIVATE
        ALLEGIANCE_SERENITY=${SERENITY_BUILD}
        QT_NO_KEYWORDS
)

install(TARGETS ${PROJECT_NAME}
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    COMPONENT applications
)

if (NOT SERENITY_BUILD)
    qt_generate_deploy_app_script(
        TARGET ${PROJECT_NAME}
        OUTPUT_SCRIPT deploy_script
    )

#    qt_generate_deploy_qml_app_script(
#        TARGET ${PROJECT_NAME}
#        OUTPUT_SCRIPT deploy_script
#        MACOS_BUNDLE_POST_BUILD
#        NO_UNSUPPORTED_PLATFORM_ERROR
#        DEPLOY_USER_QML_MODULES_ON_UNSUPPORTED_PLATFORM
#    )
    install(SCRIPT ${deploy_script} COMPONENT applications)
endif()
