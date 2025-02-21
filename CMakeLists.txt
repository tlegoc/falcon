cmake_minimum_required(VERSION 3.29)
project(falcon)

set(CMAKE_CXX_STANDARD 20)

if (WIN32)
    set(FALCON_BACKEND src/falcon_windows.cpp)
else ()
    set(FALCON_BACKEND src/falcon_posix.cpp)
endif (WIN32)

add_library(falcon STATIC inc/falcon.h
        src/falcon_common.cpp
        src/uuid.cpp
        src/falcon_server.cpp
        src/falcon_client.cpp
        src/falcon_stream.cpp
        ${FALCON_BACKEND})
target_include_directories(falcon PUBLIC inc)
target_link_libraries(falcon PUBLIC spdlog::spdlog_header_only fmt::fmt-header-only)

if (NOT WIN32)
    target_link_libraries(falcon PUBLIC uuid::uuid)
endif ()

add_subdirectory(externals)
add_subdirectory(samples)
add_subdirectory(tests)
