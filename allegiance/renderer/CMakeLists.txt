project(renderer)

if(BUILD_QT3D_RENDERER)
    add_subdirectory(qt3d)
endif()
if(BUILD_SERENITY_RENDERER)
    add_subdirectory(serenity)
endif()
