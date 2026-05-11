from pathlib import Path
import re
import argparse

import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np


# Parameters
OUTPUT_DIR = Path(__file__).resolve().parent.parent / "output"
PATTERN = "wavefunction_*.txt"
OUTPUT_MP4 = Path(__file__).resolve().parent / "wavefunction.mp4"
FIGSIZE = (16, 10)
OUTPUT_DPI = 100
BASE_HEIGHT = 6.0


def frame_time(path: Path) -> float:
	m = re.search(r"wavefunction_([0-9]+(?:\.[0-9]+)?)\.txt$", path.name)
	return float(m.group(1)) if m else -1.0


def get_grid_dimensions(data):
	"""
	Determine Nx, Ny from 2D wavefunction data.
	Data is organized as: for each y-value (outer loop), for each x-value (inner loop).
	Returns Nx, Ny, x_unique, y_unique
	"""
	x = data[:, 0]
	y = data[:, 1]
	
	# Find where x resets (indicates new y-row)
	x_unique = np.unique(x)
	y_unique = np.unique(y)
	
	Nx = len(x_unique)
	Ny = len(y_unique)
	
	return Nx, Ny, x_unique, y_unique


def figure_size(x_unique, y_unique, panels=1):
	x_span = x_unique[-1] - x_unique[0]
	y_span = y_unique[-1] - y_unique[0]
	width = BASE_HEIGHT * panels * (x_span / y_span)
	return (width, BASE_HEIGHT)


def load_frame(data, Nx, Ny):
	# Data columns: 0=x, 1=y, 2=Re[ψ], 3=Im[ψ], 4=|ψ|²
	prob = data[:, 4].reshape((Ny, Nx))
	re_psi = data[:, 2].reshape((Ny, Nx))
	im_psi = data[:, 3].reshape((Ny, Nx))
	return prob, re_psi, im_psi


def parse_args():
	parser = argparse.ArgumentParser(description="Animate saved 2D wavefunction snapshots.")
	parser.add_argument("--title", default=None, help="Optional base title for the animation.")
	parser.add_argument(
		"--output",
		default=str(OUTPUT_MP4),
		type=str,
		help="Base output video filename or path. Probability and wavefunction files are created with suffixes.",
	)
	parser.add_argument(
		"--fps",
		default=None,
		type=float,
		help="Frames per second used when saving the animation.",
	)
	parser.add_argument(
		"--output-dir",
		default=str(OUTPUT_DIR),
		type=str,
		help="Directory containing wavefunction output files.",
	)
	return parser.parse_args()


def main():
	args = parse_args()
	output_dir = Path(args.output_dir)
	files = sorted(output_dir.glob(PATTERN), key=frame_time)
	if not files:
		raise FileNotFoundError(f"No files found in {output_dir} matching {PATTERN}")

	# Load first frame to determine grid dimensions
	first_data = np.loadtxt(files[0])
	Nx, Ny, x_unique, y_unique = get_grid_dimensions(first_data)
	output_base = Path(args.output)
	if output_base.suffix:
		output_base = output_base.with_suffix("")
	prob_output = output_base.with_name(f"{output_base.name}_probability.mp4")
	wave_output = output_base.with_name(f"{output_base.name}_wavefunctions.mp4")
	extent = [x_unique[0], x_unique[-1], y_unique[0], y_unique[-1]]
	frame_label_prefix = args.title if args.title else "2D Wavefunction Evolution"
	frame_size = figure_size(x_unique, y_unique, panels=1)
	wave_size = figure_size(x_unique, y_unique, panels=2)

	# Animation parameters
	fps = args.fps if args.fps is not None else 20
	interval_ms = 1000 / fps

	# Probability density animation
	fig_prob, ax_prob = plt.subplots(1, 1, figsize=frame_size, constrained_layout=True)
	fig_prob.suptitle(frame_label_prefix, fontsize=14, fontweight='bold')
	im_prob = ax_prob.imshow(
		np.zeros((Ny, Nx)),
		extent=extent,
		origin='lower',
		cmap='magma',
		interpolation='bicubic',
		aspect='equal',
	)
	ax_prob.set_title('|ψ|² (Probability Density)')
	ax_prob.set_xlabel('x')
	ax_prob.set_ylabel('y')
	fig_prob.colorbar(im_prob, ax=ax_prob)

	def update_prob(frame_idx):
		data = np.loadtxt(files[frame_idx])
		prob, _, _ = load_frame(data, Nx, Ny)
		prob_scale = max(prob.max(), np.finfo(float).eps)
		im_prob.set_array(prob)
		im_prob.set_clim(vmin=0.0, vmax=prob_scale)
		frame_label = f"t = {frame_time(files[frame_idx]):.6f}"
		fig_prob.suptitle(f"{frame_label_prefix} | {frame_label}", fontsize=14, fontweight='bold')
		return (im_prob,)

	anim_prob = FuncAnimation(fig_prob, update_prob, frames=len(files), interval=interval_ms, blit=False)
	print(f"Saving animation to {prob_output}...")
	anim_prob.save(str(prob_output), fps=fps, dpi=OUTPUT_DPI)
	plt.close(fig_prob)

	# Wavefunction animation
	fig_wave, axes = plt.subplots(1, 2, figsize=wave_size, constrained_layout=True)
	fig_wave.suptitle(frame_label_prefix, fontsize=14, fontweight='bold')
	im_re = axes[0].imshow(
		np.zeros((Ny, Nx)),
		extent=extent,
		origin='lower',
		cmap='RdBu_r',
		interpolation='bicubic',
		aspect='equal',
	)
	im_im = axes[1].imshow(
		np.zeros((Ny, Nx)),
		extent=extent,
		origin='lower',
		cmap='RdBu_r',
		interpolation='bicubic',
		aspect='equal',
	)
	axes[0].set_title('Re[ψ]')
	axes[1].set_title('Im[ψ]')
	for ax in axes:
		ax.set_xlabel('x')
		ax.set_ylabel('y')
	fig_wave.colorbar(im_re, ax=axes[0])
	fig_wave.colorbar(im_im, ax=axes[1])

	def update_wave(frame_idx):
		data = np.loadtxt(files[frame_idx])
		_, re_psi, im_psi = load_frame(data, Nx, Ny)
		re_scale = max(np.max(np.abs(re_psi)), np.finfo(float).eps)
		im_scale = max(np.max(np.abs(im_psi)), np.finfo(float).eps)
		im_re.set_array(re_psi)
		im_im.set_array(im_psi)
		im_re.set_clim(vmin=-re_scale, vmax=re_scale)
		im_im.set_clim(vmin=-im_scale, vmax=im_scale)
		frame_label = f"t = {frame_time(files[frame_idx]):.6f}"
		fig_wave.suptitle(f"{frame_label_prefix} | {frame_label}", fontsize=14, fontweight='bold')
		return im_re, im_im

	anim_wave = FuncAnimation(fig_wave, update_wave, frames=len(files), interval=interval_ms, blit=False)
	print(f"Saving animation to {wave_output}...")
	anim_wave.save(str(wave_output), fps=fps, dpi=OUTPUT_DPI)
	plt.close(fig_wave)
	print(f"Animation saved successfully!")


if __name__ == "__main__":
	main()
