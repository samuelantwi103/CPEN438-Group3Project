# Week 3 — Kumi Kelvin Gyabaah (C/C++ Implementation & Performance Lead)

## Your Week 3 role
Design and implement the Week 3 innovation component — one original enhancement to GH-Bench, per the brief's Creativity Challenge (§N).

## Files in this folder

| File | Canonical location | What it is |
|---|---|---|
| `adaptive_momo_match.c` | `gh-bench/week3/innovation/adaptive_momo_match.c` | A self-calibrating adaptive kernel: measures its own OpenMP thread-launch overhead and per-transaction cost at runtime, then decides whether to probe transactions sequentially or in parallel |

## Build & run it standalone (WSL2 Ubuntu)

```bash
gcc -O2 -fopenmp -Wall -Wextra -std=c11 adaptive_momo_match.c -lm -o adaptive_momo_match
./adaptive_momo_match /path/to/a/momo_log.csv
```
(For the full sweep across problem sizes, see Obed's `evaluate_innovation.sh`.)

## What you personally need to verify before the demo

- [ ] **Read the file's own header comment carefully** and be ready to explain, unaided, exactly what the "self-calibrating" part means — specifically, why timing a no-op `#pragma omp parallel` region tells you anything useful (it measures thread-launch overhead on whatever machine it's run on, not a value baked into the code).
- [ ] Trace through by hand what happens for a tiny n (say n=500, smaller than `SAMPLE_SIZE`): confirm the code doesn't try to process a "remaining" chunk that doesn't exist. This is exactly the kind of edge case a defence question would probe.
- [ ] Confirm you understand why this reuses the exact hash-table-build logic from `week2/student_implementation/src/kernels.c`'s `momo_match` rather than something different — consistency with the original kernel matters for the comparison to mean anything.
- [ ] Be ready for: "add a fourth strategy live" (e.g., always-use-4-threads) — could you extend the dispatch logic on the spot?

## Individual Contribution Form (fill in before submission)

- **Name & role:** Kumi Kelvin Gyabaah — C/C++ Implementation Lead
- **Specific artefacts I personally authored:** _______________________
- **Specific tests I personally ran:** _______________________
- **Specific results I personally interpreted:** _______________________
- **Git commit hashes attributable to me:** _______________________
- **Report section(s) I personally wrote:** _______________________
- **Live defence component I am prepared to present/defend:** _______________________
- **Estimated % of total team effort (self-assessed):** _______________________
