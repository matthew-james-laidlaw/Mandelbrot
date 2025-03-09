#pragma once

#include "ColorMapData/Palette.hpp"

enum class Colormap
{
    Magma,
    Twilight,
    Viridis,
};

auto GetColormap(Colormap colormap) -> Palette;
