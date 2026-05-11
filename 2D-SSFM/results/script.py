import sys
import os
import subprocess
from pathlib import Path
import animate_wavefunction


def call_SSFM1D(Nx, Ny, Lx, padding, time, dt=0.1, wx=40, wy=40):
	"""Call the SSFM1D solver with 2D parameters."""
	subprocess.run([
		"../src/build/SSFM1D", 
		"-Nx", str(Nx), 
		"-Ny", str(Ny),
		"-Lx", str(Lx), 
		"-P", str(padding), 
		"-t", str(time), 
		"-dt", str(dt), 
		"-wx", str(wx),
		"-wy", str(wy)
	])


def animate(title="", output_file="wavefunction.mp4", fps=20, output_dir="output"):
	"""Create animation from output files.
	
	Args:
		title: Optional title for the animation
		output_file: Output video filename
		fps: Frames per second
		output_dir: Directory containing wavefunction output files
	"""
	sys.argv = ["animate_wavefunction.py"]  # Reset argv for argparse
	if title:
		sys.argv.extend(["--title", title])
	sys.argv.extend(["--output", output_file])
	sys.argv.extend(["--fps", str(fps)])
	sys.argv.extend(["--output-dir", output_dir])
	
	animate_wavefunction.main()


def test_spacial_equality():
	"""Test convergence with different spatial resolutions (Nx, Ny values)"""
	base_Nx = 256
	base_Ny = 256
	base_padding = 25
	time = 0.01
	Lx = 1.0

	for Nx in [128, 256]:
		Ny = Nx  # Keep square grids
		padding = base_padding
		
		print(f"\n{'='*60}")
		print(f"Running simulation: Nx={Nx}, Ny={Ny}, Lx={Lx}")
		print(f"{'='*60}")
		call_SSFM1D(Nx, Ny, Lx, padding, time)
		
		title = f"TDSE SSFM 2D: Spatial Resolution Nx={Nx}, Ny={Ny}"
		output = f"tdse_spatial_Nx{Nx}_Ny{Ny}.mp4"
		animate(title=title, output_file=output, fps=30)


def test_dt_stability():
	"""Test stability with different dt safety factors"""
	Nx = 128
	Ny = 128
	Lx = 1.0
	padding = 25
	time = 0.01
	
	for dt in [0.05, 0.1, 0.5]:
		print(f"\n{'='*60}")
		print(f"Running simulation: dt_safety={dt}")
		print(f"{'='*60}")
		call_SSFM1D(Nx, Ny, Lx, padding, time, dt=dt)
		
		title = f"TDSE SSFM 2D: Stability Test dt={dt}"
		output = f"tdse_stability_dt{dt}.mp4"
		animate(title=title, output_file=output, fps=30)


def test_wave_momentum():
	"""Test different initial wave numbers (momentum) in x and y"""
	Nx = 128
	Ny = 128
	Lx = 1.0
	padding = 25
	time = 0.01
	
	for wx in [5, 10, 20]:
		wy = wx  # Keep symmetric for simplicity
		print(f"\n{'='*60}")
		print(f"Running simulation: wx={wx}, wy={wy}")
		print(f"{'='*60}")
		call_SSFM1D(Nx, Ny, Lx, padding, time, wx=wx, wy=wy)
		
		title = f"TDSE SSFM 2D: Wave Momentum wx={wx}, wy={wy}"
		output = f"tdse_momentum_wx{wx}_wy{wy}.mp4"
		animate(title=title, output_file=output, fps=30)


def test_asymmetric_momentum():
	"""Test asymmetric wave numbers (different momentum in x and y)"""
	Nx = 128
	Ny = 128
	Lx = 1.0
	padding = 25
	time = 0.01
	
	print(f"\n{'='*60}")
	print(f"Running simulation: Asymmetric momentum wx=10, wy=5")
	print(f"{'='*60}")
	call_SSFM1D(Nx, Ny, Lx, padding, time, wx=10, wy=5)
	
	title = "TDSE SSFM 2D: Asymmetric Wave Packet (wx=10, wy=5)"
	output = "tdse_asymmetric_momentum.mp4"
	animate(title=title, output_file=output, fps=30)


def main():
	# Uncomment the test(s) you want to run
	test_spacial_equality()
	# test_dt_stability()
	# test_wave_momentum()
	# test_asymmetric_momentum()


if __name__ == "__main__":
	main()