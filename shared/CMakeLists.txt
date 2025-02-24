project(shared)

# Assimp library
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG master
)
set(ASSIMP_WARNINGS_AS_ERRORS OFF)
set(ASSIMP_BUILD_TESTS OFF)
set(ASSIMP_BUILD_IFC_IMPORTER OFF)
set(ASSIMP_BUILD_USE_CCACHE OFF)
set(BUILD_SHARED_LIBS OFF)
FetchContent_MakeAvailable(assimp)

# KDBindings library
find_package(KDBindings QUIET)
if(NOT TARGET KDAB::KDBindings)
    fetchcontent_declare(
        KDBindings
        GIT_REPOSITORY https://github.com/KDAB/KDBindings.git
        GIT_TAG efb54c58c3c2fce280d7089617935ec265fe4e2d # v1.1.0
        USES_TERMINAL_DOWNLOAD YES USES_TERMINAL_UPDATE YES
    )
    fetchcontent_makeavailable(KDBindings)
endif()

add_library(
    ${PROJECT_NAME} STATIC
)

set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

set(VAR_SRCS_PUBLIC)
set(VAR_SRCS_PRIVATE)

if(${WITH_NAVLIB})
    set(TDxWare_SDK_DIR "${CMAKE_SOURCE_DIR}/3rdparty/3DxWare_SDK_v4-0-3_r19962/")
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DWITH_NAVLIB) # -DTRACE_NAVLIB

    find_package(TDxWare_SDK 4.0 REQUIRED COMPONENTS 3DxWare::Navlib PATHS ${TDxWare_SDK_DIR})
    get_target_property(NAVLIB_INCLUDE_DIRS 3DxWare::Navlib INTERFACE_INCLUDE_DIRECTORIES)
    get_target_property(NAVLIB_LINK_LIBRARIES 3DxWare::Navlib INTERFACE_LINK_LIBRARIES)

    target_include_directories(${PROJECT_NAME} PUBLIC ${NAVLIB_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${NAVLIB_LINK_LIBRARIES})

    set(VAR_SRCS_PUBLIC "include/shared/spacemouse_navlib.h" "include/shared/navmodel_navlib.h"
                        "include/shared/navmodel_navlib.h"
    )
    set(VAR_SRCS_PRIVATE "src/spacemouse_navlib.cpp" "src/navmodel_navlib.cpp")
elseif(${WITH_SPNAV})
    target_compile_definitions(${PROJECT_NAME} PUBLIC -DWITH_SPNAV)
    target_link_libraries(${PROJECT_NAME} PUBLIC spnav)
    set(VAR_SRCS_PUBLIC "include/shared/spacemouse_spnav.h")
    set(VAR_SRCS_PRIVATE "src/spacemouse_spnav.cpp")
endif()

target_sources(
    ${PROJECT_NAME}
    PUBLIC ${VAR_SRCS_PUBLIC}
           "include/shared/spacemouse_impl.h"
           "include/shared/spacemouse.h"
           "include/shared/cursor.h"
           "include/shared/stereo_camera.h"
    PRIVATE ${VAR_SRCS_PRIVATE}
           "src/stereo_camera.cpp"
)

target_link_libraries(
    ${PROJECT_NAME}
    PUBLIC glm::glm assimp::assimp KDAB::KDBindings
)
target_include_directories(
    ${PROJECT_NAME}
    PUBLIC "include/"
)
set_target_properties(${PROJECT_NAME} PROPERTIES CXX_STANDARD 20)

# Copy assets to build directory
add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
)

install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/assets
    DESTINATION ${CMAKE_INSTALL_BINDIR}/
    COMPONENT applications
)
