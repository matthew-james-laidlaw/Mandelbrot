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
            auto [red, green, blue] = GetColormapPalette(colormap)[index];

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

auto MandelbrotSimd(size_t height, size_t width, Colormap colormap) -> Tensor<uint8_t, 3>
{
    // Complex-plane bounds
    const float real_start = -2.5f;
    const float real_stop  = 1.0f;
    const float imag_start = -1.0f;
    const float imag_stop  = 1.0f;

    // Create output tensor (height x width x 3)
    Tensor<uint8_t, 3> mandelbrot({height, width, 3});

    // Dispatch by rows; each task processes one (or more) entire rows.
    DispatchRows(height, [&](size_t y)
    {
        // We'll compute imag just once per row
        float imag    = imag_start + (static_cast<float>(y) / (height - 1)) * (imag_stop - imag_start);
        __m128 c_imag = _mm_set1_ps(imag);

        size_t x = 0;
        // Process pixels in blocks of 4
        for (; x + 3 < width; x += 4)
        {
            // Gather the real coordinates for these 4 pixels
            float x_coords[4];
            for (int i = 0; i < 4; ++i)
            {
                size_t curX = x + i;
                x_coords[i] = real_start + (static_cast<float>(curX) / (width - 1)) * (real_stop - real_start);
            }
            __m128 c_real = _mm_loadu_ps(x_coords);

            // Initialize z = (0, 0) for each of the 4 pixels
            __m128 a         = _mm_setzero_ps(); // real part
            __m128 b         = _mm_setzero_ps(); // imag part
            __m128 iterCount = _mm_setzero_ps();

            const float bailout_sq_val = k_bailout_radius * k_bailout_radius;
            __m128 bailout_sq          = _mm_set1_ps(bailout_sq_val);
            __m128 one                 = _mm_set1_ps(1.0f);

            // Iterate
            for (int iter = 0; iter < k_max_iterations; ++iter)
            {
                // a2 = a*a, b2 = b*b
                __m128 a2 = _mm_mul_ps(a, a);
                __m128 b2 = _mm_mul_ps(b, b);

                // magnitude^2 = a2 + b2
                __m128 mag2 = _mm_add_ps(a2, b2);

                // mask = lanes still under bailout
                __m128 mask      = _mm_cmplt_ps(mag2, bailout_sq);
                int active_lanes = _mm_movemask_ps(mask);
                if (active_lanes == 0)
                {
                    break; // all lanes escaped
                }

                // increment iteration only for active lanes
                __m128 inc = _mm_and_ps(mask, one);
                iterCount  = _mm_add_ps(iterCount, inc);

                // compute new (a, b)
                // z -> z^2 + c
                // z^2 = (a + i b)^2 = (a^2 - b^2) + i(2ab)
                __m128 twoab = _mm_mul_ps(_mm_mul_ps(a, b), _mm_set1_ps(2.0f));
                __m128 a_new = _mm_add_ps(_mm_sub_ps(a2, b2), c_real);
                __m128 b_new = _mm_add_ps(twoab, c_imag);

                // Update only the active lanes
#if defined(__SSE4_1__) // SSE4.1 approach with blendv_ps
                a = _mm_blendv_ps(a, a_new, mask);
                b = _mm_blendv_ps(b, b_new, mask);
#else
                // SSE2 fallback approach
                a = _mm_or_ps(_mm_and_ps(mask, a_new), _mm_andnot_ps(mask, a));
                b = _mm_or_ps(_mm_and_ps(mask, b_new), _mm_andnot_ps(mask, b));
#endif
            }

            // Store final values back into arrays so we can do smoothing
            float a_vals[4], b_vals[4], iter_vals[4];
            _mm_storeu_ps(a_vals, a);
            _mm_storeu_ps(b_vals, b);
            _mm_storeu_ps(iter_vals, iterCount);

            // Assign colors
            for (int i = 0; i < 4; ++i)
            {
                size_t curX = x + i;
                // If iteration < max, the pixel escaped
                if (iter_vals[i] < k_max_iterations)
                {
                    float mag = std::sqrt(a_vals[i] * a_vals[i] + b_vals[i] * b_vals[i]);
                    // smoothing factor
                    float nu = 0.0f;
                    if (mag > 0.0f) // protect log
                    {
                        nu = std::log(std::log(mag)) / std::log(2.0f);
                    }
                    float normalized = (iter_vals[i] + 1.0f - nu) / k_max_iterations;
                    // clamp to [0, 255]
                    size_t index = std::clamp(static_cast<size_t>(normalized * 255.0f), size_t(0), size_t(255));

                    // get RGB from palette
                    auto [red, green, blue] = GetColormapPalette(colormap)[index];
                    mandelbrot(y, curX, 0)  = red;
                    mandelbrot(y, curX, 1)  = green;
                    mandelbrot(y, curX, 2)  = blue;
                }
                else
                {
                    // never escaped => black
                    mandelbrot(y, curX, 0) = 0;
                    mandelbrot(y, curX, 1) = 0;
                    mandelbrot(y, curX, 2) = 0;
                }
            }
        }

        // Tail loop for any leftover pixels if width not multiple of 4
        for (; x < width; ++x)
        {
            float real = real_start + (static_cast<float>(x) / (width - 1)) * (real_stop - real_start);
            std::complex<float> c(real, imag);
            std::complex<float> z(0.0f, 0.0f);

            size_t iteration = 0;
            while (std::abs(z) < k_bailout_radius && iteration < k_max_iterations)
            {
                z = z * z + c;
                ++iteration;
            }

            if (iteration < k_max_iterations)
            {
                float nu  = 0.0f;
                float mag = std::abs(z);
                if (mag > 0.0f)
                {
                    nu = std::log(std::log(mag)) / std::log(2.0f);
                }
                float normalized        = (iteration + 1.0f - nu) / k_max_iterations;
                size_t index            = std::clamp(static_cast<size_t>(normalized * 255.0f), size_t(0), size_t(255));
                auto [red, green, blue] = GetColormapPalette(colormap)[index];
                mandelbrot(y, x, 0)     = red;
                mandelbrot(y, x, 1)     = green;
                mandelbrot(y, x, 2)     = blue;
            }
            else
            {
                mandelbrot(y, x, 0) = 0;
                mandelbrot(y, x, 1) = 0;
                mandelbrot(y, x, 2) = 0;
            }
        }
    });

    return mandelbrot;
}
