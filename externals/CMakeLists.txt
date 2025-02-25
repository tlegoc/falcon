Include(FetchContent)

add_subdirectory(spdlog)
add_subdirectory(fmt)
if(_WIN32)
    add_subdirectory(libuuid)
endif()