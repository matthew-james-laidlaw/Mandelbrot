# Mandelbrot

[![unit](https://github.com/matthew-james-laidlaw/Mandelbrot/actions/workflows/unit.yml/badge.svg?branch=main)](https://github.com/matthew-james-laidlaw/Mandelbrot/actions/workflows/unit.yml)

A command line tool for visualizing the Mandelbrot set. Generates .png images. Written in C++20.

![mandelbrot](docs/mandelbrot.png "Mandelbrot")

## Usage

```
# configure
cmake --preset release

# build
cmake --build --preset release

# run (windows)
.\build\release\bin\Release\mandelbrot.exe mandelbrot.png --colormap magma

# print help
.\build\release\bin\Release\mandelbrot.exe --help
```

## About

The `mandelbrot` CLI tool allows users to specify an output filepath, and one of a few colormaps, to save a 4k image of the Mandelbrot set. The Mandelbrot calculation is implemented as a per-pixel kernel that is dispatched over a matrix of pixels using my [tensor library](https://github.com/matthew-james-laidlaw/Tensor). The tool automatically divides work evenly across all available threads. Generic and SSE implementions of the Mandelbrot kernel are provided, with a NEON implementation in the works. The SIMD kernel is selected automatically on supported hardware, but falls back to generic if it is not supported.

This tool was written as an integration test for the previously mentioned tensor library, showcasing how the tensor class can be used as a generic container for N-Dimensional data, and how the dispatch interface can help provide threading boosts with minimal effort for users.

## Future Work

- Implement NEON Mandelbrot calculation.
- Benchmark between generic/SSE/NEON implementations and different threading configurations.

# Resources
* [Mandelbrot Plotting Algorithms](https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring)
* [CMake Project Structure Convention](https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html)
