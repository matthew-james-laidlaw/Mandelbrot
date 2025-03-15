#pragma once

#include <Tensor.hpp>
#include <lodepng.h>

#include "Expect.hpp"

#include <fstream>

auto EncodePng(const std::string& filename, Tensor<uint8_t, 3> const& rgb) -> void
{
    auto [height, width, channels] = rgb.Shape();
    Expect(channels == 3, "error: input tensor must have 3 channels (RGB)");

    auto error = lodepng::encode(filename, rgb.Data(), width, height, LCT_RGB, 8);
    Expect(!error, "error: " + std::string(lodepng_error_text(error)));
}
