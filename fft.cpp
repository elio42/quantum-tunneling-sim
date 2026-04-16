#include <complex>
#include <vector>
#include <iostream>
#include <fstream>



void write_vector_to_file(const std::vector<std::complex<double>>& vec, const std::string& filename) 
{
    std::ofstream file(filename);
    file << "Real,Imaginary\n"; // Write header
    for (const auto& c : vec) {
        file << c.real() << "," << c.imag() << "\n"; // Write real
    }
    file.close();
}



std::vector<std::complex<double>> x2(int n) {
    std::vector<std::complex<double>> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = std::complex<double>(i * 2.0, 0.0); // Example values
    }
    return result;
}


int main() {
    std::cout << "Hello, FFT!" << std::endl;
    auto arr = x2(10);

    write_vector_to_file(arr, "output.csv");
    return 0;
}







