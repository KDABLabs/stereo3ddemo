project(allegiance VERSION ${PROJECT_VERSION})

# Build Qt3D/Serenity Renderers
add_subdirectory(renderer)

# Build different applications for different Renderers / UI Frameworks
add_subdirectory(applications)

# Flutter UI
add_subdirectory(flutter)
