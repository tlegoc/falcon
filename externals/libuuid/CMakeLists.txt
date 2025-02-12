FetchContent_Declare(
        libuuid
        GIT_REPOSITORY  https://github.com/gershnik/libuuid-cmake.git
        GIT_TAG         v2.39.1
        GIT_SHALLOW     TRUE
)

FetchContent_MakeAvailable(libuuid)