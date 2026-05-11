#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <complex>
#include <fftw3.h>

const double boudary_potential = 1e6;
const double hbar = 1;
const double m = 1;

class SSFM2D {
    private:
        // Grid coordinates
        std::vector<double> grid_x;
        std::vector<double> grid_y;
        
        // Wavefunction and potentials (flat 2D arrays)
        std::vector<std::complex<double>> waveFunction;
        std::vector<double> enviroment_potential;
        
        // Frequency grids
        std::vector<double> k_x;
        std::vector<double> k_y;
        
        // Operators (flat 2D arrays)
        std::vector<std::complex<double>> U_p;
        std::vector<std::complex<double>> U_k;
        
        // FFTW plans
        fftw_plan plan_forward;
        fftw_plan plan_backward;
        fftw_complex *psi_in;

        double dt;
        double dk_x;
        double dk_y;
        double h;
        double Lx;
        double Ly;
        double Nx;
        double Ny;
        size_t padding;
        double dt_saftey_factor;
        int number_of_waves_x;
        int number_of_waves_y;

        inline size_t index(size_t ix, size_t iy) const { return iy * (size_t)Nx + ix; }
        double calculate_norm();

        void configure_fftw_plans();
        void set_parameters();
        void initialize_frequency_grids();
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
         SSFM2D(double Nx = 256, double Ny = 256, double Lx = 1.0, size_t padding = 16, 
             double dt_saftey_factor = 0.2, int number_of_waves_x = 10, int number_of_waves_y = 10);
        ~SSFM2D();
        
        double get_dt() const { return dt; }
        void step();
        std::vector<double> calculate_probability_density();
        void output_wavefunction(const std::string& path_to_file, double current_time);
};