#include "argparse/argparse.hpp"

#include "Mandelbrot.hpp"

#include <PNG.hpp>
#include <PPM.hpp>
#include <Tensor.hpp>

int main(int argc, char* argv[])
{
    argparse::ArgumentParser program("mandelbrot_generator_colormap");

    // Positional argument for output file
    program.add_argument("output")
        .help("Path for the .ppm file to be saved");

    // Optional argument for colormap
    program.add_argument("--colormap")
        .default_value(std::string("plasma"))
        .help("Which colormap to use: plasma, inferno, or magma");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (const std::runtime_error& err)
    {
        std::cerr << err.what() << "\n";
        std::cerr << program;
        return 1;
    }

    // Get user choices from CLI
    std::string outputPath  = program.get<std::string>("output");
    std::string colormapStr = program.get<std::string>("--colormap");

    // Map string -> enum
    Colormap colormapChoice;
    if (colormapStr == "magma")
    {
        colormapChoice = Colormap::Magma;
    }
    else if (colormapStr == "twilight")
    {
        colormapChoice = Colormap::Twilight;
    }
    else if (colormapStr == "viridis")
    {
        colormapChoice = Colormap::Viridis;
    }
    else
    {
        std::cerr << "Unknown colormap \"" << colormapStr << "\", defaulting to viridis.\n";
        colormapChoice = Colormap::Viridis;
    }

    // Render parameters
    const size_t width  = 3840; // 4K resolution
    const size_t height = 2160;

    // Generate the fractal
    auto rgb = Mandelbrot(height, width, colormapChoice);

    EncodePng(outputPath, rgb);

    return 0;
}
