cmake_minimum_required(VERSION 3.20)

project(Allegiance VERSION 0.2.4)

execute_process(
    COMMAND git log -1 --format=%cd --date=format:%Y-%m-%d
    OUTPUT_VARIABLE GIT_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(-DALLEGIANCE_PROJECT_VERSION="${PROJECT_VERSION}"
                -DALLEGIANCE_BUILD_STR="v${PROJECT_VERSION} - ${GIT_COMMIT_DATE}")


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

set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL applications)
include(CPack)
