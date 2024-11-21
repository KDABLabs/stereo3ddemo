add_subdirectory(common)

if(BUILD_QT3D_RENDERER)
    # Qt Base Application with Qt3D Renderer
    add_subdirectory(qt3d)
endif()

if(BUILD_SERENITY_RENDERER_QT)
    # Qt Base Application with Serenity Renderer
    add_subdirectory(serenity)
endif()
