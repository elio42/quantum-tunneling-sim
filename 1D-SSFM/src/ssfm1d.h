#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <fftw3.h>

const double hbar = 1;
const double m = 1;

class SSFM1D {
    private:
        std::vector<double> grid;
        std::vector<std::complex<double>> waveFunction;
        std::vector<double> enviroment_potential;
        std::vector<double> k;
        std::vector<std::complex<double>> U_p;
        std::vector<std::complex<double>> U_k;
        fftw_plan plan_forward;
        fftw_plan plan_backward;
        fftw_complex *psi_in;
        double dt;
        double dk;
        double dx;
        double N;
        double L;
        size_t padding;
        double dt_saftey_factor;
        int number_of_waves;
        double boundary_pot;
        double sigma;
        double tunneling_pot;
        bool tunneling_wall_enabled;

        void configure_fftw_plans();
        void set_parameters();
        void initialize_grid();
        void initialize_wavefunction();
        void initialize_enviroment_potential();
        void initialize_potential_operator();
        void initialise_kinetic_operator();
        void normalize_wavefunction();

        void potential_half_step();
        void forward_fft();
        void kinetic_full_step();
        void inverse_fft();
        void potential_half_step_and_normalize();

    public:
        SSFM1D(double N, double L, size_t padding, double dt_saftey_factor, int number_of_waves, double boundary_pot, double sigma, double tunneling_pot, bool tunneling_wall_enabled);
        ~SSFM1D();
        
        double get_dt() const { return dt; }
        void step();
        std::vector<double> calculate_probability_density();
        double calculate_area_under_probability_density();
        void output_wavefunction(const std::string& path_to_file, double current_time);
        void output_enviroment_potential(const std::string& path_to_file);
};