cmake_minimum_required(VERSION 3.20)

project(Allegiance VERSION 0.3.0)

execute_process(
    COMMAND git log -1 --format=%cd --date=format:%Y-%m-%d
    OUTPUT_VARIABLE GIT_COMMIT_DATE
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
add_definitions(
    -DALLEGIANCE_PROJECT_VERSION="${PROJECT_VERSION}" -DALLEGIANCE_BUILD_STR="v${PROJECT_VERSION} - ${GIT_COMMIT_DATE}"
)

include(FetchContent)
include(CMakeDependentOption)
include(FeatureSummary)

find_program(FLUTTER_EXE flutter)


option(BUILD_UNIT_TESTS "Build Unit Tests" ON)
option(BUILD_QT_UI "Build Qt UI" ON)

cmake_dependent_option(
    BUILD_QT3D_RENDERER
    "Build Qt3D renderer"
    ON
    "BUILD_QT_UI"
    OFF
)

option(BUILD_KDGUI_UI "Build KDGui UI" ON)

option(BUILD_SERENITY_RENDERER "Build Serenity renderer" ON)
cmake_dependent_option(
    BUILD_FLUTTER_UI
    "Build Flutter UI"
    ON
    "BUILD_SERENITY_RENDERER;FLUTTER_EXE"
    OFF
)

if(WIN32 OR APPLE)
    option(WITH_NAVLIB "Compile with support for 3DxWare Navlib" ON)
    set(WITH_SPNAV OFF)
else()
    find_library(SPNAV_LIB spnav)
    cmake_dependent_option(
        WITH_SPNAV
        "Compile with libspnav for spacemouse support"
        ON
        "SPNAV_LIB"
        OFF
    )
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

set(BUILD_SERENITY_RENDERER_QT ${BUILD_SERENITY_RENDERER})
set(BUILD_SERENITY_RENDERER_KDGUI ${BUILD_SERENITY_RENDERER})

file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/assets)

add_subdirectory(shared)
add_subdirectory(allegiance)

if(BUILD_UNIT_TESTS)
    add_subdirectory(tests)
endif()

if(WIN32)
    set(CPACK_GENERATOR "ZIP;NSIS")
else()
    set(CPACK_GENERATOR "ZIP")
endif()
set(CPACK_COMPONENTS_GROUPING ALL_COMPONENTS_IN_ONE)
set(CPACK_ARCHIVE_COMPONENT_INSTALL ON)
set(CPACK_COMPONENTS_ALL applications)
include(CPack)

set(BUILD_SERENITY_QT_DEMO  ${BUILD_SERENITY_RENDERER} AND ${BUILD_QT_UI})
set(BUILD_QT3D_DEMO  ${BUILD_QT3D_RENDERER} AND ${BUILD_QT_UI})
add_feature_info(Serenity-Demo-Qt BUILD_SERENITY_QT_DEMO
                 "Serenity (Vulkan) Demo with Qt based UI")
add_feature_info(Qt3D-Demo BUILD_QT3D_DEMO
                 "Qt3D (OpenGL) Demo with Qt based UI")

feature_summary(WHAT PACKAGES_FOUND ENABLED_FEATURES PACKAGES_NOT_FOUND
                     DISABLED_FEATURES INCLUDE_QUIET_PACKAGES)
