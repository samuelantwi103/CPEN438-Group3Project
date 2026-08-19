#!/usr/bin/env python3
"""
gen_rainfall_grid.py — seeded synthetic Volta-basin rainfall raster
generator for the rainfall_interpolate kernel.

Owner: Obed Ninson (Python/MATLAB + Test/Docs Lead)

Generates a smooth spatial rainfall field (sum of a few low-frequency 2D
sinusoids, so nearby cells are correlated — realistic for interpolation)
over a rows x cols grid, then knocks out `missing_rate` of cells to
GH_RAIN_MISSING (-9999.0), simulating gaps in a real gauge/satellite
rainfall product.

Output format (matches gh_bench.c's load_rainfall parser):
    line 1: "rows,cols"
    remaining: rows*cols comma-separated doubles, row-major

Usage:
    python gen_rainfall_grid.py --rows 200 --cols 200 --seed 30153 \
        --missing-rate 0.12 --out ../datasets/rainfall_grid.csv
"""
import argparse
import math
import random

MISSING = -9999.0


def generate(rows: int, cols: int, seed: int, missing_rate: float):
    rng = random.Random(seed)
    # Seed-derived spatial-frequency and phase parameters -> each team's
    # field has a genuinely different (but internally consistent) shape.
    freqs = [(rng.uniform(1, 4), rng.uniform(1, 4), rng.uniform(0, 2 * math.pi))
              for _ in range(3)]
    amps = [rng.uniform(20, 60) for _ in range(3)]
    base = rng.uniform(80, 150)

    grid = [[0.0] * cols for _ in range(rows)]
    for r in range(rows):
        for c in range(cols):
            u, v = r / rows, c / cols
            val = base
            for (fu, fv, phase), amp in zip(freqs, amps):
                val += amp * math.sin(2 * math.pi * fu * u + 2 * math.pi * fv * v + phase)
            val += rng.gauss(0, 3.0)
            grid[r][c] = max(0.0, val)

    n_missing = int(rows * cols * missing_rate)
    all_cells = [(r, c) for r in range(rows) for c in range(cols)]
    rng.shuffle(all_cells)
    for (r, c) in all_cells[:n_missing]:
        grid[r][c] = MISSING

    return grid


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--rows", type=int, default=200)
    ap.add_argument("--cols", type=int, default=200)
    ap.add_argument("--seed", type=int, required=True)
    ap.add_argument("--missing-rate", type=float, default=0.12)
    ap.add_argument("--out", type=str, required=True)
    args = ap.parse_args()
    if not 0.0 <= args.missing_rate < 1.0:
        ap.error("--missing-rate must be in [0.0, 1.0)")
    if args.rows <= 0 or args.cols <= 0:
        ap.error("--rows and --cols must be positive integers")

    grid = generate(args.rows, args.cols, args.seed, args.missing_rate)
    with open(args.out, "w") as f:
        f.write(f"{args.rows},{args.cols}\n")
        for row in grid:
            f.write(",".join(f"{v:.6f}" for v in row) + ",\n")
    n_missing = sum(1 for row in grid for v in row if v == MISSING)
    print(f"wrote {args.rows}x{args.cols} grid to {args.out} "
          f"({n_missing} missing cells, seed={args.seed})")


if __name__ == "__main__":
    main()
