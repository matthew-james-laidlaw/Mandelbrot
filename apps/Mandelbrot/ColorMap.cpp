#include "ColorMap.hpp"

#include "ColorMapData/Magma.hpp"
#include "ColorMapData/Twilight.hpp"
#include "ColorMapData/Viridis.hpp"

auto GetColormap(Colormap colormap) -> Palette
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
        return viridis256;
    }
}
