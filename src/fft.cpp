#include <complex>

#include<iostream>

#include <fstream>
#include<string.h>

#include <tuple>
#include <cmath>
#include "fft.hpp"



using namespace std;

void FFT::write_vector_to_file(const vector<complex<double>>& vec, const string& filename) 
{
    ofstream file(filename);
    cout << "Writing vector to file: " << filename << endl;
    file << "Real,Imaginary\n"; // Write header
    for (const auto& c : vec) {
        file << c.real() << "," << c.imag() << "\n"; // Write real
    }
    file.close();
}




vector<complex<double>> FFT::x2(int n) {
    vector<complex<double>> result(n);
    for (int i = 0; i < n; ++i) {
        result[i] = complex<double>(i *i , 0.0); // Example values
    }
    return result;
}


vector<complex<double>> FFT::e(int n, int frequency) {
    vector<complex<double>> result(n);


    for (int i = 0; i < n; ++i) {
        double angle = 2 * M_PI * frequency * i / n;

        double real_part = cos(angle);
        double imag_part = sin(angle);


        result[i] = complex<double>(real_part, imag_part);
        
    }
    return result;
}



vector<complex<double>> FFT::gauss(int n) {
    vector<complex<double>> result(n);


    for (int i = 0; i < n; ++i) {
        int ishift =0;

        if(i< n/2) {
            ishift = i+n/2;
        }else {
            ishift = i-n/2;
        }

        double width = 30;
        double height = 1.0;
        double center = n / 2.0;


        double real_part = height * exp(-pow(ishift - center, 2) / (2 * pow(width, 2)));
        result[i] = complex<double>(real_part, 0.0); // Example values
    }
    return result;
}

bool FFT::is_power_of_two(size_t n) {
    //unchecked
    return (n & (n - 1)) == 0 && n > 0;
}

tuple<vector<complex<double>>, vector<complex<double>>> FFT::split_even_odd(const vector<complex<double>>& input) {
    vector<complex<double>> even;
    vector<complex<double>> odd;
    



    bool even_index = true;
    for(auto el:input) {
        if (even_index) {
            even.push_back(el);
        } else {
            odd.push_back(el);
        }
        even_index = !even_index;
    }


    
    return make_tuple(even, odd);
}


complex<double> FFT::twiddle_factor(int k, int n) {
    double angle = -2.0 * M_PI * k / n;
    return complex<double>(cos(angle), sin(angle));
}

vector<complex<double>> FFT::fft(const vector<complex<double>>& input) {
    
    const size_t n = input.size();

    if (n== 1){
        return input;
    }
    if (!is_power_of_two(n)) {
        throw invalid_argument("Input size must be a power of 2");
    }

    vector<complex<double>> out(n);


    auto [even, odd] = split_even_odd(input);

    even = fft(even);
    odd = fft(odd);

    for(size_t i = 0; i < n/2; ++i) {
        out[i] = even[i] + odd[i] * twiddle_factor(i, n);
        out[i + n/2] = even[i] - odd[i] * twiddle_factor(i, n);
    }
    return out; // Placeholder return value
}

void FFT::print_vector(const vector<complex<double>>& vec) {
    for (const auto& c : vec) {
        cout << c.real() << " + " << c.imag() << "i" << endl;
    }
}









