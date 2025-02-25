cmake_minimum_required(VERSION 3.29)
project(falcon)

set(CMAKE_CXX_STANDARD 20)

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
    set(MACOSX TRUE)
endif()

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

if (NOT WIN32 AND NOT MACOSX)
#    find_package(libuuid REQUIRED)
    target_link_libraries(falcon PUBLIC uuid)
elseif (MACOSX)
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(UUID REQUIRED uuid)
    target_link_libraries(falcon PUBLIC ${UUID_LIBRARIES})
    target_include_directories(falcon PUBLIC ${UUID_INCLUDE_DIRS})
endif ()

add_subdirectory(externals)
add_subdirectory(samples)
add_subdirectory(tests)
