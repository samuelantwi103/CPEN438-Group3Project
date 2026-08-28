# Week 3 — Kumi Kelvin Gyabaah (C/C++ Implementation & Performance Lead)

Designed and implemented the Week 3 innovation component — one original enhancement to GH-Bench.

## Files in this folder

| File | Location | What it is |
|---|---|---|
| `adaptive_momo_match.c` | `student_implementation/src/adaptive_momo_match.c` | A self-calibrating adaptive kernel: measures its own OpenMP thread-launch overhead and per-transaction cost at runtime, then decides whether to probe transactions sequentially or in parallel |

## Build & run it standalone (WSL2 Ubuntu)

```bash
gcc -O2 -fopenmp -Wall -Wextra -std=c11 adaptive_momo_match.c -lm -o adaptive_momo_match
./adaptive_momo_match /path/to/a/momo_log.csv
```
(For the full sweep across problem sizes, see `evaluate_innovation.sh`.)

## Individual Contribution Form

- **Name & role:** Kumi Kelvin Gyabaah — C/C++ Implementation Lead
- **Specific artefacts I personally authored:** _______________________
- **Specific tests I personally ran:** _______________________
- **Specific results I personally interpreted:** _______________________
- **Git commit hashes attributable to me:** _______________________
- **Report section(s) I personally wrote:** _______________________
- **Live defence component I am prepared to present/defend:** _______________________
- **Estimated % of total team effort (self-assessed):** _______________________
