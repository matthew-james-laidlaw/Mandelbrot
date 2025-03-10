# Mandelbrot
[![unit](https://github.com/matthew-james-laidlaw/Matrix/actions/workflows/unit.yml/badge.svg?branch=main)](https://github.com/matthew-james-laidlaw/Matrix/actions/workflows/unit.yml)

A mandelbrot image generating application and a (somewhat) generic 1D, 2D, and 3D tensor library.

## What it is
* Mandelbrot implemented as a generic or SIMD kernel dispatched across a tensor
* A usable application that has SSE support for x64 hosts
* Thread dispatched across std::thread::hardware_concurrency() number of threads (all available cores!) for speed
## What it is not (yet)
* Benchmarked between x64/arm64, generic/SSE/NEON, different threading amounts
* Auto tune framework
* Mandelbrot implemented as purely matrix arithmetic
* Expression templates for lazy evaluation
* I really want to compare mandelbrot using generic/SIMD, per-pixel, threaded kernels against Mandelbrot that is generated via matrix arithmetic (i.e. linspace and all that) with lazily evalutaed expression templates, and threaded/SIMD matrix arithmetic primitives
* NEON implementation
* A readily consumable tensor library -- feel free to pull this in as a submodule and try, but it's just not packaged that way yet
* GPU

Mostly self-implemented. There are libraries pulled in for argument parsing, unit testing, and image encoding e.g. lodepng).

# Design Decisions
* The thread dispatching was inspired by the SYCL framework (defining a per-element lambda, and letting the framework pass that to a thread pool, theoretically should work for threading/SIMD/GPU if that support is added)
* The focus of this library is on image processing, thus the tensor class is not fully generic for any order. Really, this library supports 1D, 2D, and 3D tensors where a 3D tensor can represent a planar image (channels x height x width) or an interleaved image (height x width x channels).
* The class is generic where possible, and when it is not, template specializations define how operations are defined for that order. One example is indexing. Generically finding a linear index into a 1D, 2D, or 3D tensor would be more computationally intensive than just applying the arithmetic needed to find a linear index for the given order. Thus, a template specialization exists for each case.

# Resources
* [Mandelbrot Plotting Algorithms](https://en.wikipedia.org/wiki/Plotting_algorithms_for_the_Mandelbrot_set#Continuous_(smooth)_coloring)
* [CMake Project Structure Convention](https://cliutils.gitlab.io/modern-cmake/chapters/basics/structure.html)
