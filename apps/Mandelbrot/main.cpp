#include "argparse/argparse.hpp"

#include <PNG.hpp>
#include <Tensor.hpp>

#include "Mandelbrot.hpp"
#include "Time.hpp"

#include <sstream>

auto Main(int argc, char** argv) -> int
{
    argparse::ArgumentParser program("mandelbrot");

    program.add_argument("output")
        .help("Path for the .png file to be saved");

    program.add_argument("-c", "--colormap")
        .default_value(std::string("magma"))
        .help("Which color palette to use: magma, twilight, or viridis")
        .nargs(1)
        .metavar("(magma|twilight|viridis)");

    try
    {
        program.parse_args(argc, argv);
    }
    catch (std::exception const& e)
    {
        // rethrow exception with added help text to display to the user
        std::stringstream builder;
        builder << e.what() << std::endl;
        builder << program;
        throw std::runtime_error(builder.str());
    }

    auto output_path   = program.get<std::string>("output");
    auto colormap_name = program.get<std::string>("--colormap");

    auto colormap = GetColormapByName(colormap_name);

    // 4k resolution
    const size_t height = 2160;
    const size_t width  = 3840;

    auto [mandelbrot, mandelbrot_elapsed] = Time([&]()
    {
        return Mandelbrot(height, width, colormap);
    });

    auto encode_elapsed = Time([&]()
    {
        EncodePng(output_path, mandelbrot);
    });

    std::cout << "Mandelbrot Generation: " << mandelbrot_elapsed.count() << "s" << std::endl;
    std::cout << "PNG Encoding:          " << encode_elapsed.count() << "s" << std::endl;

    return 0;
}

auto main(int argc, char** argv) -> int
{
    try
    {
        return Main(argc, argv);
    }
    catch (std::exception const& e)
    {
        std::cerr << e.what() << std::endl;
        return 1;
    }
}
