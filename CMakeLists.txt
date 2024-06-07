cmake_minimum_required(VERSION 3.20)

project(Allegiance VERSION 0.2.3)

add_definitions(-DALLEGIANCE_PROJECT_VERSION="${CMAKE_PROJECT_VERSION}")

include(FetchContent)
include(CMakeDependentOption)

option(BUILD_QT_UI "Build Qt UI" ON)
option(BUILD_FLUTTER_UI "Build Flutter UI" OFF)

option(BUILD_QT3D_RENDERER "Build Qt3D renderer" ON)
option(BUILD_SERENITY_RENDERER "Build Serenity renderer" ON)

if(NOT BUILD_QT_UI)
    if(BUILD_QT3D_RENDERER)
        set(BUILD_QT3D_RENDERER OFF)
        message(STATUS "Qt3D renderer requires Qt UI")
    endif()
endif()

if(NOT BUILD_SERENITY_RENDERER)
    if(BUILD_FLUTTER_UI)
        set(BUILD_FLUTTER_UI OFF)
        message(STATUS "Flutter UI requires Serenity renderer")
    endif()
endif()

if(WIN32 OR APPLE)
    option(WITH_NAVLIB "Compile with support for 3DxWare Navlib" ON)
    set(WITH_SPNAV OFF)
else()
    find_library(SPNAV_LIB spnav)
    cmake_dependent_option(WITH_SPNAV "Compile with libspnav for spacemouse support" ON "SPNAV_LIB" OFF)
endif()

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# Projects
find_package(glm)
if(NOT glm_FOUND AND NOT BUILD_SERENITY_RENDERER)
    FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG master
    )
    FetchContent_MakeAvailable(glm)
endif()

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets)

# Not working yet
if(BUILD_FLUTTER_UI
   AND FLUTTER_EXE_FOUND
   AND BUILD_SERENITY_RENDERER
)
    set(POSTFIX "KDAB_SMC_Vulkan_Flutter")
    set(ALLEGIANCE_UI "Flutter")
    set(ALLEGIANCE_RENDERER "Serenity")
endif()

add_subdirectory(shared)
add_subdirectory(allegiance)

#if(BUILD_SERENITY_RENDERER)
#    find_package(Serenity)
#    if(BUILD_FLUTTER_UI)
#        find_package(nlohmann_json REQUIRED)
#    endif()
#
#    if(NOT Serenity_FOUND)
#        find_package(spdlog)
#        message("Loading Serenity...")
#        FetchContent_Declare(
#            Serenity
#            GIT_REPOSITORY "ssh://codereview.kdab.com:29418/kdab/serenity.git"
#            GIT_TAG origin/master
#        )
#        set(SERENITY_BUILD_SHARED_LIBS OFF)
#        set(SERENITY_BUILD_EXAMPLES OFF)
#        set(SERENITY_BUILD_TESTS OFF)
#        set(SERENITY_ENABLE_RMLUI OFF)
#        FetchContent_MakeAvailable(Serenity)
#    endif()
#
#    if(NOT DEFINED CompileShaderSet)
#        include(CompileShader.cmake)
#    endif()
#
#    if(BUILD_FLUTTER_UI)
#        find_program(FLUTTER_EXE flutter)
#        if(NOT FLUTTER_EXE STREQUAL "FLUTTER_EXE-NOTFOUND")
#            set(FLUTTER_EXE_FOUND TRUE)
#        else()
#            set(FLUTTER_EXE_FOUND FALSE)
#        endif()
#        if(FLUTTER_EXE_FOUND)
#            message(STATUS "Flutter found: ${FLUTTER_EXE}")
#        else()
#            message(STATUS "Flutter not found, Flutter UI will not be built")
#            set(BUILD_FLUTTER_UI OFF)
#        endif()
#    endif()
#endif()
#
#
#
#if(BUILD_QT_UI)
#    set(QT_COMPONENTS Core Gui Widgets Quick QuickWidgets QuickControls2)
#    if(BUILD_QT3D_RENDERER)
#        list(
#            APPEND
#            QT_COMPONENTS
#            3DCore
#            3DExtras
#            3DRender
#            3DInput
#        )
#    endif()
#    find_package(
#        Qt6
#        COMPONENTS ${QT_COMPONENTS}
#        REQUIRED
#    )
#    qt_standard_project_setup()
#endif()
#
#set(SCENE_DIR ${CMAKE_CURRENT_BINARY_DIR}/scene)
#file(MAKE_DIRECTORY ${SCENE_DIR})
#
#if(BUILD_SERENITY_RENDERER)
#    #add_subdirectory(allegiance/serenity)
#endif()
#

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL applications)
include(CPack)
