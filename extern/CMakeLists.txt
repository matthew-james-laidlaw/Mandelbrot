add_subdirectory(argparse)

add_library(lodepng
    ${CMAKE_CURRENT_SOURCE_DIR}/lodepng/lodepng.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/lodepng/lodepng_util.cpp)
target_include_directories(lodepng PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/lodepng)

add_subdirectory(tensor)
