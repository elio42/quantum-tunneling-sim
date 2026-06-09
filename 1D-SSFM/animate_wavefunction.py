import re
from pathlib import Path
import matplotlib.pyplot as plt
from matplotlib.animation import FuncAnimation
import numpy as np

def get_frame_time(path: Path) -> float:
    m = re.search(r"wavefunction_([0-9]+(?:\.[0-9]+)?)\.txt$", path.name)
    return float(m.group(1)) if m else -1.0

def load_frame(path: Path):
    with open(path) as file:
        probability_density = float(file.readline())
    data = np.atleast_2d(np.loadtxt(path, skiprows=1))
    return probability_density, data

def animate_wavefunction(output_dir: Path, output_mp4: Path, fps: float = 50.0, title: str = None, param_text: str = None):
    FIGSIZE = (12.8, 7.2)
    DPI = 100
    
    files = sorted(Path(output_dir).glob("wavefunction_*.txt"), key=get_frame_time)
    probability_density, first_data = load_frame(files[0])
    x = first_data[:, 0]

    fig, ax_psi = plt.subplots(figsize=FIGSIZE)

    # --- Left Axis: Wavefunction (Linear) ---
    line_re, = ax_psi.plot(x, first_data[:, 1], label="Re[psi]", color="#1fb45d96", lw=1.0)
    line_im, = ax_psi.plot(x, first_data[:, 2], label="Im[psi]", color="#0e9fff81", lw=1.0)
    line_prob, = ax_psi.plot(x, first_data[:, 3], label="Probability density", color="#a02c2c", lw=1.0)
    
    ax_psi.set_xlabel("x")
    ax_psi.set_ylabel("psi")
    
    # Force 1/3 zero alignment: Y_min = -0.5 * Y_max
    y_limits = ax_psi.get_ylim()
    y_max_data = max(abs(y_limits[0]), abs(y_limits[1]))
    Y_max = y_max_data * 1.4 #! This can be used to add some headroom to the plot.
    ax_psi.set_ylim(-0.5 * Y_max, Y_max)

    # --- Right Axis: Potential (Symlog) ---
    env_data = np.atleast_2d(np.loadtxt(Path(output_dir) / "enviroment_potential.txt"))
    env_v = np.interp(x, env_data[:, 0], env_data[:, 1])
    
    ax_v = ax_psi.twinx()
    
    # Set linthresh=1.0 so log scale starts exactly at 1e0, 1e1, etc.
    ax_v.set_yscale("symlog", linthresh=1.0)
    ax_v.fill_between(x, 0, env_v, color="gray", alpha=0.3, label="Environment potential", zorder=0)
    ax_v.set_ylabel("V(x) [Symlog]")

    # 3. Calculate Symlog limits to force the 0-line to exactly 33.33% height
    v_max = np.max(env_v)
    V_top = v_max * 10 if v_max > 0 else 10.0  # Upper bound (log headroom)
    
    # A symlog(linthresh=1) axis consists of 1 linear segment (0 to 1) plus log segments (1 to V_top)
    pos_visual_segments = 1.0 + np.log10(V_top)
    
    # We want exactly half as many visual segments below 0 to achieve the 1:2 screen ratio
    neg_visual_segments = 0.5 * pos_visual_segments
    
    # Convert visual segments back to the required numerical lower bound
    if neg_visual_segments <= 1.0:
        V_bot = neg_visual_segments  # Falls within the linear region (0 to -1)
    else:
        V_bot = 10 ** (neg_visual_segments - 1.0)  # Falls within the log region
        
    ax_v.set_ylim(-V_bot, V_top)

    # --- Parameter Text Box ---
    if param_text:
        # Place a text box outside the plot area on the right side
        fig.subplots_adjust(right=0.85) # Make room on the right
        fig.text(0.87, 0.5, param_text, fontsize=10, family='monospace',
                 verticalalignment='center',
                 bbox=dict(boxstyle='round', facecolor='white', alpha=0.8, edgecolor='gray'))

    # --- Legend ---
    lines_psi, labels_psi = ax_psi.get_legend_handles_labels()
    lines_v, labels_v = ax_v.get_legend_handles_labels()
    ax_psi.legend(lines_psi + lines_v, labels_psi + labels_v, loc="upper center")

    # --- Animation ---
    def update(frame_idx):
        probability_density, data = load_frame(files[frame_idx])
        line_re.set_ydata(data[:, 1])
        line_im.set_ydata(data[:, 2])
        line_prob.set_ydata(data[:, 3])
        
        frame_label = f"current time = {get_frame_time(files[frame_idx]):.6f} | P = {probability_density:.6f}"
        ax_psi.set_title(f"{title} | {frame_label}" if title else frame_label)
        return line_re, line_im, line_prob

    interval_ms = 1000.0 / fps
    anim = FuncAnimation(fig, update, frames=len(files), interval=interval_ms, blit=False)

    out_path = Path(output_mp4)
    if not out_path.suffix:
        out_path = out_path.with_suffix(".mp4")

    out_path.parent.mkdir(parents=True, exist_ok=True)

    anim.save(out_path, fps=fps, dpi=DPI)
    plt.close(fig)

if __name__ == "__main__":
    # Default values matched to project structure
    base_dir = Path(__file__).resolve().parent
    
    animate_wavefunction(
        output_dir=base_dir / "output",
        output_mp4=base_dir / "results" / "wavefunction.mp4",
        fps=30.0, # Equivalent to original 1000/50
        title="ESC 202 Simulation"
    )