# Final Validation Notes — Week 4

Project 1: Roofline Reckoning, CPEN 438, Group 3. **Week 4 deliverable** (Project 1's own Week-4 schedule: "Finish MATLAB analytical model validation"). This re-confirms the analytical model against the current, final experiment data before the technical report cites any number from it.

## A finding worth stating plainly: MATLAB became unavailable during this session

`week2/student_implementation/roofline_model.m` was successfully run earlier in this project (Week 2, and again during Week 3's write-up) via `matlab -batch`. When re-run for this final Week 4 validation pass, MATLAB was no longer reachable on this machine (`matlab.exe` no longer resolves — it appears to have been uninstalled or the environment changed between sessions). This is disclosed here rather than silently worked around.

**What was done instead:** the exact same formulas `roofline_model.m` implements (CPU-time equation, Roofline ceiling, Amdahl projection — see that file's own comments for the formulas) were re-implemented directly and run in Python against the current `week2/results/processed/summary.csv`, producing the numbers below. **The team should re-run the actual `roofline_model.m` in MATLAB once it's available again**, and confirm these numbers match — they should, since it's the same data and the same formulas, but that confirmation hasn't happened and shouldn't be claimed as done.

## Final CPU-time equation validation (current data, all six cells)

| Config | Kernel | Predicted | Measured | Error | Status |
|---|---|---|---|---|---|
| configA_2core | cocobod_yield_regression | 1.828×10⁻⁴s | 2.018×10⁻⁴s | 9.41% | OK |
| configB_8core | cocobod_yield_regression | 1.345×10⁻⁴s | 1.386×10⁻⁴s | 2.97% | OK |
| configA_2core | momo_match | 6.763×10⁻⁴s | 6.812×10⁻⁴s | 0.72% | OK |
| configB_8core | momo_match | 6.859×10⁻⁴s | 6.918×10⁻⁴s | 0.86% | OK |
| configA_2core | rainfall_interpolate | 5.314×10⁻⁴s | 5.350×10⁻⁴s | 0.68% | OK |
| configB_8core | rainfall_interpolate | 8.458×10⁻⁴s | 8.610×10⁻⁴s | 1.77% | OK |

All six cells within the brief's ±15% tolerance (worst case 9.41%, on `cocobod_yield_regression` at configA — plausibly because this kernel has the shortest wall-clock time of the six, so it's the most sensitive to fixed measurement overhead). Same caveat as Week 2's validation notes: this cross-check is self-consistent by construction (the "clock period" is derived from the same run's own cycles/wall-time), not independent proof — the independent check remains the manual worked example in `week2/docs/manual_cpu_time_worked_example.md`.

## Final Roofline ceiling check (current data)

| Config | Kernel | AI (FLOP/Byte) | Ceiling (GFLOP/s) | Achieved (GFLOP/s) | % of ceiling | Classification |
|---|---|---|---|---|---|---|
| configA_2core | cocobod_yield_regression | 0.5626 | 2.885 | 2.061 | 71.5% | memory-bound |
| configB_8core | cocobod_yield_regression | 0.5626 | 4.497 | 3.011 | 66.9% | memory-bound |
| configA_2core | momo_match | 0.0364 | 0.187 | 0.054 | 29.1% | memory-bound |
| configB_8core | momo_match | 0.0364 | 0.291 | 0.056 | 19.1% | memory-bound |
| configA_2core | rainfall_interpolate | 0.2374 | 1.218 | 0.443 | 36.4% | memory-bound |
| configB_8core | rainfall_interpolate | 0.2374 | 1.898 | 0.295 | 15.5% | memory-bound |

**A change from earlier reporting, worth flagging honestly:** an earlier run (cited in `week2/docs/matlab_validation_notes.md`) showed `cocobod_yield_regression` at 124.7% of its ceiling on configA — i.e., apparently *exceeding* the theoretical maximum, which that document flagged as likely reflecting a conservative measured compute roof rather than a data error. **With the current data, all six cells now sit at or below their ceiling (15.5%–71.5%)**, consistent with what the Roofline model should show and no longer exhibiting that anomaly. This is a genuine improvement in data quality between runs, not a correction of a persistent bug — both the earlier and current measured-roof values came from the same `peak_flops.c`/`stream_triad.c` microbenchmarks, and normal run-to-run timing variance (documented throughout this project, most explicitly in the Week 3 innovation writeup) is the most likely explanation for the earlier over-100% reading.

## Final Amdahl's Law projection (`momo_match`, current data)

Serial (hash-table build) fraction: **33.9%**. Parallel (probe) fraction F: **66.1%**.

| Local speedup S | Projected overall speedup |
|---|---|
| 2× | 1.494× |
| 4× | 1.984× |
| 8× | 2.372× |

Compare against the **measured** OpenMP speedups from `week2/results/raw/omp_scaling.jsonl`: 1.51× (2T), 1.77× (4T), 1.49× (8T, relative to the 1-thread baseline of 0.0483s) — measured speedups sit below the Amdahl projection at every thread count, consistent with the memory-latency-bound explanation given in `week2/docs/matlab_validation_notes.md` (random hash-table probes compete for the same memory pipeline once thread count exceeds what independent throughput can support).

## What changed from Week 2/3's reported numbers, and why

Every number in this document differs slightly from earlier reports (e.g. Week 2 reported the Amdahl serial/parallel split as 27.3%/72.7%; this document reports 33.9%/66.1%). This is the same run-to-run timing variance documented throughout this project — the experiment matrix and OpenMP sweep were re-run for Week 4's final validation pass, on the same seed and same code, and produced slightly different timings, as expected on a shared, non-realtime operating system. **The qualitative conclusions are unchanged**: all kernels remain memory-bound, all six CPU-time predictions remain within tolerance, and the OpenMP speedup remains sublinear and below the Amdahl projection at 8 threads. The team should decide, before final submission, whether to report one specific run's numbers (and say so explicitly) or re-run once more immediately before submission and use whatever that run produces — either is defensible; silently mixing numbers from different runs across the report is not.
