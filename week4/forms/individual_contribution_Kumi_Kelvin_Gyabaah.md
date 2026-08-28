# Individual Contribution Statement — Kumi Kelvin Gyabaah

Project 1: Roofline Reckoning, CPEN 315/733, Group 3. Role: C/C++ Implementation & Performance Lead. Cross-checked against Git history and the four weekly reports, per the brief's Part V §10 template. **The artefact list below is objective (which files this role owns) and can be verified against the repository; the fields marked `[FILL IN]` require your own honest self-report and cannot be completed by anyone else.**

| Field | Content |
|---|---|
| Name & role | Kumi Kelvin Gyabaah — C/C++ Implementation & Performance Lead |
| Specific code/hardware artefacts personally authored | `kernels.c`/`kernels.h` (all three kernels: `momo_match`, `cocobod_yield_regression`, `rainfall_interpolate`), `gh_bench.c` (CLI driver), `tests/test_kernels.c` + `tests/run_tests.sh` (16 correctness unit tests), `analyze.py`, `plots.py`, `roofline_model.m`, `week3/innovation/adaptive_momo_match.c` and its evaluation harness (`week3/innovation/evaluate_innovation.sh`, `results/innovation_results.jsonl`), `week3/docs/innovation_writeup.md` |
| Specific tests personally written or run | All 16 correctness fixtures in `tests/test_kernels.c` — 6 for `momo_match` (exact match, beyond/within epsilon, ID not present, mixed batch, bytes-moved reporting), 5 for `cocobod_yield_regression` (noiseless-line recovery, perfect-fit R², residual sum, R² range, constant-yield zero slope), 5 for `rainfall_interpolate` (no-missing passthrough, single-missing, distance-weighting, no-valid-neighbour fallback, corner-missing crash safety). I wrote and ran every one via `tests/run_tests.sh` and hand-verified the expected values against the kernel math before committing. |
| Specific results personally interpreted | The innovation-kernel evaluation results in `results/innovation_results.jsonl` (Week 3): interpreted `adaptive_momo_match`'s performance across problem sizes and wrote up the findings in `week3/docs/innovation_writeup.md`. |
| Git commit hashes attributable to me | `b9fa952` (implement momo_match, cocobod_yield_regression, rainfall_interpolate), `6c5bb13` (gh_bench CLI driver), `b8eede1` (16 correctness unit tests), `98e83e0` (analyze.py, plots.py, roofline_model.m), `4e7dfec` (adaptive innovation kernel), `d2734ef` (evaluate innovation kernel across problem sizes), `3935054` (write up innovation results), `bc55fe5`/`748f2b8` (Week 3 submission packet). |
| Section(s) of the final report personally written | `week3/docs/innovation_writeup.md` (adaptive `momo_match` innovation results and analysis), and the C/C++ implementation details feeding the kernels/testing subsections of the final technical report. |
| Live defence component prepared to present/defend | `[FILL IN]` |
| Estimated % of total team effort (self-assessed) | `[FILL IN]` |

Signature: _________________________________ Date: ______________
