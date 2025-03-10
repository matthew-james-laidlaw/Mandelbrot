#pragma once

#include "ColorMapData/Magma.hpp"
#include "ColorMapData/Twilight.hpp"
#include "ColorMapData/Viridis.hpp"

#include <iostream>
#include <string>

enum class Colormap
{
    Magma,
    Twilight,
    Viridis,
};

auto GetColormapByName(std::string const& name) -> Colormap
{
    if (name == "magma")
    {
        return Colormap::Magma;
    }
    else if (name == "twilight")
    {
        return Colormap::Twilight;
    }
    else if (name == "viridis")
    {
        return Colormap::Viridis;
    }
    else
    {
        std::cerr << "no colormap (or invalid colormap) requested, defaulting to magma" << std::endl;
        return Colormap::Magma;
    }
}

auto GetColormapPalette(Colormap colormap) -> Palette
{
    switch (colormap)
    {
    case Colormap::Magma:
        return magma256;
    case Colormap::Twilight:
        return twilight256;
    case Colormap::Viridis:
        return viridis256;
    default:
        return magma256;
    }
}
