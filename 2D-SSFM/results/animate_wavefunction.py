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
FIGSIZE = (12.8, 7.2)  # ~16:9 aspect ratio
OUTPUT_DPI = 100


def frame_time(path: Path) -> float:
	m = re.search(r"wavefunction_([0-9]+(?:\.[0-9]+)?)\.txt$", path.name)
	return float(m.group(1)) if m else -1.0


def parse_args():
	parser = argparse.ArgumentParser(description="Animate saved wavefunction snapshots.")
	parser.add_argument("--title", default=None, help="Optional base title for the animation.")
	parser.add_argument(
		"--output",
		default=str(OUTPUT_MP4),
		type=str,
		help="Output video filename or path. .mp4 is added automatically when no extension is given.",
	)
	parser.add_argument(
		"--fps",
		default=None,
		type=float,
		help="Frames per second used when saving the animation.",
	)
	return parser.parse_args()


def main():
	args = parse_args()
	files = sorted(OUTPUT_DIR.glob(PATTERN), key=frame_time)
	if not files:
		raise FileNotFoundError(f"No files found in {OUTPUT_DIR} matching {PATTERN}")

	first = np.atleast_2d(np.loadtxt(files[0]))
	x = first[:, 0]

	fig, ax = plt.subplots(figsize=FIGSIZE)
	line_re, = ax.plot(x, first[:, 1], label="Re[psi]", color="#1fb45d96", lw=1.0)
	line_im, = ax.plot(x, first[:, 2], label="Im[psi]", color="#0e9fff81", lw=1.0)
	line_prob, = ax.plot(x, first[:, 3], label="Probability density", color="#a02c2c", lw=1.0)
	ax.set_xlabel("x")
	ax.set_ylabel("psi")
	ax.legend()

	fps = args.fps if args.fps is not None else 1000 / 50
	interval_ms = 1000 / fps
	output_mp4 = Path(args.output)
	if output_mp4.suffix == "":
		output_mp4 = output_mp4.with_suffix(".mp4")

	def update(i):
		data = np.atleast_2d(np.loadtxt(files[i]))
		line_re.set_ydata(data[:, 1])
		line_im.set_ydata(data[:, 2])
		line_prob.set_ydata(data[:, 3])
		frame_label = f"current time = {frame_time(files[i]):.6f}"
		if args.title:
			ax.set_title(f"{args.title} | {frame_label}")
		else:
			ax.set_title(frame_label)
		return line_re, line_im, line_prob


	anim = FuncAnimation(fig, update, frames=len(files), interval=interval_ms, blit=False)
	anim.save(output_mp4, fps=fps, dpi=OUTPUT_DPI)


if __name__ == "__main__":
	main()
