#include "ssfm2d.h"
#include <cfenv>
#include <filesystem>
#include "../include/argparse.hpp"
#include "../include/indicators.hpp"

#if defined(__linux__)
#include <fenv.h>
#endif

namespace {
void initialize_output_directory(const std::string& output_dir) {
    namespace fs = std::filesystem;

    fs::path dir_path(output_dir);
    if (!fs::exists(dir_path)) {
        fs::create_directories(dir_path);
        return;
    }

    for (const auto& entry : fs::directory_iterator(dir_path)) {
        if (entry.is_regular_file()) {
            fs::remove(entry.path());
        }
    }
}

std::size_t progress_percent(double current_time, double final_time) {
    if (final_time <= 0.0) {
        return 100;
    }

    double ratio = current_time / final_time;
    if (ratio < 0.0) {
        ratio = 0.0;
    } else if (ratio > 1.0) {
        ratio = 1.0;
    }

    return static_cast<std::size_t>(ratio * 100.0);
}
}

int main(int argc, char *argv[])
{ // int argc, char *argv[]
#if defined(__linux__)
    feenableexcept(FE_INVALID | FE_DIVBYZERO | FE_OVERFLOW);
#endif
    // Get Arguments:
    argparse::ArgumentParser parser("1D-SSFM", "0.0.1");

    parser.add_argument("-o", "--output-dir")
        .help("Directory to save output files relative to calling directory")
        .default_value(std::string("output/"))
        .nargs(1);
    
    parser.add_argument("-t", "--total-time")
        .help("Total simulation time")
        .default_value(0.01)
        .scan<'g', double>()
        .nargs(1);

    parser.add_argument("-N", "--number-of-points")
        .help("Number of grid points")
        .default_value(1024.0)
        .scan<'g', double>()
        .nargs(1);

    parser.add_argument("-L", "--length")
        .help("Length of the grid")
        .default_value(1.0)
        .scan<'g', double>()
        .nargs(1);

    parser.add_argument("-P", "--padding")
        .help("Padding for the grid")
        .default_value(size_t{50})
        .scan<'u', size_t>()
        .nargs(1);

    parser.add_argument("-i", "--output-interval")
        .help("Simulation-time interval between saved output files")
        .default_value(0.00005)
        .scan<'g', double>()
        .nargs(1);

    parser.add_argument("-dt", "--delta-time")
        .help("Time step saftey factor, 1 would be the maximum allowed by the stability condition.")
        .default_value(0.1)
        .scan<'g', double>()
        .nargs(1);

    parser.add_argument("-w", "--number-of-waves")
        .help("Number of waves in the initial wave packet, controls the initial momentum of the wave packet.")
        .default_value(40)
        .scan<'i', int>()
        .nargs(1);
    
    try {
        parser.parse_args(argc, argv);
    } catch (const std::exception &e) {
        std::cerr << "Error parsing arguments: " << e.what() << std::endl;
        return 1;
    }

    // Get argument values
    std::string output_dir = parser.get<std::string>("--output-dir");
    double final_time = parser.get<double>("--total-time");
    double output_interval = parser.get<double>("--output-interval");

    // Run The Simulation.
    std::cout << "Starting simulation" << std::endl;
    SSFM2D ssfm(parser.get<double>("--number-of-points"), parser.get<double>("--length"), parser.get<size_t>("--padding"), parser.get<double>("--delta-time"), parser.get<int>("--number-of-waves"));
    double current_time = 0;
    double next_output_time = 0.0;
    int i = 0;
    initialize_output_directory(output_dir);

    indicators::ProgressBar progress_bar{
        indicators::option::PrefixText{"Progress"},
        indicators::option::ShowPercentage{true},
        indicators::option::BarWidth{40}
    };

    while (current_time < final_time) {
        if (current_time >= next_output_time){
            ssfm.output_wavefunction(output_dir, current_time);
            next_output_time += output_interval;
            progress_bar.set_progress(progress_percent(current_time, final_time));
        }
        ssfm.step();
        current_time += ssfm.get_dt();
        i++;
    }

    progress_bar.set_progress(100);
    std::cout << "Simulation completed with a total of " << i << " steps over " << final_time << " time units." << std::endl;

    return 0;
}
