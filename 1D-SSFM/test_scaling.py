#!/usr/bin/env python3
import subprocess
import matplotlib.pyplot as plt
import numpy as np
import sys


EXEC = "build/SSFM1D"  # adjust if needed
REPEAT = 2


def run_time(exec_path, args_list):
    cmd = [exec_path, "--time-code"] + args_list
    p = subprocess.run(cmd, capture_output=True, text=True, check=True)
    if p.stderr:
        print(p.stderr, end='', file=sys.stdout)
    return float(p.stdout.strip())


def params_to_args(params):
    args = []
    for k, v in params.items():
        args.append(k)
        if v is not None:
            args.append(str(v))
    return args


def test_N_scaling():
    base = {"--tunneling-wall-disabled": None, "--delta-time": 0.8, "--total-time": 0.1}
    Ns = [256, 512, 1024, 2048, 4096, 8192]
    times = []
    for N in Ns:
        p = base.copy()
        p["--number-of-points"] = N
        p["--length"] = 3 * (N / 1024)  # Scale L with N
        p["--padding"] = int(50 * (N / 1024))
        vals = []
        for _ in range(REPEAT):
            vals.append(run_time(EXEC, params_to_args(p)))
            print(f"N={N}, run time: {vals[-1]:.2f} ms")
        times.append(sum(vals) / len(vals))
    return Ns, times


def test_b_scaling():
    base = {"--length": 3, "--tunneling-wall-disabled": None, "--delta-time": 0.8, "--total-time": 0.1}
    bvals = [1e2, 1e3, 1e4, 1e5, 1e6, 1e7, 1e8]
    times = []
    for b in bvals:
        p = base.copy()
        p["--boundary-potential"] = b
        vals = []
        for _ in range(REPEAT):
            vals.append(run_time(EXEC, params_to_args(p)))
            print(f"Boundary potential={b}, run time: {vals[-1]:.2f} ms")
        times.append(sum(vals) / len(vals))
    return bvals, times


def main():
    Ns, tN = test_N_scaling()
    bvals, tB = test_b_scaling()

    plt.style.use("seaborn-v0_8-darkgrid")
    fig, ax = plt.subplots(1, 2, figsize=(11, 5))

    Ns_arr = np.array(Ns)
    tN_arr = np.array(tN)
    ax[0].loglog(Ns_arr, tN_arr, marker='o', markersize=6, label='measured', color='#2E86AB', lw=1.5)

    # reference N log N line scaled to median of measured data
    ref_N = Ns_arr * np.log(Ns_arr)
    scaleN = np.median(tN_arr) / np.median(ref_N)
    ax[0].loglog(Ns_arr, ref_N * scaleN, ls='--', color='#A23B72', label=r'$N\log N$ (ref)')

    ax[0].set_xlabel("N (number of grid points)")
    ax[0].set_ylabel("Time (ms)")
    ax[0].set_title("Scaling with N")
    ax[0].legend(frameon=True, fontsize=9)

    b_arr = np.array(bvals)
    tB_arr = np.array(tB)
    ax[1].loglog(b_arr, tB_arr, marker='s', markersize=6, label='measured', color='#2E86AB', lw=1.5)

    # reference linear scaling (V^1) in potential-limited regime
    ref_b = b_arr
    linear_idx = b_arr >= 1e6
    scaleb = np.median(tB_arr[linear_idx]) / np.median(ref_b[linear_idx])
    ax[1].loglog(b_arr, ref_b * scaleb, ls='--', color='#A23B72', label=r'$V$ (ref)')

    ax[1].set_xlabel("Boundary potential")
    ax[1].set_ylabel("Time (ms)")
    ax[1].set_title("Scaling with boundary potential")
    ax[1].legend(frameon=True, fontsize=9)

    for a_sub in ax:
        a_sub.grid(True, which='both', alpha=0.3)

    plt.tight_layout()
    plt.savefig("time_scaling.png", dpi=200)
    print("Saved time_scaling.png")


if __name__ == "__main__":
    main()
