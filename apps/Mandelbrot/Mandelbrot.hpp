#pragma once

#include <Dispatch.hpp>
#include <Tensor.hpp>
#include "ColorMap.hpp"
#include <complex>
#include <vector>
#include <cmath>
#include <algorithm>

// Compute the Mandelbrot set with continuous (smooth) coloring and build a histogram.
auto MandelbrotSmoothHistogram(size_t height, size_t width) -> Tensor<float, 2>
{
    const size_t max_iterations = 1000;
    const float real_start = -2.5f;
    const float real_stop  = 1.0f;
    const float imag_start = -1.0f;
    const float imag_stop  = 1.0f;

    Tensor<float, 2> smooth({height, width});

    // First pass: Compute smooth iteration counts.
    Dispatch(height, width, [&](size_t y, size_t x)
    {
        // Map pixel coordinate to Mandelbrot plane.
        float real = real_start + (static_cast<float>(x) / (width - 1)) * (real_stop - real_start);
        float imag = imag_start + (static_cast<float>(y) / (height - 1)) * (imag_stop - imag_start);
        std::complex<float> c(real, imag);
        std::complex<float> z(0.0f, 0.0f);

        size_t iteration = 0;
        while (std::abs(z) < 2.0f && iteration < max_iterations)
        {
            z = z * z + c;
            ++iteration;
        }

        if (iteration < max_iterations)
        {
            // Compute smooth (continuous) iteration count.
            // Note: std::abs(z) is guaranteed to be >=2 here.
            float nu = std::log(std::log(std::abs(z))) / std::log(2.0f);
            smooth(y, x) = iteration + 1 - nu;
        }
        else
        {
            smooth(y, x) = static_cast<float>(iteration);
        }
    });

    // Build histogram for escaped points (those with iteration < max_iterations).
    std::vector<size_t> histogram(max_iterations, 0);
    size_t total = 0;
    for (size_t y = 0; y < height; ++y)
    {
        for (size_t x = 0; x < width; ++x)
        {
            float value = smooth(y, x);
            if (value < max_iterations)  // pixel escaped
            {
                // Use the integer part (floor) of the smooth value for histogram binning.
                size_t bin = std::min(max_iterations - 1, static_cast<size_t>(value));
                histogram[bin]++;
                total++;
            }
        }
    }

    // Compute the cumulative histogram (CDF).
    std::vector<double> cumulative(histogram.size(), 0.0);
    double sum = 0.0;
    for (size_t i = 0; i < histogram.size(); ++i)
    {
        sum += histogram[i];
        cumulative[i] = sum;
    }

    // Second pass: Remap each smooth value to a normalized value using the histogram.
    Dispatch(height, width, [&](size_t y, size_t x)
    {
        float value = smooth(y, x);
        if (value < max_iterations)
        {
            size_t bin = std::min(max_iterations - 1, static_cast<size_t>(value));
            // Calculate normalized hue based on the cumulative distribution.
            double hue = cumulative[bin];
            // Add fractional contribution from the current bin.
            hue += (value - bin) * histogram[bin];
            hue /= total;  // Normalize to [0,1]
            smooth(y, x) = static_cast<float>(hue);
        }
        else
        {
            // Points inside the Mandelbrot set.
            smooth(y, x) = 0.0f;
        }
    });

    return smooth;
}

// Generate the Mandelbrot image using the remapped values from histogram coloring.
auto GenerateMandelbrotImage(size_t height, size_t width, Colormap colormap) -> Tensor<uint8_t, 3>
{
    // Get the normalized, histogram-equalized values.
    auto intensity = MandelbrotSmoothHistogram(height, width);
    Tensor<uint8_t, 3> rgb({height, width, 3});

    Dispatch(height, width, [&](size_t y, size_t x)
    {
        // Color color = getColorFromColormap(colormap, intensity(y, x));
        Color color = magma32[static_cast<size_t>(std::clamp(intensity(y, x) * 255.0, 0.0, 255.0))];
        rgb(y, x, 0) = static_cast<unsigned char>(std::clamp(color.r * 255.0, 0.0, 255.0));
        rgb(y, x, 1) = static_cast<unsigned char>(std::clamp(color.g * 255.0, 0.0, 255.0));
        rgb(y, x, 2) = static_cast<unsigned char>(std::clamp(color.b * 255.0, 0.0, 255.0));
    });

    return rgb;
}
