cmake_minimum_required(VERSION 3.20)

project(Allegiance VERSION 0.0.1)

include(FetchContent)

option(FORCE_QT "Force Use Qt3D" OFF)

if(NOT FORCE_QT)
    message("Loading Serenity...")
    FetchContent_Declare(
            Serenity
            GIT_REPOSITORY "ssh://codereview.kdab.com:29418/kdab/serenity.git"
            GIT_TAG origin/master
    )
    set(SERENITY_BUILD_EXAMPLES OFF)
    set(SERENITY_BUILD_TESTS OFF)
    set(BUILD_SHARED_LIBS_DEFAULT OFF)
    FetchContent_MakeAvailable(Serenity)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets Quick 3DCore 3DExtras 3DRender 3DInput REQUIRED)

set(POSTFIX "_qt")
set(SERENITY_BUILD 0)
add_subdirectory("allegiance" ${CMAKE_BINARY_DIR}/allegiance_qt)

if(NOT FORCE_QT)
    set(POSTFIX "_serenity")
    set(SERENITY_BUILD 1)
    add_subdirectory("allegiance" ${CMAKE_BINARY_DIR}/allegiance_serenity)
endif()
