#include <iostream>
#include <complex>
#include <vector>
#include <fstream>
#include <string>
#include "fft.hpp"

#include "gnuplot-iostream.h"

int main() {

    Gnuplot gp;


    std::cout << "Hello FFT" << std::endl;
    std::vector<std::complex<double>> arr = FFT::e(1024,10);
    arr = FFT::forward(arr);
    arr = FFT::inverse(arr);

    





    // 2. Unpack into 3D coordinates
    // X = Real, Y = Imaginary, Z = Array Index
    std::vector<std::tuple<double, double>> plot_data;
    for (size_t i = 0; i < arr.size(); ++i) {
        plot_data.push_back(std::make_tuple(
            float(i),
            arr[i].real()
            
        ));
    }

// 3. Format the graph for the Complex Plane
    gp << "set title 'Complex Plane (Argand Diagram)'\n";
    gp << "set xlabel 'index'\n";
    gp << "set ylabel 'Real Part'\n";
    gp << "set grid\n";
    
    // 'linespoints' draws a line AND puts a dot at every data point
    gp << "plot '-' with linespoints title 'Complex Data'\n";
    
    // 4. Send the unpacked data
    gp.send1d(plot_data);

    // Pause so the window stays openF
    std::cout << "Press Enter to exit..." << std::endl;
    std::cin.get();

    return 0;

}