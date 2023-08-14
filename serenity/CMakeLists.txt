project(allegiance_s)

add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
	PRIVATE
		"../shared/resources.qrc"
		"src/entry_main.cpp"
)
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Qt${QT_VER}::Core
		Qt${QT_VER}::Gui
		Qt${QT_VER}::Widgets
)
target_include_directories(${PROJECT_NAME}
	PRIVATE
		"include"
)
set_target_properties(${PROJECT_NAME} 
	PROPERTIES
		CXX_STANDARD 20
		AUTOMOC ON
		AUTORCC ON
)
target_precompile_headers(${PROJECT_NAME}
	PRIVATE
		"include/pch.h"
)