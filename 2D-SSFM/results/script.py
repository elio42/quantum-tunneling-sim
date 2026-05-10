import sys
import os
import subprocess

def call_SSFM1D(N, L, padding, time, dt=0.1, w=40):
    subprocess.run(["build/SSFM1D", "-N", str(N), "-L", str(L), "-P", str(padding), "-t", str(time), "-dt", str(dt), "-w", str(w)])

def call_animation_script(title, output, fps):
    subprocess.run(["python3", "results/animate_wavefunction.py", "--title", title, "--output", output, "--fps", str(fps)])

def test_spacial_equality():
    """Test convergence with different spatial resolutions (N values)"""
    base_N = 1024
    base_padding = 25
    time = 0.07
    L = 1

    for N in [1024, 2048]:
        padding = base_padding * (N // base_N)
        
        call_SSFM1D(N, L, padding, time)
        
        title = f"TDSE SSFM: Spatial Convergence N={N}"
        output = f"results/tdse_spacial_N{N}.mp4"
        call_animation_script(title, output, 30)

def test_dt_stability():
    """Test stability with different dt safety factors"""
    N = 1024
    L = 1
    padding = 25
    time = 0.07
    
    for dt in [0.01, 0.1, 1.0]:
        call_SSFM1D(N, L, padding, time, dt=dt)
        
        title = f"TDSE SSFM: Stability dt={dt}"
        output = f"results/tdse_stability_dt{dt}.mp4"
        call_animation_script(title, output, 30)

def test_wave_momentum():
    """Test different initial wave numbers (momentum)"""
    N = 1024
    L = 1
    padding = 25
    time = 0.07
    
    for w in [40, 80, 120]:
        call_SSFM1D(N, L, padding, time, w=w)
        
        title = f"TDSE SSFM: Wave Number w={w}"
        output = f"results/tdse_momentum_w{w}.mp4"
        call_animation_script(title, output, 30)

def test_domain_scaling():
    """Test larger domain with doubled resolution"""
    N = 2048
    L = 2
    padding = 50
    time = 0.07
    
    call_SSFM1D(N, L, padding, time)
    
    title = f"TDSE SSFM: Larger Domain L={L}, N={N}"
    output = f"results/tdse_domain_L{L}_N{N}.mp4"
    call_animation_script(title, output, 30)

def main():
    test_spacial_equality()
    test_dt_stability()
    test_wave_momentum()
    test_domain_scaling()

if __name__ == "__main__":
    main()