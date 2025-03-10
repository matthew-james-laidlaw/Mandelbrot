# Mandelbrot

[![unit](https://github.com/matthew-james-laidlaw/Matrix/actions/workflows/unit.yml/badge.svg?branch=main)](https://github.com/matthew-james-laidlaw/Matrix/actions/workflows/unit.yml)

A mandelbrot visualizing application built on top of a (somewhat) generic 1D, 2D, and 3D tensor library. See [user-guide.md](docs/user-guid.md) for detailed build & usage instructions.

*Mostly self-implemented with external libraries pulled in for argument parsing, unit testing, and image encoding (e.g., [lodepng](https://github.com/lvandeve/lodepng)).*

![mandelbrot](docs/example.png "Mandelbrot")

## What It Is

- **Command Line Tool:**  
  The command line tool allows users to specify an output image path, and one of a few colormaps to print out a 4k Mandelbrot visualization (as a .png). The Mandelbrot calculation is implemented as a per-pixel kernel dispatched over a matrix of pixels. There are generic and SSE implementations available, with NEON support in the works. The tool automatically selects SSE/NEON if available but falls back to a slower, generic implementation if not. Work is divided across all available cores via `std::thread::hardware_concurrency()`, where each core takes care of some number of rows.

- **Tensor Library:**  
  The underlying library provides a tensor class supporting 1D, 2D, and 3D collections of elements. The class is tailored for image processing use cases where a 2D tensor could represent a grayscale image and a 3D tensor could represent either a planar image (channels x height x width) or an interleaved image (height x width x channels). However, general matrix arithmetic is provided for 2D tensors (matrices) and could work as the foundation of a simple linear algebra library.

- **Personal Project:**  
  This project is a passion project designed to explore the domains of image processing, generic programming, performance engineering, and C++ project management.

## Future Work

- Implement NEON Mandelbrot calculation.
- Package tensor library for consumption as a library (feel free to pull it in as a submodule and try, but it's not formally packaged yet).
- Benchmark between x64/arm64, generic/SSE/NEON, and different threading configurations.
- Implement [expression templates](https://en.wikipedia.org/wiki/Expression_templates) for lazy evaluation of matrix arithmetic, and to avoid storage of intermediate results.
- Implement Mandelbrot calculation with matrix arithmetic instead of a per-element kernel and compare performance.

# Resources
* [Mandelbrot Plotting Algorithms](https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring)
* [CMake Project Structure Convention](https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html)
