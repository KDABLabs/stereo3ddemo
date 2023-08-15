cmake_minimum_required(VERSION 3.20)

project(Allegiance VERSION 0.0.1)

option(USE_QTVER "Use Qt version" OFF)

set(DEF_QTVER "6" CACHE INTERNAL "Default Qt version")
set(QTVER_LIST "5;6" CACHE INTERNAL "List of possible Qt versions")
set(ALL_QTVER ${DEF_QTVER} CACHE STRING "Used Qt version")
set_property(CACHE ALL_QTVER PROPERTY STRINGS ${QTVER_LIST})

find_package(Qt5 COMPONENTS Core Gui Widgets 3DCore 3DExtras 3DRender 3DInput)
if(Qt5_FOUND)
	set(QT_VER "5")
	add_subdirectory("allegiance" ${CMAKE_BINARY_DIR}/allegiance5)
endif()

find_package(Qt6 COMPONENTS Core Gui Widgets 3DCore 3DExtras 3DRender 3DInput)
if(Qt6_FOUND)
	set(QT_VER "6")
	add_subdirectory("allegiance" ${CMAKE_BINARY_DIR}/allegiance6)
endif()


if(NOT USE_QTVER)
	find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets)
	find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)
	set(QT_VER ${QT_VERSION_MAJOR})
	# At this point, QT_VER is set to the Qt version that was first found in path.
else()
	find_package(Qt${ALL_QTVER} COMPONENTS Core Gui Widgets REQUIRED)
	set(QT_VER ${ALL_QTVER})
endif()
add_subdirectory("serenity")
