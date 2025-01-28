cmake_minimum_required(VERSION 3.30)
project(falcon)

set(CMAKE_CXX_STANDARD 20)

add_library(falcon STATIC src/library.cpp)
target_include_directories(falcon PUBLIC inc)

add_subdirectory(externals)
add_subdirectory(samples)
add_subdirectory(tests)