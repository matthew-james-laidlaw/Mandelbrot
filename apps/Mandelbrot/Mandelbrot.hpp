#pragma once

#include <Dispatch.hpp>
#include <Tensor.hpp>

#include "ColorMap.hpp"

static constexpr size_t k_max_iterations = 100;
static constexpr float k_bailout_radius  = 256.0f;

/** @brief Generate a visualization of the Mandelbrot set using the given color palette.
 * @param[in] height The height of the output image.
 * @param[in] width The width of the output image.
 * @param[in] colormap The color palette to use.
 * @returns A 3D tensor (height x width x 3) representing an interleaved RGB image.
 */
auto Mandelbrot(size_t height, size_t width, Colormap colormap) -> Tensor<uint8_t, 3>
{
    // define the bounds of the complex plane to visualize
    float real_start = -2.5f;
    float real_stop  = 1.0f;
    float imag_start = -1.0f;
    float imag_stop  = 1.0f;

    auto mandelbrot = Tensor<uint8_t, 3>({height, width, 3});

    // apply the following operation to every pixel in the image
    Dispatch(height, width, [&](size_t y, size_t x)
    {
        // map the pixel coordinate to a point in the complex plane
        float real = real_start + (static_cast<float>(x) / (width - 1)) * (real_stop - real_start);
        float imag = imag_start + (static_cast<float>(y) / (height - 1)) * (imag_stop - imag_start);

        std::complex<float> c(real, imag);
        std::complex<float> z(0.0f, 0.0f);

        size_t iteration = 0;

        // iterate the mandelbrot function until the point escapes or the maximum number of iterations is reached
        while (std::abs(z) < k_bailout_radius && iteration < k_max_iterations)
        {
            z = z * z + c;
            ++iteration;
        }

        if (iteration < k_max_iterations) // points that escape are colored based on the number of iterations it took to escape
        {
            // apply smoothing to reduce banding
            float nu         = std::log(std::log(std::abs(z))) / std::log(2.0f);
            float normalized = (iteration + 1 - nu) / k_max_iterations;

            // map normalized value in range (0.0 - 1.0) to a colormap index in range (0 - 255)
            size_t index = std::clamp(static_cast<size_t>(normalized * 255.0f), 0ull, 255ull);

            // get the corresponding RGB value from the palette
            auto [red, green, blue] = GetColormap(colormap)[index];

            mandelbrot(y, x, 0) = red;
            mandelbrot(y, x, 1) = green;
            mandelbrot(y, x, 2) = blue;
        }
        else // points that do not escape are colored black
        {
            mandelbrot(y, x, 0) = 0;
            mandelbrot(y, x, 1) = 0;
            mandelbrot(y, x, 2) = 0;
        }
    });

    return mandelbrot;
}
