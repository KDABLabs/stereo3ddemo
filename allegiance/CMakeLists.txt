project(allegiance${POSTFIX})


if(SERENITY_BUILD)
	CompileShaderSet(SerenityScene multiview-scene)
endif()


add_executable(${PROJECT_NAME})
target_sources(${PROJECT_NAME}
	PRIVATE
		"../shared/resources.qrc"
		"src/entry_main.cpp"
		"include/app.h"
		"include/window.h"
		"include/serenity.h"
		"include/serenity_impl.h"
		"include/sliders.h"
		"include/camera_control.h"
)
target_link_libraries(${PROJECT_NAME}
	PRIVATE
		Qt6::Core
		Qt6::Gui
		Qt6::Widgets
		Qt6::3DExtras
		KDGpu::KDGpu
		Serenity::Core
		Serenity::Gui
		Serenity::Logic
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
target_compile_definitions(${PROJECT_NAME}
	PRIVATE
		ALLEGIANCE_SERENITY=${SERENITY_BUILD}
)

if(WIN32)
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:Serenity::Gui>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:Serenity::Logic>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:KDGpu::KDGpu>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:KDGui>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:Serenity::Core>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:KDFoundation>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:KDUtils>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:spdlog::spdlog>
			${CMAKE_CURRENT_BINARY_DIR})
	add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy_if_different
			$<TARGET_FILE:KDGpu::KDGpuKDGui>
			${CMAKE_CURRENT_BINARY_DIR})
endif()