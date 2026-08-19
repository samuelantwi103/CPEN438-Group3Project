#!/usr/bin/env python3
"""
analyze.py — turns raw gh_bench / peak_flops / stream_triad JSONL output
into a per-kernel summary CSV (Project 1: Roofline Reckoning).

Owner: Obed Ninson (Python/MATLAB + Test/Docs Lead)

Input formats (schemas fixed by the C authors, not chosen here):
  - kernel results: one JSON object per line, as printed by gh_bench.c's
    print_result() — fields: kernel, n, run, warmup, wall_seconds,
    user_seconds, sys_seconds, instructions, cycles, cache_refs,
    cache_misses, counters_valid, flops, bytes_moved, phase1_seconds,
    phase2_seconds, gflops, arithmetic_intensity.
  - peak_flops.c / stream_triad.c: one JSON object total (single line),
    fields {"gflops": ...} and {"bandwidth_gbs": ...} respectively.

KNOWN GAP (flagging, not silently working around): run_experiment_matrix.sh
invokes gh_bench with a --config-id flag, but gh_bench.c's actual argv
parsing is positional (kernel, path, reps, seed) and has no such flag, and
print_result() never emits a config/core-count field. So a results.jsonl
produced by that script cannot be split back into configA_2core vs
configB_8core from its own content. Until that's reconciled, this script
lets the caller supply --config-id to *label* one results file at a time
(run gh_bench once per config, analyze each file separately), rather than
pretending the field exists in the data.

Usage:
    python analyze.py --results results/raw/results_configA.jsonl \
        --config-id configA_2core \
        --peak-flops results/raw/peak_flops.jsonl \
        --stream-triad results/raw/stream_triad.jsonl \
        --out results/summary.csv
    # repeat per config, appending to the same --out file with --append
"""
import argparse
import csv
import json
import os
import statistics as stats


def load_jsonl(path):
    rows = []
    with open(path) as f:
        for line in f:
            line = line.strip()
            if not line:
                continue
            rows.append(json.loads(line))
    return rows


def load_single_json(path):
    with open(path) as f:
        for line in f:
            line = line.strip()
            if line:
                return json.loads(line)
    raise ValueError(f"{path}: no JSON object found")


def median(values):
    values = [v for v in values if v is not None]
    return stats.median(values) if values else 0.0


def summarize_kernel(rows):
    """rows: all (non-warmup) result lines for one kernel. Returns a dict
    of summary stats. Medians are used instead of means because timing
    distributions from a shared, non-isolated machine are right-skewed
    (occasional OS-scheduling stalls), and the project brief's own
    methodology section (§L) calls for the median-of-N-reps convention."""
    n = rows[0]["n"]
    wall = median([r["wall_seconds"] for r in rows])
    gflops = median([r["gflops"] for r in rows])
    ai = median([r["arithmetic_intensity"] for r in rows])
    counters_ok = all(r.get("counters_valid") for r in rows)

    ipc = 0.0
    miss_rate = 0.0
    if counters_ok:
        cycles = median([r["cycles"] for r in rows])
        instr = median([r["instructions"] for r in rows])
        refs = median([r["cache_refs"] for r in rows])
        misses = median([r["cache_misses"] for r in rows])
        ipc = (instr / cycles) if cycles else 0.0
        miss_rate = (misses / refs) if refs else 0.0

    p1 = median([r["phase1_seconds"] for r in rows])
    p2 = median([r["phase2_seconds"] for r in rows])
    serial_fraction = (p1 / (p1 + p2)) if (p1 + p2) > 0 else None

    return {
        "n": n,
        "n_runs": len(rows),
        "median_wall_seconds": wall,
        "median_gflops": gflops,
        "arithmetic_intensity": ai,
        "counters_valid": counters_ok,
        "ipc": ipc,
        "cache_miss_rate": miss_rate,
        "amdahl_serial_fraction": serial_fraction,
    }


def roofline_classify(ai, peak_gflops, peak_bandwidth_gbs):
    """Attainable GFLOP/s per the Roofline model: min(peak compute,
    AI * peak bandwidth). Ridge point is where the two roofs cross."""
    ridge_ai = peak_gflops / peak_bandwidth_gbs if peak_bandwidth_gbs else float("inf")
    attainable = min(peak_gflops, ai * peak_bandwidth_gbs)
    region = "memory-bound" if ai < ridge_ai else "compute-bound"
    return attainable, region, ridge_ai


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--results", required=True, help="gh_bench results JSONL for ONE config")
    ap.add_argument("--config-id", required=True,
                     help="label for this run (e.g. configA_2core) — see KNOWN GAP in the docstring")
    ap.add_argument("--peak-flops", required=True, help="peak_flops.c single-line JSON output")
    ap.add_argument("--stream-triad", required=True, help="stream_triad.c single-line JSON output")
    ap.add_argument("--out", required=True)
    ap.add_argument("--append", action="store_true",
                     help="append to --out instead of overwriting (for multi-config runs)")
    args = ap.parse_args()

    rows = load_jsonl(args.results)
    if not rows:
        raise SystemExit(f"{args.results}: no records found")

    peak = load_single_json(args.peak_flops)
    triad = load_single_json(args.stream_triad)
    peak_gflops = peak["gflops"]
    peak_bandwidth = triad["bandwidth_gbs"]

    by_kernel = {}
    for r in rows:
        if r.get("warmup"):
            continue
        by_kernel.setdefault(r["kernel"], []).append(r)

    out_rows = []
    for kernel, krows in sorted(by_kernel.items()):
        summ = summarize_kernel(krows)
        attainable, region, ridge_ai = roofline_classify(
            summ["arithmetic_intensity"], peak_gflops, peak_bandwidth)
        pct_of_roofline = (summ["median_gflops"] / attainable * 100.0) if attainable > 0 else 0.0
        out_rows.append({
            "config_id": args.config_id,
            "kernel": kernel,
            "n": summ["n"],
            "n_runs": summ["n_runs"],
            "median_wall_seconds": f"{summ['median_wall_seconds']:.9f}",
            "median_gflops": f"{summ['median_gflops']:.6f}",
            "arithmetic_intensity": f"{summ['arithmetic_intensity']:.6f}",
            "counters_valid": summ["counters_valid"],
            "ipc": f"{summ['ipc']:.4f}",
            "cache_miss_rate": f"{summ['cache_miss_rate']:.6f}",
            "amdahl_serial_fraction": (
                f"{summ['amdahl_serial_fraction']:.6f}"
                if summ["amdahl_serial_fraction"] is not None else ""
            ),
            "ridge_point_ai": f"{ridge_ai:.6f}",
            "attainable_gflops": f"{attainable:.6f}",
            "pct_of_roofline": f"{pct_of_roofline:.2f}",
            "region": region,
        })

    fieldnames = list(out_rows[0].keys())
    mode = "a" if (args.append and os.path.exists(args.out)) else "w"
    write_header = not (mode == "a")
    os.makedirs(os.path.dirname(args.out) or ".", exist_ok=True)
    with open(args.out, mode, newline="") as f:
        w = csv.DictWriter(f, fieldnames=fieldnames)
        if write_header:
            w.writeheader()
        w.writerows(out_rows)

    print(f"[{args.config_id}] peak_gflops={peak_gflops:.3f} "
          f"peak_bandwidth_gbs={peak_bandwidth:.3f}")
    for row in out_rows:
        print(f"  {row['kernel']:28s} AI={row['arithmetic_intensity']:>10s} "
              f"GFLOP/s={row['median_gflops']:>10s} "
              f"({row['pct_of_roofline']}% of roofline, {row['region']})")
    print(f"wrote {len(out_rows)} rows to {args.out} ({'appended' if mode == 'a' else 'new file'})")


if __name__ == "__main__":
    main()
