cmake_minimum_required(VERSION 3.29.3)

project(MATRIX CXX)

include(GoogleTest)
enable_testing()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(${PROJECT_NAME}_ENABLE_BENCHMARKS OFF)

add_subdirectory(extern)
add_subdirectory(src)
add_subdirectory(tests)
add_subdirectory(apps)
