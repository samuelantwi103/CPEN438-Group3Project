# Individual Contribution Statement — Kumi Kelvin Gyabaah

Project 1: Roofline Reckoning, CPEN 315/733, Group 3. Role: C/C++ Implementation & Performance Lead.

| Field | Content |
|---|---|
| Name & role | Kumi Kelvin Gyabaah — C/C++ Implementation & Performance Lead |
| Specific code/hardware artefacts personally authored | `student_implementation/src/kernels.c`/`kernels.h` (all three kernels: `momo_match`, `cocobod_yield_regression`, `rainfall_interpolate`), `gh_bench.c` (CLI driver), `tests/test_kernels.c` + `tests/run_tests.sh` (16 correctness unit tests), `student_implementation/analyze.py`, `plots.py`, `roofline_model.m`, `student_implementation/src/adaptive_momo_match.c` and its evaluation harness (`student_implementation/evaluate_innovation.sh`, `results/raw/innovation_results.jsonl`), `docs/innovation_writeup.md` |
| Specific tests personally written or run | All 16 correctness fixtures in `tests/test_kernels.c` — 6 for `momo_match` (exact match, beyond/within epsilon, ID not present, mixed batch, bytes-moved reporting), 5 for `cocobod_yield_regression` (noiseless-line recovery, perfect-fit R², residual sum, R² range, constant-yield zero slope), 5 for `rainfall_interpolate` (no-missing passthrough, single-missing, distance-weighting, no-valid-neighbour fallback, corner-missing crash safety). I wrote and ran every one via `tests/run_tests.sh` and hand-verified the expected values against the kernel math before committing. |
| Specific results personally interpreted | The innovation-kernel evaluation results in `results/raw/innovation_results.jsonl` (Week 3): interpreted `adaptive_momo_match`'s performance across problem sizes and wrote up the findings in `docs/innovation_writeup.md`. |
| Git commit hashes attributable to me | `b9fa952` (implement momo_match, cocobod_yield_regression, rainfall_interpolate), `6c5bb13` (gh_bench CLI driver), `b8eede1` (16 correctness unit tests), `98e83e0` (analyze.py, plots.py, roofline_model.m), `4e7dfec` (adaptive innovation kernel), `d2734ef` (evaluate innovation kernel across problem sizes), `3935054` (write up innovation results), `bc55fe5`/`748f2b8` (Week 3 submission packet). |
| Section(s) of the final report personally written | `docs/innovation_writeup.md` (adaptive `momo_match` innovation results and analysis), and the C/C++ implementation details feeding the kernels/testing subsections of the final technical report. |
| Live defence component prepared to present/defend | *"Add a fourth kernel signature live."* Prepared to re-derive the FLOP counts for all three existing kernels by hand and sketch a fourth kernel's signature/loop structure live, following the same interface pattern as `momo_match`/`cocobod_yield_regression`/`rainfall_interpolate` in `kernels.c`. |
| Estimated % of total team effort (self-assessed) | ~30%, based on commit share (9 of my own commits) and being sole author of all three core kernels, the CLI driver, the full correctness test suite, and the innovation kernel — the largest single share of code authorship on the project. |

Signature: Kumi Kelvin Gyabaah Date: 2026-08-28
