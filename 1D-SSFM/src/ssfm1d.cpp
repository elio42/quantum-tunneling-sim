#include "ssfm1d.h"

/*
Documentation, //ToDo: Allow for parameters.
*/
SSFM1D::SSFM1D(double N, double L, size_t padding, double dt_saftey_factor, int number_of_waves, double boundary_pot, double sigma, double tunneling_pot, bool tunneling_wall_enabled) : N(N), L(L), padding(padding), dt_saftey_factor(dt_saftey_factor), number_of_waves(number_of_waves), boundary_pot(boundary_pot), sigma(sigma), tunneling_pot(tunneling_pot), tunneling_wall_enabled(tunneling_wall_enabled) {
    waveFunction.resize(N);
    configure_fftw_plans();
    set_parameters();
    initialize_grid();
    initialize_wavefunction();
    initialize_enviroment_potential();
    initialize_potential_operator();
    initialise_kinetic_operator();
}

SSFM1D::~SSFM1D() {
    fftw_destroy_plan(plan_forward);
    fftw_destroy_plan(plan_backward);
}

void SSFM1D::configure_fftw_plans() {
    psi_in = reinterpret_cast<fftw_complex*>(waveFunction.data());
    plan_forward = fftw_plan_dft_1d(N, psi_in, psi_in, FFTW_FORWARD, FFTW_MEASURE);
    plan_backward = fftw_plan_dft_1d(N, psi_in, psi_in, FFTW_BACKWARD, FFTW_MEASURE);
}

/*
Calculate the maximum time step to enssure numerical stability.
The time step should satisfy the stability condition: $\Delta t \ll \min{(\frac{2m (\Delta x)^2}{\pi \hbar}, \frac{\pi \hbar}{V_{max}})}$
*/
void SSFM1D::set_parameters() {
    dk = 2 * M_PI / L;
    dx = L / N;
    
    double dt_V = (boundary_pot > 1e-9) ? (M_PI * hbar) / boundary_pot : std::numeric_limits<double>::max();
    dt = dt_saftey_factor * std::min((2 * m * dx * dx) / (M_PI * hbar), dt_V);
}

/*
Initialise the grid with the "coordinate" values.
*/
void SSFM1D::initialize_grid() {
    grid.resize(N);
    for (size_t i = 0; i < grid.size(); ++i) {
        grid[i] = -L / 2 + i * dx;
    }
}

/*
Initialise the wavefunction with a Gaussian wave packet: $\psi(x,0) = \frac{1}{(2 \pi \sigma^2)^{1/4}} \cdot \exp{(- \frac{(x-x_0)^2}{4 \sigma^2} + ik_ox)}$
Where:
  - $\sigma$: Width of the Gaussian wave packet, make sure it fits well within the grid.
  - $k_0$: Initial wave number, controls the momentum of the wave packet. Should be: $k_0 < \frac{2 \pi}{10 \Delta x}$ to avoid aliasing.
  - $x_0$: Initial position of the wave packet.
*/
void SSFM1D::initialize_wavefunction() {
    double k0 = number_of_waves * (2.0 * M_PI / L);
    if (k0 >= ((2 * M_PI)/(10 * dx)) * 0.5){
        std::cerr << "Warning: k0 is large, this may lead to aliasing:" << std::endl;
    }
    double x0 = 0;
    
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] = (1 / pow(2 * M_PI * sigma * sigma, 0.25)) *
                 std::exp(std::complex<double>(-pow(grid[i] - x0, 2) / (4 * sigma * sigma), k0 * grid[i]));
    }
    normalize_wavefunction();
}

/*
Initialise the enviroment potential. Use this to build different scenarios.
*/
void SSFM1D::initialize_enviroment_potential() {
    // Number of grid points to pad on each side of the potential well
    // Maybe come up with an expressing based on dx so that the padding has a fixed lenght. The padding should be large enough to avoid any quantum tunneling through the boundaries of the grid.
    // If there is quantum tunneling through the boundary, waves will wrap around to the other side due to the perodicity of fourier transform.
    enviroment_potential.resize(N);
    int wall = (4.0 / 1024.0) * N;
    int wall_start = (256.0 / 1024.0) * N;

    for (size_t i = 0; i < enviroment_potential.size(); ++i) {
        if (i < padding || i >= enviroment_potential.size() - padding) {
            enviroment_potential[i] = boundary_pot;
        } else {
            enviroment_potential[i] = 0;
        }
    }
    
    if (tunneling_wall_enabled) {
        std::cerr << "Tunneling wall enabled with potential: " << tunneling_pot << std::endl;
        for (int i = 0; i < wall; ++i) {
            enviroment_potential[wall_start + i] = tunneling_pot;
        }
    }
}

/*
Initialise the potential evolution operator in real space: $U_V(x) = \exp{(-i \frac{V(x) \Delta t}{2 \hbar})}$
*/
void SSFM1D::initialize_potential_operator() {
    U_p.resize(N);
    for (size_t i = 0; i < U_p.size(); ++i) {
        //U_p[i] = std::exp(std::complex<double>(0, -enviroment_potential[i] * dt / (2 * hbar)));
        U_p[i] = std::polar(1.0, -enviroment_potential[i] * dt / (2 * hbar));
    }
}

/*
Precompute the kinetic evolution propagator in Fourier space: $U_t(k) = \exp{(-i \frac{\hbar k^2 \Delta t}{2m})}$
*/
void SSFM1D::initialise_kinetic_operator() {
    k.resize(N);
    U_k.resize(N);
    dk = 2 * M_PI / L;

    for (size_t i = 0; i < k.size(); ++i) {
        if (i < N / 2) {
            k[i] = i * dk;
        } else {
            k[i] = (i - N) * dk;
        }
    }

    for (size_t i = 0; i < U_k.size(); ++i) {
        //U_k[i] = std::exp(std::complex<double>(0, -hbar * k[i] * k[i] * dt / (2 * m)));
        U_k[i] = std::polar(1.0, -hbar * k[i] * k[i] * dt / (2 * m));
    }
}

/*
Make sure to normalise such that: $\sum_{j=0}^{N-1} |\psi(x_i)|^2 \Delta x = 1$
*/
void SSFM1D::normalize_wavefunction() {
    double norm = 0;
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        norm += std::norm(waveFunction[i]) * dx;
    }
    norm = sqrt(norm);
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] /= norm;
    }
}


/*
Half step potential evolution: $\psi(x, t + \frac{\Delta t}{2}) = \psi(x, t) \cdot \exp{(-i \frac{V(x) \Delta t}{2 \hbar})} = \psi(x, t) \cdot U_V(x)$
*/
void SSFM1D::potential_half_step() {
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] *= U_p[i];
    }
}

void SSFM1D::forward_fft() {
    fftw_execute(plan_forward);
}

/*
$\phi(k, t + \Delta t) = \phi(jk, t) \cdot \exp{(-i \frac{\hbar k^2 \Delta t}{2m})} = \phi(k, t) \cdot U_t(k)$
*/
void SSFM1D::kinetic_full_step() {
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] *= U_k[i];
    }
}

void SSFM1D::inverse_fft() {
    fftw_execute(plan_backward);
}

void SSFM1D::potential_half_step_and_normalize() {
    double inv_N = 1.0 / N;
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] = (waveFunction[i] * inv_N) * U_p[i];
    }
}

void SSFM1D::step() {
    potential_half_step();
    forward_fft();
    kinetic_full_step();
    inverse_fft();
    potential_half_step_and_normalize();
    //normalize_wavefunction();
}

/*
Calculate the probability density given by: $P(x) = \psi(x)^* \psi(x) = |\psi(x)|^2$
*/
std::vector<double> SSFM1D::calculate_probability_density() {
    std::vector<double> probability_density(waveFunction.size());
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        probability_density[i] = std::norm(waveFunction[i]);
    }
    return probability_density;
}

double SSFM1D::calculate_area_under_probability_density() {
    double area = 0;
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        area += std::norm(waveFunction[i]) * dx;
    }
    return area;
}

void SSFM1D::output_wavefunction(const std::string& path_to_file, double current_time) {
    std::string filename = "wavefunction_" + std::to_string(current_time) + ".txt";
    FILE* file = fopen((path_to_file + filename).c_str(), "w");
    
    if (file == nullptr) {
        std::cerr << "Error opening file for writing: " << path_to_file << std::endl;
        return;
    }

    double area = calculate_area_under_probability_density();
    fprintf(file, "%f\n", area);

    std::vector<double> probability_density = calculate_probability_density();

    for (size_t i = 0; i < waveFunction.size(); ++i) {
        fprintf(file, "%f %f %f %f\n", grid[i], std::real(waveFunction[i]), std::imag(waveFunction[i]), probability_density[i]);
    }

    fclose(file);
}

void SSFM1D::output_enviroment_potential(const std::string& path_to_file) {
    std::string filename = "enviroment_potential.txt";
    FILE* file = fopen((path_to_file + filename).c_str(), "w");
    
    if (file == nullptr) {
        std::cerr << "Error opening file for writing: " << path_to_file << std::endl;
        return;
    }

    for (size_t i = 0; i < enviroment_potential.size(); ++i) {
        fprintf(file, "%f %f\n", grid[i], enviroment_potential[i]);
    }

    fclose(file);
}
