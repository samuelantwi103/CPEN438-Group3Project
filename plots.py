#!/usr/bin/env python3
"""
plots.py — Roofline and Amdahl's-Law plots from analyze.py's summary CSV
(Project 1: Roofline Reckoning).

Owner: Obed Ninson (Python/MATLAB + Test/Docs Lead)

Consumes results/summary.csv as written by analyze.py (columns: config_id,
kernel, arithmetic_intensity, median_gflops, attainable_gflops,
ridge_point_ai, pct_of_roofline, region, amdahl_serial_fraction, ...) plus
the same peak_flops.c / stream_triad.c single-line JSON files analyze.py
reads, so the roofline "roof" lines are drawn from the same measured
numbers the points are classified against (not re-guessed here).

Usage:
    python plots.py --summary results/summary.csv \
        --peak-flops results/raw/peak_flops.jsonl \
        --stream-triad results/raw/stream_triad.jsonl \
        --out-dir results/plots
"""
import argparse
import csv
import json
import os

import matplotlib
matplotlib.use("Agg")  # headless — WSL2 dev box has no display server
import matplotlib.pyplot as plt


def load_summary(path):
    with open(path, newline="") as f:
        return list(csv.DictReader(f))


def load_single_json(path):
    with open(path) as f:
        for line in f:
            line = line.strip()
            if line:
                return json.loads(line)
    raise ValueError(f"{path}: no JSON object found")


def plot_roofline(rows, peak_gflops, peak_bandwidth_gbs, out_path):
    fig, ax = plt.subplots(figsize=(7, 5))

    ai_min = min(0.01, min(float(r["arithmetic_intensity"]) for r in rows) / 2)
    ai_max = max(peak_gflops / peak_bandwidth_gbs * 10,
                 max(float(r["arithmetic_intensity"]) for r in rows) * 2)

    # Bandwidth-bound roof: gflops = AI * peak_bandwidth, up to the ridge.
    ridge_ai = peak_gflops / peak_bandwidth_gbs
    bw_x = [ai_min, ridge_ai]
    bw_y = [ai_min * peak_bandwidth_gbs, peak_gflops]
    ax.plot(bw_x, bw_y, "-", color="#1f77b4", label=f"memory roof ({peak_bandwidth_gbs:.1f} GB/s)")

    # Compute roof: flat line at peak_gflops from the ridge point onward.
    ax.plot([ridge_ai, ai_max], [peak_gflops, peak_gflops], "-", color="#d62728",
             label=f"compute roof ({peak_gflops:.1f} GFLOP/s)")

    markers = {"momo_match": "o", "cocobod_yield_regression": "s", "rainfall_interpolate": "^"}
    colors = {"configA_2core": "#2ca02c", "configB_8core": "#9467bd"}
    seen_labels = set()
    for r in rows:
        ai = float(r["arithmetic_intensity"])
        gflops = float(r["median_gflops"])
        kernel = r["kernel"]
        cfg = r["config_id"]
        label = f"{kernel} @ {cfg}"
        marker = markers.get(kernel, "x")
        color = colors.get(cfg, "#333333")
        ax.scatter([ai], [gflops], marker=marker, color=color,
                   s=70, zorder=5, label=None if label in seen_labels else label)
        seen_labels.add(label)

    ax.set_xscale("log")
    ax.set_yscale("log")
    ax.set_xlabel("Arithmetic Intensity (FLOP/byte)")
    ax.set_ylabel("Performance (GFLOP/s)")
    ax.set_title("Roofline — GH-Bench kernels")
    ax.axvline(ridge_ai, color="gray", linestyle=":", linewidth=1)
    ax.legend(fontsize=8, loc="lower right")
    ax.grid(True, which="both", linestyle="--", alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def amdahl_speedup(serial_fraction, p):
    if serial_fraction is None:
        return None
    return 1.0 / (serial_fraction + (1.0 - serial_fraction) / p)


def plot_amdahl(rows, out_path, max_cores=8):
    # One curve per (kernel, config_id) that has a serial-fraction estimate.
    fig, ax = plt.subplots(figsize=(7, 5))
    core_range = list(range(1, max_cores + 1))
    plotted = False
    for r in rows:
        sf = r.get("amdahl_serial_fraction", "")
        if sf == "":
            continue
        sf = float(sf)
        speedups = [amdahl_speedup(sf, p) for p in core_range]
        ax.plot(core_range, speedups, marker="o", markersize=3,
                 label=f"{r['kernel']} @ {r['config_id']} (f={sf:.3f})")
        plotted = True

    if not plotted:
        ax.text(0.5, 0.5, "no kernel reported phase1/phase2 timing\n"
                            "(amdahl_serial_fraction empty for all rows)",
                ha="center", va="center", transform=ax.transAxes)
    else:
        ax.plot(core_range, core_range, "--", color="gray", linewidth=1, label="ideal (linear)")

    ax.set_xlabel("Cores (p)")
    ax.set_ylabel("Predicted speedup")
    ax.set_title("Amdahl's Law — predicted speedup by serial fraction")
    ax.legend(fontsize=8)
    ax.grid(True, linestyle="--", alpha=0.3)
    fig.tight_layout()
    fig.savefig(out_path, dpi=150)
    plt.close(fig)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--summary", required=True)
    ap.add_argument("--peak-flops", required=True)
    ap.add_argument("--stream-triad", required=True)
    ap.add_argument("--out-dir", required=True)
    args = ap.parse_args()

    rows = load_summary(args.summary)
    if not rows:
        raise SystemExit(f"{args.summary}: no rows")

    peak = load_single_json(args.peak_flops)
    triad = load_single_json(args.stream_triad)

    os.makedirs(args.out_dir, exist_ok=True)
    roofline_path = os.path.join(args.out_dir, "roofline.png")
    amdahl_path = os.path.join(args.out_dir, "amdahl.png")

    plot_roofline(rows, peak["gflops"], triad["bandwidth_gbs"], roofline_path)
    plot_amdahl(rows, amdahl_path)

    print(f"wrote {roofline_path}")
    print(f"wrote {amdahl_path}")


if __name__ == "__main__":
    main()
