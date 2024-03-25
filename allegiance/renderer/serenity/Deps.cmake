set(FETCHCONTENT_UPDATES_DISCONNECTED ON)

find_package(Serenity QUIET)
if(NOT Serenity_FOUND)
    find_package(spdlog)
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
    set(SERENITY_QT_QML_PLUGIN OFF)
    set(SERENITY_QT_QUICK_OVERLAY OFF)
    FetchContent_MakeAvailable(Serenity)
endif()

find_package(assimp QUIET)
if(NOT assimp_FOUND)
    FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG master
    )
    set(ASSIMP_WARNINGS_AS_ERRORS OFF)
    set(BUILD_SHARED_LIBS OFF)
    FetchContent_MakeAvailable(assimp)
endif()