add_subdirectory(common)

if(BUILD_QT3D_RENDERER)
    # Qt Base Application with Qt3D Renderer
    add_subdirectory(qt3d)
endif()

message(STATUS "Qt Vulkan Feature: ${QT_FEATURE_vulkan}")
if(BUILD_SERENITY_RENDERER_QT AND QT_FEATURE_vulkan)

    # Qt Base Application with Serenity Renderer
    add_subdirectory(serenity)
endif()
