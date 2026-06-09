import subprocess
from pathlib import Path
from animate_wavefunction import animate_wavefunction

# Paths
BASE_DIR = Path(__file__).resolve().parent
EXEC_PATH = BASE_DIR / "build" / "SSFM1D"
OUTPUT_DATA_DIR = BASE_DIR / "output"
RESULTS_DIR = BASE_DIR / "results"

def run_and_animate(params: dict, title: str, output_name: str):
    """Helper to run the C++ simulation and immediately generate the animation."""
    RESULTS_DIR.mkdir(parents=True, exist_ok=True)
    
    # Construct command using long argument names
    cmd = [str(EXEC_PATH)]
    for key, val in params.items():
        cmd.extend([f"--{key}", str(val)])
    
    print(f"Running: {title} ...")
    subprocess.run(cmd, check=True)
    
    # Extract only physics-related parameters (using C++ defaults if not specified)
    physics_params = {
        "L": params.get("length", 3),
        "σ": params.get("sigma", 0.04),
        "w": params.get("number-of-waves", 40),
        "V_b": params.get("boundary-potential", 1e6)
    }
    param_text = "\n".join([f"{k} = {v}" for k, v in physics_params.items()])
    
    output_mp4 = RESULTS_DIR / output_name
    animate_wavefunction(
        output_dir=OUTPUT_DATA_DIR,
        output_mp4=output_mp4,
        fps=30.0,
        title=title,
        param_text=param_text  # Pass the formatted string
    )
    print(f"Completed: {output_mp4.name}\n")

def test_spatial_resolution():
    """Test identical physics with different grid resolutions."""
    base_params = {"length": 3, "total-time": 0.1, "delta-time": 0.5}
    
    for N in [512, 1024, 2048]:
        params = base_params.copy()
        params["number-of-points"] = N
        params["padding"] = int(50 * (N / 1024)) # Scale padding with N
        
        run_and_animate(
            params,
            title=f"Spatial Resolution (N={N})",
            output_name=f"resolution_N{N}.mp4"
        )

def test_high_momentum_failure():
    """Increase momentum until numerical dispersion/aliasing destroys the packet."""
    base_params = {"number-of-points": 1024, "length": 3, "total-time": 0.05}
    
    # 40 is stable, 100 pushes limits, 250 should show severe numerical artifacts
    for w in [40, 100, 250]:
        params = base_params.copy()
        params["number-of-waves"] = w
        
        run_and_animate(
            params,
            title=f"High Momentum Failure (w={w})",
            output_name=f"momentum_w{w}.mp4"
        )

def test_dt_stability():
    """Test safety factor for the time step. SSFM accumulates phase errors at high dt."""
    base_params = {"number-of-points": 1024, "length": 3, "total-time": 0.1}
    
    # 0.5 is safe, 2.0 pushes error limits, 10.0 should show severe deviations
    for dt in [0.5, 2.0, 10.0]:
        params = base_params.copy()
        params["delta-time"] = dt
        
        run_and_animate(
            params,
            title=f"Time Step Stability (dt factor={dt})",
            output_name=f"stability_dt{dt}.mp4"
        )

def test_sigma_variation():
    """Test wave packet spread in a wider domain."""
    base_params = {"number-of-points": 2048, "length": 7, "total-time": 0.15}
    
    for sigma in [0.05, 0.2, 0.5]:
        params = base_params.copy()
        params["sigma"] = sigma
        
        run_and_animate(
            params,
            title=f"Wave Packet Width (sigma={sigma})",
            output_name=f"sigma_{sigma}.mp4"
        )

def test_boundary_tunneling():
    """SUGGESTED: Lower boundary potential to observe finite wall penetration."""
    base_params = {"number-of-points": 1024, "length": 3, "total-time": 0.1}
    
    # 1e6 acts as infinite well, 5000 allows tails, 500 allows severe leakage/tunneling
    for b in [1e6, 5000, 500]:
        params = base_params.copy()
        params["boundary-potential"] = b
        
        run_and_animate(
            params,
            title=f"Finite Wall Penetration (V_b={b})",
            output_name=f"boundary_V{b}.mp4"
        )

def test_domain_lengths():
    """Test the simulation in boxes of different lengths with constant N=1024."""
    # Using defaults: N=1024, total-time=0.1, delta-time=0.5
    base_params = {"number-of-points": 1024, "total-time": 0.1}
    
    for L in [1, 2, 5]:
        params = base_params.copy()
        params["length"] = L
        
        # Ensure padding is sufficient for the specific length
        params["padding"] = 50 
        
        run_and_animate(
            params,
            title=f"Domain Length Study (L={L})",
            output_name=f"length_L{L}.mp4"
        )

def test_tunneling_pot():
    "Testing different wall pots for tunneling"
    base_params = {"number-of-points": 1024, "length": 3, "total-time": 0.1, "sigma": 0.18}
    
    for pot in [1e2, 1e3, 1e4]:
        params = base_params.copy()
        params["tunneling-pot"] = pot
        
        run_and_animate(
            params,
            title=f"Domain Length Study (L={pot})",
            output_name=f"Tunneling_Potential{pot}.mp4"
        )

if __name__ == "__main__":
    print("Starting ESC 202 Simulation Sweeps...\n")
    # test_spatial_resolution()
    # test_dt_stability()
    # test_domain_lengths()
    # test_high_momentum_failure()
    # test_sigma_variation()
    # test_boundary_tunneling()
    test_tunneling_pot()
    print("All sweeps completed successfully.")