cmake_minimum_required(VERSION 3.20)

project(Allegiance VERSION 0.0.1)

option(USE_QTVER "Use Qt version" OFF)

set(DEF_QTVER "6" CACHE INTERNAL "Default Qt version")
set(QTVER_LIST "5;6" CACHE INTERNAL "List of possible Qt versions")
set(ALL_QTVER ${DEF_QTVER} CACHE STRING "Used Qt version")
set_property(CACHE ALL_QTVER PROPERTY STRINGS ${QTVER_LIST})

if(NOT USE_QTVER)
	find_package(QT NAMES Qt6 Qt5 COMPONENTS Core REQUIRED)
	find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)
	set(QT_VER ${QT_VERSION_MAJOR})
else()
	find_package(Qt${ALL_QTVER} COMPONENTS Core Gui Widgets REQUIRED)
	set(QT_VER ${ALL_QTVER})
endif()

add_subdirectory("allegiance")