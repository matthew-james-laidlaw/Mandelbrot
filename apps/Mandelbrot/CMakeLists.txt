add_executable(mandelbrot main.cpp)
target_link_libraries(mandelbrot argparse foundation dispatch)

add_custom_target(smoke_test
    COMMAND $<TARGET_FILE:mandelbrot> mandelbrot.png
)
