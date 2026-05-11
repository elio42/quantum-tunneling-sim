#include "ssfm2d.h"

/*
2D Split-Step Fourier Method (SSFM) solver.
Simulates electron wavefunction evolution under the time-dependent Schrödinger equation.
*/
SSFM2D::SSFM2D(double Nx, double Ny, double Lx, size_t padding, 
               double dt_saftey_factor, int number_of_waves_x, int number_of_waves_y) 
        : Lx(Lx), Nx(Nx), Ny(Ny), padding(padding), dt_saftey_factor(dt_saftey_factor), 
      number_of_waves_x(number_of_waves_x), number_of_waves_y(number_of_waves_y) {
    
    waveFunction.resize((size_t)Nx * (size_t)Ny);
    configure_fftw_plans();
    set_parameters();
    initialize_frequency_grids();
    initialize_grid();
    initialize_wavefunction();
    initialize_enviroment_potential();
    initialize_potential_operator();
    initialise_kinetic_operator();
}

SSFM2D::~SSFM2D() {
    fftw_destroy_plan(plan_forward);
    fftw_destroy_plan(plan_backward);
}

void SSFM2D::configure_fftw_plans() {
    psi_in = reinterpret_cast<fftw_complex*>(waveFunction.data());
    plan_forward = fftw_plan_dft_2d((int)Ny, (int)Nx, psi_in, psi_in, FFTW_FORWARD, FFTW_MEASURE);
    plan_backward = fftw_plan_dft_2d((int)Ny, (int)Nx, psi_in, psi_in, FFTW_BACKWARD, FFTW_MEASURE);
}

/*
Calculate the maximum time step to ensure numerical stability.

!Old:The time step should satisfy the stability condition: $\Delta t \ll \min{(\frac{2m (\Delta x)^2}{\pi \hbar}, \frac{\pi \hbar}{V_{max}})}$
TODO: Verify stability constraint in 2D: Δt must account for max(kx² + ky²) = 2π²(1/h²) - **Code** shold be correct as is now. Maybe check the literature again in the future.

$T_{max} \le \frac{\hbar^2}{2m}(k_{x,max}^2 + k_{y,max}^2)$. Since $k_{max} \approx \pi/h$
*/
void SSFM2D::set_parameters() {
    h = Lx / Nx;
    Ly = Ny * h;
    dk_x = 2 * M_PI / Lx;
    dk_y = 2 * M_PI / Ly;
    
    double dt_V = (boudary_potential > 1e-9) ? (M_PI * hbar) / boudary_potential : std::numeric_limits<double>::max();
    double dt_kinetic = (m * h * h) / (M_PI * hbar);
    dt = dt_saftey_factor * std::min(dt_kinetic, dt_V);
}

/*
Initialize 1D frequency grids for kx and ky with FFTW half-positive-half-negative layout.
dk_x = 2π/Lx, dk_y = 2π/Ly
*/
void SSFM2D::initialize_frequency_grids() {
    k_x.resize((size_t)Nx);
    k_y.resize((size_t)Ny);
    
    // Fill k_x with FFTW layout: [0, dk_x, 2*dk_x, ..., (Nx/2-1)*dk_x, -(Nx/2)*dk_x, ..., -dk_x]
    for (size_t i = 0; i < k_x.size(); ++i) {
        if (i < (size_t)Nx / 2) {
            k_x[i] = i * dk_x;
        } else {
            k_x[i] = (i - (size_t)Nx) * dk_x;
        }
    }
    
    // Fill k_y similarly with dk_y = 2π/Ly
    for (size_t j = 0; j < k_y.size(); ++j) {
        if (j < (size_t)Ny / 2) {
            k_y[j] = j * dk_y;
        } else {
            k_y[j] = (j - (size_t)Ny) * dk_y;
        }
    }
}

/*
Initialize 2D coordinate grids.
Grid spans [-Lx/2, Lx/2] in x-direction and [-Ly/2, Ly/2] in y-direction with spacing h.
*/
void SSFM2D::initialize_grid() {
    grid_x.resize((size_t)Nx);
    grid_y.resize((size_t)Ny);
    
    for (size_t i = 0; i < grid_x.size(); ++i) {
        grid_x[i] = -Lx / 2 + i * h;
    }
    
    for (size_t j = 0; j < grid_y.size(); ++j) {
        grid_y[j] = -Ly / 2 + j * h;
    }
}

/*
Initialize the wavefunction with a 2D separable Gaussian wave packet:

$k_{max} = \pi / h$ for both x and y directions.
I then apply a hardcoded saftey factor of 0.5. Though this only prints something to stderr.
*/
void SSFM2D::initialize_wavefunction() {
    double sigma = Lx / 10;  // Use Lx as reference
    double k0x = number_of_waves_x * (2.0 * M_PI / Lx);
    double k0y = number_of_waves_y * (2.0 * M_PI / Ly);
    
    double saftey_factor = 0.5; // Reduce k0 to be safely within the Nyquist limit
    double aliasing_limit_x = (M_PI / h) * saftey_factor;
    double aliasing_limit_y = (M_PI / h) * saftey_factor;
    
    if (k0x >= aliasing_limit_x) {
        std::cerr << "Warning: k0x is large, this may lead to aliasing" << std::endl;
    }
    if (k0y >= aliasing_limit_y) {
        std::cerr << "Warning: k0y is large, this may lead to aliasing" << std::endl;
    }
    
    double x0 = 0, y0 = 0;
    
    for (size_t j = 0; j < (size_t)Ny; ++j) {
        for (size_t i = 0; i < (size_t)Nx; ++i) {
            double x = grid_x[i];
            double y = grid_y[j];
            double gauss = (1 / pow(2 * M_PI * sigma * sigma, 0.25)) *
                          std::exp(-((x - x0) * (x - x0) + (y - y0) * (y - y0)) / (4 * sigma * sigma));
            double phase = k0x * x + k0y * y;
            waveFunction[index(i, j)] = gauss * std::exp(std::complex<double>(0.0, phase));
        }
    }
    
    normalize_wavefunction();
}

/*
Initialise the enviroment potential. Use this to build different scenarios.
*/
void SSFM2D::initialize_enviroment_potential() {
    // Number of grid points to pad on each side of the potential well
    // Maybe come up with an expressing based on dx so that the padding has a fixed lenght. The padding should be large enough to avoid any quantum tunneling through the boundaries of the grid.
    // If there is quantum tunneling through the boundary, waves will wrap around to the other side due to the perodicity of fourier transform.

    // int wall = 6;
    // double wall_potential = 1e4;

    enviroment_potential.resize((size_t)Nx * (size_t)Ny, 0.0);
    
    for (size_t j = 0; j < (size_t)Ny; ++j) {
        for (size_t i = 0; i < (size_t)Nx; ++i) {
            bool is_x_boundary = (i < padding) || (i >= (size_t)Nx - padding);
            bool is_y_boundary = (j < padding) || (j >= (size_t)Ny - padding);
            
            if (is_x_boundary || is_y_boundary) {
                enviroment_potential[index(i, j)] = boudary_potential;
            } else {
                enviroment_potential[index(i, j)] = 0.0;
            }
        }
    }
}

/*
Initialise the potential evolution operator in real space: $U_V(x, y) = \exp{(-i \frac{V(x, y) \Delta t}{2 \hbar})}$
*/
void SSFM2D::initialize_potential_operator() {
    U_p.resize((size_t)Nx * (size_t)Ny);
    
    for (size_t j = 0; j < (size_t)Ny; ++j) {
        for (size_t i = 0; i < (size_t)Nx; ++i) {
            size_t idx = index(i, j);
            U_p[idx] = std::polar(1.0, -enviroment_potential[idx] * dt / (2 * hbar));
        }
    }
}

/*
Precompute the kinetic evolution propagator in Fourier space: $U_t(k_x, k_y) = \exp{(-i \frac{\hbar (k_x^2 + k_y^2) \Delta t}{2m})}$
*/
void SSFM2D::initialise_kinetic_operator() {
    U_k.resize((size_t)Nx * (size_t)Ny);
    
    for (size_t j = 0; j < (size_t)Ny; ++j) {
        for (size_t i = 0; i < (size_t)Nx; ++i) {
            double k_sq = k_x[i] * k_x[i] + k_y[j] * k_y[j];
            U_k[index(i, j)] = std::polar(1.0, -hbar * k_sq * dt / (2 * m));
        }
    }
}

/*
Normalise wavefunction such that: $\sum_{j=0}^{N-1} |\psi(x_i, y_j)|^2 \Delta x \Delta y = 1$
*/
void SSFM2D::normalize_wavefunction() {
    double norm = 0;
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        norm += std::norm(waveFunction[i]) * h * h;
    }
    norm = sqrt(norm);
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] /= norm;
    }
}

/*
Calculate norm (should be close to 1.0 if numerical scheme is stable).
*/
double SSFM2D::calculate_norm() {
    double norm = 0;
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        norm += std::norm(waveFunction[i]) * h * h;
    }
    return norm;
}

/*
Half-step potential evolution: $\psi(x, y, t + \frac{\Delta t}{2}) = \psi(x, y, t) \cdot \exp{(-i \frac{V(x, y) \Delta t}{2 \hbar})} = \psi(x, y, t) \cdot U_V(x, y)$
*/
void SSFM2D::potential_half_step() {
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] *= U_p[i];
    }
}

void SSFM2D::forward_fft() {
    fftw_execute(plan_forward);
}

/*
$\phi(k, t + \Delta t) = \phi(jk, t) \cdot \exp{(-i \frac{\hbar k^2 \Delta t}{2m})} = \phi(k, t) \cdot U_t(k)$
*/
void SSFM2D::kinetic_full_step() {
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] *= U_k[i];
    }
}

void SSFM2D::inverse_fft() {
    fftw_execute(plan_backward);
}

/*
Half-step potential evolution with 1/(Nx*Ny) IFFT normalization.
*/
void SSFM2D::potential_half_step_and_normalize() {
    double inv_N = 1.0 / ((size_t)Nx * (size_t)Ny);
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        waveFunction[i] = (waveFunction[i] * inv_N) * U_p[i];
    }
}

/*
Strang splitting step: V/2 -> FFT -> K -> IFFT -> V/2
*/
void SSFM2D::step() {
    potential_half_step();
    forward_fft();
    kinetic_full_step();
    inverse_fft();
    potential_half_step_and_normalize();
    
    // Print norm to stdout for monitoring
    // double norm = calculate_norm();
    // printf("Norm: %.15f\n", norm);
}

/*
Calculate the probability density given by: $P(x) = \psi(x)^* \psi(x) = |\psi(x)|^2$
*/
std::vector<double> SSFM2D::calculate_probability_density() {
    std::vector<double> probability_density(waveFunction.size());
    for (size_t i = 0; i < waveFunction.size(); ++i) {
        probability_density[i] = std::norm(waveFunction[i]);
    }
    return probability_density;
}

/*
Output wavefunction to file in format: x y Re(ψ) Im(ψ) |ψ|²
*/
void SSFM2D::output_wavefunction(const std::string& path_to_file, double current_time) {
    std::string filename = "wavefunction_" + std::to_string(current_time) + ".txt";
    FILE* file = fopen((path_to_file + filename).c_str(), "w");
    
    if (file == nullptr) {
        std::cerr << "Error opening file for writing: " << path_to_file << std::endl;
        return;
    }

    std::vector<double> probability_density = calculate_probability_density();

    for (size_t j = 0; j < (size_t)Ny; ++j) {
        for (size_t i = 0; i < (size_t)Nx; ++i) {
            size_t idx = index(i, j);
            fprintf(file, "%f %f %f %f %f\n", 
                    grid_x[i], grid_y[j], 
                    std::real(waveFunction[idx]), 
                    std::imag(waveFunction[idx]), 
                    probability_density[idx]);
        }
    }

    fclose(file);
}
