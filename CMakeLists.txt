cmake_minimum_required(VERSION 3.20)

project(Allegiance VERSION 0.0.1)

include(FetchContent)

option(BUILD_QT3D_RENDERER "Build Qt3D renderer" ON)
option(BUILD_SERENITY_RENDERER "Build Serenity renderer" ON)

if(WIN32 OR APPLE)
    option(WITH_NAVLIB "Compile with support for 3DxWare Navlib" ON)
    set(WITH_SPNAV OFF)
else()
    option(WITH_SPNAV "Compile with libspnav for spacemouse support" ON)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(BUILD_SERENITY_RENDERER)
    find_package(Serenity)

    if(NOT Serenity_FOUND)
        message("Loading Serenity...")
        FetchContent_Declare(
            Serenity
            GIT_REPOSITORY "ssh://codereview.kdab.com:29418/kdab/serenity.git"
            GIT_TAG origin/master
        )
        set(SERENITY_BUILD_SHARED_LIBS OFF)
        set(SERENITY_BUILD_EXAMPLES OFF)
        set(SERENITY_BUILD_TESTS OFF)
        set(SERENITY_ENABLE_RMLUI OFF)
        FetchContent_MakeAvailable(Serenity)
    endif()

    if(NOT DEFINED CompileShaderSet)
        include(CompileShader.cmake)
    endif()
endif()

find_package(glm)
if(NOT glm_FOUND)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(glm)
endif()

set(QT_COMPONENTS Core Gui Widgets Quick)
if(BUILD_QT3D_RENDERER)
    list(
        APPEND
        QT_COMPONENTS
        3DCore
        3DExtras
        3DRender
        3DInput
    )
endif()
find_package(
    Qt6
    COMPONENTS ${QT_COMPONENTS}
    REQUIRED
)

qt_standard_project_setup()

if(BUILD_QT3D_RENDERER)
    set(POSTFIX "KDAB_SMC_OpenGL")
    set(ALLEGIANCE_UI "Qt")
    set(ALLEGIANCE_RENDERER "Qt3D")
    add_subdirectory("allegiance" ${CMAKE_CURRENT_BINARY_DIR}/allegiance_qt)
endif()

if(BUILD_SERENITY_RENDERER)
    set(POSTFIX "KDAB_SMC_Vulkan_Qt")
    set(ALLEGIANCE_UI "Qt")
    set(ALLEGIANCE_RENDERER "Serenity")
    add_subdirectory(allegiance ${CMAKE_CURRENT_BINARY_DIR}/allegiance_serenity_qt)
endif()

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL applications)
include(CPack)
