#!/usr/bin/env bash
# run_experiment_matrix.sh — builds gh_bench and runs the full
# kernel x configuration experiment matrix (Project 1 brief, section L).
# Owner: Amponsah Jonathan Boadu (Hardware/Perf Lead)
#
# Run from WSL (needs perf_event_open + taskset, both Linux-only):
#   wsl -d Ubuntu -- bash /mnt/c/.../gh-bench/scripts/run_experiment_matrix.sh
set -euo pipefail
cd "$(dirname "$0")/.."

echo "--- building gh_bench ---"
gcc -O2 -Wall -Wextra -std=c11 -Istudent_implementation/include \
    student_implementation/src/harness.c \
    student_implementation/src/kernels.c \
    student_implementation/src/gh_bench.c \
    -lm -o student_implementation/gh_bench

RESULTS=results/raw/results.jsonl
SEED=829488
mkdir -p results/raw
rm -f "$RESULTS"

declare -A CONFIGS=( [configA_2core]="0-1" [configB_8core]="0-7" )
declare -A INPUTS=( [momo]="datasets/momo_log.csv" [cocobod]="datasets/cocobod_data.csv" [rainfall]="datasets/rainfall_grid.csv" )

for cfg_id in "${!CONFIGS[@]}"; do
  cpus="${CONFIGS[$cfg_id]}"
  for kernel in momo cocobod rainfall; do
    inp="${INPUTS[$kernel]}"
    echo "--- $kernel @ $cfg_id (cpus $cpus) ---"
    taskset -c "$cpus" ./student_implementation/gh_bench \
      --kernel "$kernel" --input "$inp" --config-id "$cfg_id" \
      --reps 5 --warmup 1 --seed "$SEED" --out "$RESULTS"
  done
done

echo "--- done: $(wc -l < "$RESULTS") records written to $RESULTS ---"
