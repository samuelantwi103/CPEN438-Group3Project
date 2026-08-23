#!/usr/bin/env bash
# evaluate_innovation.sh — sweeps problem size and compares the adaptive
# kernel against both fixed baselines (always-sequential, always-parallel).
# Project 1: Roofline Reckoning, CPEN 315/733, Group 3, Week 3
#
# Owner: Obed Ninson (Python/MATLAB + Test/Docs Lead)
#
# Path note: this repo is currently flat (datasets/, gh_bench.c, harness.c
# etc. all sit at repo root — see run_full_pipeline.sh's own docstring for
# the still-open question of whether a week1/week2/week3 top-level reorg
# is coming). Paths below match the ACTUAL current layout, not the
# reorganised one run_full_pipeline.sh assumes; update both scripts
# together if/when the team does that reorg.
set -euo pipefail
cd "$(dirname "$0")"          # -> week3/innovation/
REPO_ROOT="$(cd ../.. && pwd)"

echo "--- building adaptive_momo_match ---"
gcc -O2 -fopenmp -Wall -Wextra -std=c11 adaptive_momo_match.c -lm -o adaptive_momo_match

OUT="$REPO_ROOT/results/innovation_results.jsonl"
mkdir -p "$(dirname "$OUT")"
: > "$OUT"
DATA_DIR=/tmp/gh_bench_innovation_data
mkdir -p "$DATA_DIR"
SEED=829488

for n in 1000 5000 20000 100000 500000 1000000; do
  csv="$DATA_DIR/momo_${n}.csv"
  python3 "$REPO_ROOT/datasets/gen_momo_log.py" --n "$n" --seed "$SEED" --match-rate 0.85 --out "$csv" >/dev/null
  echo "--- n=$n ---"
  taskset -c 0-7 ./adaptive_momo_match "$csv" >> "$OUT"
done

echo "--- done: $(wc -l < "$OUT") records written to $OUT ---"
cat "$OUT"
