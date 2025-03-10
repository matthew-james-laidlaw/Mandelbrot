#pragma once


#include <Dispatch.hpp>
#include <Tensor.hpp>

#include "InstructionSet.hpp"
#include "ColorMap.hpp"

static constexpr size_t k_max_iterations = 100;

// bounds of the complex plane to visualize
static constexpr float k_real_start = -2.5f;
static constexpr float k_real_stop  = 1.0f;
static constexpr float k_imag_start = -1.0f;
static constexpr float k_imag_stop  = 1.0f;

static constexpr float k_bailout_radius         = 256.0f;
static constexpr float k_bailout_radius_squared = k_bailout_radius * k_bailout_radius;

/** @brief Generate a visualization of the Mandelbrot set using the given color palette.
 * @param[in] height The height of the output image.
 * @param[in] width The width of the output image.
 * @param[in] colormap The color palette to use.
 * @returns A 3D tensor (height x width x 3) representing an interleaved RGB image.
 */
auto MandelbrotGeneric(size_t height, size_t width, Colormap colormap) -> Tensor<uint8_t, 3>
{
    auto mandelbrot = Tensor<uint8_t, 3>({height, width, 3});

    // apply the following operation to every pixel in the image
    DispatchElement(height, width, [&](size_t y, size_t x)
    {
        // map the pixel coordinate to a point in the complex plane
        float real = k_real_start + (static_cast<float>(x) / (width - 1)) * (k_real_stop - k_real_start);
        float imag = k_imag_start + (static_cast<float>(y) / (height - 1)) * (k_imag_stop - k_imag_start);

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
            size_t index = std::clamp(static_cast<size_t>(normalized * 255.0f), size_t(0), size_t(255));

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

auto MandelbrotSSE(size_t height, size_t width, Colormap colormap) -> Tensor<uint8_t, 3>
{
#if __SUPPORTS_SSE__
    auto mandelbrot = Tensor<uint8_t, 3>({height, width, 3});

    // apply the following operation to every row in the image
    DispatchRow(height, [&](size_t y)
    {
        // compute imaginary component for current row
        float imag = k_imag_start + (static_cast<float>(y) / (height - 1)) * (k_imag_stop - k_imag_start);

        // process pixels in chunks of four
        for (size_t x_start = 0; x_start < width; x_start += 4)
        {
            // compute real component for current set of four pixels
            // float real[i] = k_real_start + (static_cast<float>(x[i]) / (width - 1)) * (k_real_stop - k_real_start);
            __m128 v_real_start      = _mm_set1_ps(k_real_start);
            __m128 v_real_stop       = _mm_set1_ps(k_real_stop);
            __m128 v_real_range      = _mm_sub_ps(v_real_stop, v_real_start);
            __m128 v_width_minus_one = _mm_set1_ps(static_cast<float>(width - 1));
            __m128 v_indices         = _mm_setr_ps(static_cast<float>(x_start + 0),
                                                   static_cast<float>(x_start + 1),
                                                   static_cast<float>(x_start + 2),
                                                   static_cast<float>(x_start + 3));
            __m128 v_ratio           = _mm_div_ps(v_indices, v_width_minus_one);
            __m128 v_c_real          = _mm_add_ps(v_real_start, _mm_mul_ps(v_ratio, v_real_range));

            // set imaginary component of c to constant value for current row
            __m128 v_c_imag = _mm_set1_ps(imag);

            // initialize z to (0+0i) for each pixel
            __m128 v_z_real = _mm_setzero_ps();
            __m128 v_z_imag = _mm_setzero_ps();

            // initialize iteration counts to 0 and set maximum iterations
            __m128i vi_iterations = _mm_setzero_si128();
            __m128i vi_max_iters  = _mm_set1_epi32(k_max_iterations);

            // set bailout squared threshold
            __m128 v_bailout_sq = _mm_set1_ps(k_bailout_radius_squared);

            // set constant 2 for later use in Mandelbrot calculations
            __m128 v_two = _mm_set1_ps(2.0f);

            /*

                iterate Mandelbrot formula until all lanes have either diverged or reached the maximum iteration count
                emulates the following sequential code:

                while (std::abs(z) < k_bailout_radius && iteration < k_max_iterations)
                {
                    z = z * z + c;
                    ++iteration;
                }

            */

            while (true)
            {
                __m128 v_z_real_sq   = _mm_mul_ps(v_z_real, v_z_real);
                __m128 v_z_imag_sq   = _mm_mul_ps(v_z_imag, v_z_imag);
                __m128 v_z_magnitude = _mm_add_ps(v_z_real_sq, v_z_imag_sq);

                // determine lanes where magnitude is less than bailout
                __m128 v_within_bailout = _mm_cmplt_ps(v_z_magnitude, v_bailout_sq);

                // determine lanes where iteration count is below maximum
                __m128i vi_iter_lt_max = _mm_cmplt_epi32(vi_iterations, vi_max_iters);
                __m128 v_iter_lt_max   = _mm_castsi128_ps(vi_iter_lt_max);

                // combine masks to find lanes that are still active
                __m128 v_active = _mm_and_ps(v_within_bailout, v_iter_lt_max);

                // create bitmask from active lanes; if no lane is active, exit loop
                int active_mask = _mm_movemask_ps(v_active);
                if (!active_mask)
                {
                    break;
                }

                // increment iteration counts for lanes within bailout
                __m128i vi_within_bailout_int = _mm_castps_si128(v_within_bailout);
                __m128i vi_one                = _mm_set1_epi32(1);
                __m128i vi_increment          = _mm_and_si128(vi_within_bailout_int, vi_one);
                vi_iterations                 = _mm_add_epi32(vi_iterations, vi_increment);

                // compute new z values using Mandelbrot formula: z = z^2 + c
                __m128 v_new_z_real = _mm_add_ps(_mm_sub_ps(v_z_real_sq, v_z_imag_sq), v_c_real);
                __m128 v_new_z_imag = _mm_add_ps(_mm_mul_ps(v_two, _mm_mul_ps(v_z_real, v_z_imag)), v_c_imag);

                // update z values only for lanes that are still within bailout
                v_z_real = _mm_or_ps(_mm_and_ps(v_within_bailout, v_new_z_real), _mm_andnot_ps(v_within_bailout, v_z_real));
                v_z_imag = _mm_or_ps(_mm_and_ps(v_within_bailout, v_new_z_imag), _mm_andnot_ps(v_within_bailout, v_z_imag));
            }

            // store computed iteration counts and z values from SIMD registers
            int iter_counts[4];
            _mm_storeu_si128(reinterpret_cast<__m128i*>(iter_counts), vi_iterations);

            float z_real[4];
            float z_imag[4];
            _mm_storeu_ps(z_real, v_z_real);
            _mm_storeu_ps(z_imag, v_z_imag);

            // assign colors to pixels based on iteration counts
            for (size_t x = x_start; x < x_start + 4; ++x)
            {
                size_t iteration = iter_counts[x - x_start];
                std::complex<float> z(z_real[x - x_start], z_imag[x - x_start]);

                if (iteration < k_max_iterations) // points that escape are colored based on the number of iterations it took to escape
                {
                    // apply smoothing to reduce banding
                    float nu         = std::log(std::log(std::abs(z))) / std::log(2.0f);
                    float normalized = (iteration + 1 - nu) / k_max_iterations;

                    // map normalized value to colormap index (range 0 - 255)
                    size_t index = std::clamp(static_cast<size_t>(normalized * 255.0f), size_t(0), size_t(255));

                    // retrieve rgb color from colormap palette
                    auto [red, green, blue] = GetColormapPalette(colormap)[index];

                    mandelbrot(y, x, 0) = red;
                    mandelbrot(y, x, 1) = green;
                    mandelbrot(y, x, 2) = blue;
                }
                else // color non-escaped points as black
                {
                    mandelbrot(y, x, 0) = 0;
                    mandelbrot(y, x, 1) = 0;
                    mandelbrot(y, x, 2) = 0;
                }
            }
        }

        // process any trailing pixels with scalar code
        for (size_t x = (width - width % 4); x < width; ++x)
        {
            // compute real component for current pixel
            float ratio = static_cast<float>(x) / (width - 1);
            float real  = k_real_start + ratio * (k_real_stop - k_real_start);
            std::complex<float> c(real, imag);
            std::complex<float> z(0, 0);
            size_t iteration = 0;

            // iterate Mandelbrot formula
            while (std::abs(z) < k_bailout_radius && iteration < k_max_iterations)
            {
                z = z * z + c;
                ++iteration;
            }

            if (iteration < k_max_iterations) // point escaped; compute color based on iteration count
            {
                float nu                = std::log(std::log(std::abs(z))) / std::log(2.0f);
                float normalized        = (iteration + 1 - nu) / k_max_iterations;
                size_t index            = std::clamp(static_cast<size_t>(normalized * 255.0f), size_t(0), size_t(255));
                auto [red, green, blue] = GetColormapPalette(colormap)[index];

                mandelbrot(y, x, 0) = red;
                mandelbrot(y, x, 1) = green;
                mandelbrot(y, x, 2) = blue;
            }
            else // point did not escape; color black
            {
                mandelbrot(y, x, 0) = 0;
                mandelbrot(y, x, 1) = 0;
                mandelbrot(y, x, 2) = 0;
            }
        }
    });

    return mandelbrot;
#else
    throw std::runtime_error("this binary was not compiled with SSE support");
#endif
}

auto MandelbrotNEON(size_t height, size_t width, Colormap colormap) -> Tensor<uint8_t, 3>
{
#if __SUPPORTS_NEON__
    std::cout << "WARNING: NEON not yet implemented, falling back to generic version" << std::endl;
    return MandelbrotGeneric(height, width, colormap);
#else
    throw std::runtime_error("this binary was not compiled with NEON support");
#endif
}

auto Mandelbrot(size_t height, size_t width, Colormap colormap) -> Tensor<uint8_t, 3>
{
    if (SupportsSSE())
    {
        std::cout << "Running Mandelbrot with SSE instruction set." << std::endl;
        return MandelbrotSSE(height, width, colormap);
    }
    else if (SupportsNEON())
    {
        std::cout << "Running Mandelbrot with NEON instruction set." << std::endl;
        return MandelbrotNEON(height, width, colormap);
    }
    else
    {
        std::cout << "Running Mandelbrot with generic instruction set." << std::endl;
        return MandelbrotGeneric(height, width, colormap);
    }
}
