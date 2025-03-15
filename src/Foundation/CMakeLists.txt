add_library(foundation INTERFACE)
target_include_directories(foundation INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(foundation INTERFACE lodepng tensor)
