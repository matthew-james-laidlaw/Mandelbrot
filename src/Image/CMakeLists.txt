add_library(image INTERFACE)
target_include_directories(image INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(image INTERFACE tensor foundation lodepng)
