project(allegiance${POSTFIX})



add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
        PRIVATE
                "../shared/resources.qrc"
                "src/entry_main.cpp"
                "include/app.h"
                "include/window.h"
                "include/serenity.h"
                "include/serenity_impl.h"
                "include/sliders.h"
                "include/camera_control.h"
                "include/qt3d_impl.h"
)
target_link_libraries(${PROJECT_NAME}
        PRIVATE
                Qt6::Core
                Qt6::Gui
                Qt6::Widgets
                Qt6::3DExtras
                Qt6::Quick
)
if(SERENITY_BUILD)
    CompileShaderSet(${PROJECT_NAME} multiview-scene)
    set(RESOURCE_FILES
        "${CMAKE_CURRENT_BINARY_DIR}/multiview-scene.vert.spv"
        "${CMAKE_CURRENT_BINARY_DIR}/multiview-scene.frag.spv"
    )
    install(FILES ${RESOURCE_FILES}
            DESTINATION ${CMAKE_INSTALL_BINDIR}
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
)

if(WIN32 AND SERENITY_BUILD)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Serenity::Gui>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Serenity::Logic>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:KDGpu::KDGpu>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:KDGui>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:Serenity::Core>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:KDFoundation>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:KDUtils>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:spdlog::spdlog>
            ${CMAKE_CURRENT_BINARY_DIR})
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_FILE:KDGpu::KDGpuKDGui>
            ${CMAKE_CURRENT_BINARY_DIR})
endif()

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
