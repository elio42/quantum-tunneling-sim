# NAND-Flash_simulation_ESC202



## Building and dependencies

**Dependencies:**

- `fftw3`

**Build:** (from `(...)/1D-SSFM`)

- `cmake -S src -B build`
- `cmake --build build`
- `./build/SSFM1D`

suggestion for parameters

- "Nice looking":
  - `./build/SSFM1D -N 4096 -L 20 -s 0.5 -w 120 -t 2 -i 0.0025 --tunneling-pot 700 && python3 animate_wavefunction.py`
- Realistic values:
  - `./build/SSFM1D -N 4096 -L 1935 -s 48 -w 138 -t 20000 -i 20 --tunneling-pot 0.114 -b 10 && python3 animate_wavefunction.py`
  - 