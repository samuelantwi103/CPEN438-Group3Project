#!/usr/bin/env python3
"""
gen_cocobod_data.py — seeded synthetic COCOBOD regional yield/rainfall/
fertiliser dataset generator for the cocobod_yield_regression kernel.

Owner: Obed Ninson (Python/MATLAB + Test/Docs Lead)

Models a linear relationship yield = b0 + b1*rainfall + b2*fertiliser + noise
with team-specific true coefficients derived from the seed, so each team's
regression fit (and R^2) is genuinely distinct while still recoverable,
this is the ground truth the correctness unit tests check the kernel
against (see tests/test_cocobod_correctness.py).

Output CSV columns: rainfall_mm,fertiliser_kg_ha,yield_tonnes_ha

Usage:
    python gen_cocobod_data.py --n 20000 --seed 30153 \
        --out ../datasets/cocobod_data.csv
"""
import argparse
import csv
import random


def true_coefficients(seed: int):
    """Deterministic, seed-derived 'ground truth' coefficients so  that every
    team's dataset has a different but reproducible regression target."""
    rng = random.Random(seed ^ 0xC0C0BAD)
    b0 = round(rng.uniform(0.3, 0.9), 4)      # base yield, t/ha
    b1 = round(rng.uniform(0.0008, 0.0025), 6)  # t/ha per mm rainfall
    b2 = round(rng.uniform(0.002, 0.006), 6)    # t/ha per kg/ha fertiliser
    return b0, b1, b2


def generate(n: int, seed: int):
    rng = random.Random(seed)
    b0, b1, b2 = true_coefficients(seed)
    rows = []
    for _ in range(n):
        rainfall = round(rng.uniform(800.0, 2200.0), 2)   # mm/year, Ghana cocoa belt range
        fert = round(rng.uniform(0.0, 400.0), 2)           # kg/ha
        noise = rng.gauss(0.0, 0.08)
        yield_ha = max(0.0, b0 + b1 * rainfall + b2 * fert + noise)
        rows.append((rainfall, fert, round(yield_ha, 4)))
    return rows, (b0, b1, b2)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--n", type=int, default=20000)
    ap.add_argument("--seed", type=int, required=True)
    ap.add_argument("--out", type=str, required=True)
    args = ap.parse_args()
    if args.n <= 0:
        ap.error("--n must be a positive integer")

    rows, truth = generate(args.n, args.seed)
    with open(args.out, "w", newline="") as f:
        w = csv.writer(f)
        w.writerow(["rainfall_mm", "fertiliser_kg_ha", "yield_tonnes_ha"])
        w.writerows(rows)
    print(f"wrote {len(rows)} records to {args.out} (seed={args.seed})")
    print(f"ground-truth coefficients: b0={truth[0]} b1={truth[1]} b2={truth[2]}")
    print("(kernel's fitted b0/b1/b2 should recover these within noise — "
          "used by tests/test_cocobod_correctness.py)")


if __name__ == "__main__":
    main()
