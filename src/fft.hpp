#include <complex>
#include <vector>
#include <tuple>
#include <complex>
#include <cmath>
#include <string>

class FFT {
    

    public:
        static std::vector<std::complex<double>> fft(const std::vector<std::complex<double>>& input);
        static void write_vector_to_file(const std::vector<std::complex<double>>& vec, const std::string& filename);
        static std::vector<std::complex<double>> x2(int n);
        static std::vector<std::complex<double>> e(int n, int frequency = 1);
        static std::vector<std::complex<double>> gauss(int n);
        static void print_vector(const std::vector<std::complex<double>>& vec);
    
    private:
        static bool is_power_of_two(size_t n);
        static std::tuple<std::vector<std::complex<double>>, std::vector<std::complex<double>>> split_even_odd(const std::vector<std::complex<double>>& input);
        static std::complex<double> twiddle_factor(int k, int n);

};
