#!/usr/bin/env bash
# run_tests.sh — compile and run the kernel correctness unit tests.
# Project 1: Roofline Reckoning, CPEN 438, Group 3
# Owner: Kumi Kelvin Gyabaah (C/C++ Implementation Lead)
#
# Usage: ./tests/run_tests.sh   (from anywhere; paths are script-relative)
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"
BIN="$SCRIPT_DIR/test_kernels_bin"

gcc -O0 -g -Wall -Wextra -std=c11 \
    "$SCRIPT_DIR/test_kernels.c" "$REPO_ROOT/kernels.c" \
    -I"$REPO_ROOT" -lm -o "$BIN"

"$BIN"
status=$?

rm -f "$BIN"
exit $status
