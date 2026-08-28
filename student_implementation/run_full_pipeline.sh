#!/usr/bin/env bash
# run_full_pipeline.sh — Week 3 "integrated prototype" deliverable
# (brief §4: "Integration, experimentation, optimisation" -> "Integrated
# prototype"). Runs the ENTIRE project end-to-end from a clean checkout,
# across all three week folders, as one command — proof the pieces built
# separately by four people across three weeks actually function as one
# system, not four disconnected parts.
#
# Project 1: Roofline Reckoning, CPEN 438, Group 3
# Run from WSL2 Ubuntu (needs perf_event_open, taskset, gcc, python3):
#   bash week3/integration/run_full_pipeline.sh
set -euo pipefail
cd "$(dirname "$0")/../.."   # -> gh-bench/

echo "=========================================================="
echo " STAGE 1/6 (week1): regenerate seeded datasets"
echo "=========================================================="
cd week1/datasets
python3 gen_momo_log.py --n 20000 --seed 829488 --match-rate 0.85 --out ../../week2/datasets/momo_log.csv
python3 gen_cocobod_data.py --n 20000 --seed 829488 --out ../../week2/datasets/cocobod_data.csv
python3 gen_rainfall_grid.py --rows 200 --cols 200 --seed 829488 --missing-rate 0.12 --out ../../week2/datasets/rainfall_grid.csv
python3 gen_momo_log.py --n 1000000 --seed 829488 --match-rate 0.85 --out ../../week2/datasets/momo_log_scaling.csv
cd ../..

echo "=========================================================="
echo " STAGE 2/6 (week1): run the Level-1 proof-of-concept demo"
echo "=========================================================="
cd week1/instructor_demo/c
gcc -O2 -Wall -Wextra -std=c11 demo_roofline.c -o demo_roofline
./demo_roofline 2000000 5 829488 > /tmp/demo_check.jsonl
echo "demo produced $(wc -l < /tmp/demo_check.jsonl) records (not appended to the tracked log -- that's frozen evidence from Week 1)"
rm -f demo_roofline
cd ../../..

echo "=========================================================="
echo " STAGE 3/6 (week2): build gh_bench and run the 20 unit tests"
echo "=========================================================="
bash week2/tests/run_tests.sh
rm -f week2/tests/test_kernels

echo "=========================================================="
echo " STAGE 4/6 (week2): run the full 3x2x6 experiment matrix"
echo "=========================================================="
bash week2/scripts/run_experiment_matrix.sh
rm -f week2/student_implementation/gh_bench

echo "=========================================================="
echo " STAGE 5/6 (week2): run the OpenMP scaling extension"
echo "=========================================================="
bash week2/scripts/run_omp_scaling.sh
rm -f week2/scripts/omp_probe_scaling

echo "=========================================================="
echo " STAGE 6/6 (week3): run the Week 3 innovation evaluation"
echo "=========================================================="
bash week3/innovation/evaluate_innovation.sh
rm -f week3/innovation/adaptive_momo_match

echo "=========================================================="
echo " PIPELINE COMPLETE."
echo " Remaining steps (Python/MATLAB, run separately -- see"
echo " week2/README.md and week3/README.md):"
echo "   cd week2/student_implementation && python analyze.py ... && python plots.py"
echo "   matlab -batch \"cd('week2/student_implementation'); roofline_model\""
echo "=========================================================="
