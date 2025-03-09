#pragma once

#include "ColorMapData/Palette.hpp"

#include <string>

enum class Colormap
{
    Magma,
    Twilight,
    Viridis,
};

auto GetColormapByName(std::string const& name) -> Colormap;
auto GetColormapPalette(Colormap colormap) -> Palette;
