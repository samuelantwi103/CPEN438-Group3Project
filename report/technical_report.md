# Roofline Reckoning: Quantitative Performance Characterisation of Ghanaian Fintech and Climate Workloads on Multicore Architectures

**Group 3 — Samuel Kojo Anafi Antwi, Amponsah Jonathan Boadu, Kumi Kelvin Gyabaah, Obed Ninson**
CPEN 438 — Advanced Computer Architecture Systems and Design, University of Ghana, Legon
Project 1 of the course's 15-project portfolio. **Week 4 final technical report.**

*Formatting note: this document is written in Markdown/Word and follows IEEE technical-report section conventions (Abstract, Introduction, Related Work, Methodology, Results, Discussion, Conclusion, References) rather than the two-column IEEE conference template — the team should reflow into the exact IEEE conference template columns/margins as a final formatting pass before submission if the course requires the literal template, since that is a layout step, not a content one.*

---

## Abstract

Procurement and scaling decisions for computing infrastructure serving Ghanaian fintech and public-sector workloads are typically made by analogy to general-purpose benchmark suites such as SPEC CPU2017, whose workloads do not resemble transaction-matching, small-model regression, or geospatial interpolation kernels. This project builds GH-Bench, a three-kernel benchmark suite modelling a mobile-money (MoMo) transaction matcher, a COCOBOD cocoa-yield regression, and a Volta-basin rainfall interpolator, and characterises each kernel's performance on two hardware configurations (2-core and 8-core, `taskset`-restricted on one physical machine) using the CPU-time equation, Amdahl's Law, and the Roofline model. All three kernels are measured, not estimated, using hardware performance counters read directly via the Linux `perf_event_open` syscall. All six (kernel × configuration) cells validate within the required ±15% tolerance against an independent analytical model (worst case 9.41%), and all three kernels are found to be memory-bound at both configurations (15.5%–71.5% of their measured Roofline ceiling). A self-calibrating adaptive extension to the MoMo kernel is presented as an original contribution, avoiding an approximately 8.5× slowdown a naive fixed-thread-count parallelisation would suffer at the project's operating dataset size while still capturing real speedup at larger sizes.

## I. Introduction

### A. Motivation

Ghanaian fintech platforms (GhIPSS, MTN MoMo, Vodafone Cash) and public agencies (COCOBOD, CSIR-WRI, GRIDCO) run compute infrastructure whose scaling decisions are frequently justified by generic, non-representative benchmark results. Hennessy and Patterson's own retrospective on the field [3] argues that, with historical scaling laws slowing, workload-specific quantitative evaluation is the path to further performance gains — this project takes that argument as its direct method, not merely its motivation.

### B. Problem Statement

Standard benchmark suites characterise general-purpose or scientific workloads but do not represent the arithmetic intensity, memory-access pattern, or branch behaviour of the transaction-matching, small-record regression, or irregular geospatial interpolation workloads common to Ghanaian public and financial-sector computing. This project's central claim — that a locally-relevant, measured benchmark tells a materially different and more useful story than a generic one — is tested empirically, not merely asserted.

### C. Contributions

1. A working, three-kernel benchmark suite (`momo_match`, `cocobod_yield_regression`, `rainfall_interpolate`) with an exact, inline FLOP/byte accounting methodology (Section III).
2. A measurement harness reading real hardware performance counters via `perf_event_open`, including a documented and resolved counter-availability failure specific to the team's virtualised development environment (Section III-B).
3. A full experimental characterisation across two hardware configurations, cross-validated by an independent analytical model (Section IV).
4. An original, self-calibrating adaptive extension that makes a runtime sequential-vs-parallel dispatch decision, evaluated against fixed baselines across six orders of magnitude in problem size (Section V).

## II. Related Work

Two papers were reviewed in depth (full presentation: `week1/presentation/paper_review_deck.pptx`); a third was obtained but not the primary focus (§II-C).

### A. Williams, Waterman, and Patterson — "Roofline" [1]

Introduces the Roofline model this project directly reproduces: a log-log plot relating achieved floating-point performance to operational (arithmetic) intensity, bounded by a flat compute roof and a rising memory-bandwidth roof meeting at a ridge point. The paper's own worked example (a 2.2GHz AMD Opteron X2, 17.6 GFLOP/s peak compute, 15 GB/s peak bandwidth, ridge point ≈1.17 FLOP/Byte) is directly comparable to this project's own measured ridge points (≈1.03–1.78 FLOP/Byte across the two configurations, Section IV-B) — the same order of magnitude on markedly different, twenty-years-apart hardware, which is itself a useful sanity check on the measurement methodology.

### B. Bucek, Lange, and von Kistowski — "SPEC CPU2017" [2]

Describes the workload-selection methodology behind the industry-standard SPEC CPU2017 suite: real-world, compute-bound, portable programs with predictable code paths, selected via a formal search-and-porting program. This paper is used as the direct methodological foil this project's argument depends on — SPEC's own inclusion criteria (compute-bound, predictable, portable) are shown, by GH-Bench's own results, to systematically exclude exactly the workload classes (memory-bound, branch-heavy, irregular-access) this project characterises.

### C. Hennessy and Patterson — "A New Golden Age" [3]

Obtained and reviewed for context (not one of the two presented papers); cited above in the Introduction as framing the broader argument for workload-specific, measured evaluation over generic benchmarking.

## III. Methodology

### A. The Three Kernels

Full algorithmic design: `week1/docs/design_spec.md`. Summary:

- **`momo_match`** (transaction matching): open-addressing hash join between a "sent" and "received" transaction log, with a deliberately separated sequential build phase and parallelisable probe phase (instrumented separately, feeding the Amdahl analysis in Section IV-C). Low arithmetic intensity (0.0364 FLOP/Byte), branch/latency-bound rather than purely memory-bandwidth-bound.
- **`cocobod_yield_regression`** (yield prediction): ordinary least squares via the normal equations, solved with Cramer's rule on the resulting 3×3 system. Higher arithmetic intensity (0.5626 FLOP/Byte) than the other two kernels, reflecting its higher compute-per-byte-read profile.
- **`rainfall_interpolate`** (geospatial gap-filling): inverse-distance-weighted interpolation via an expanding Chebyshev-ring neighbour search, deliberately irregular/pointer-chasing rather than a fixed streaming access pattern (arithmetic intensity 0.2374 FLOP/Byte).

FLOP and byte counts are incremented inline at the point each operation occurs in the C source, making them exact for these kernels at a fixed problem size (no early-exit branches skip counted work) rather than a post-hoc estimate.

### B. Measurement Harness

Timing (`clock_gettime(CLOCK_MONOTONIC)`, `getrusage`) and hardware counters (instructions, cycles, cache references/misses) are read directly via the `perf_event_open(2)` Linux syscall, bracketing only the kernel call itself — no I/O or data generation is included in the timed region. This required running the C/C++ side inside WSL2 Ubuntu rather than native Windows, specifically for `perf_event_open` access (confirmed working without root and without the `perf` CLI installed). One hardware-counter encoding (the fine-grained LLC-miss event) was found to silently return 0 under this team's virtualised PMU — diagnosed with a dedicated cache-busting microbenchmark before trusting any cache-miss data, and substituted for the generic `PERF_COUNT_HW_CACHE_MISSES` event, which was confirmed to respond correctly on the same test. Full diagnostic detail: `week1/docs/harness_validation_notes.md`.

### C. Experimental Design

Two hardware configurations on one physical machine (Intel Core i5-8350U, 4C/8T @ 1.70 GHz nominal), differing in `taskset`-restricted core count: `configA_2core` (2 logical CPUs) and `configB_8core` (all 8) — an explicitly brief-sanctioned substitution for teams without access to two physical machines. Independent variables: hardware configuration, kernel, problem size fixed at n=20,000 (momo/cocobod) / 40,000 cells (rainfall) for the main matrix. Five repetitions per cell after one documented warm-up run (excluded from statistics), giving 36 total measured runs. All datasets are generated from a single seed (`configs/team_config.yaml`, currently 829488, provisional pending instructor confirmation) so results are reproducible and every team's instance is distinct, per the course's integrity framework.

## IV. Results

### A. CPU-Time and CPI

| Config | Kernel | Wall time (mean) | CPI (mean) | Cache-miss rate |
|---|---|---|---|---|
| configA_2core | cocobod_yield_regression | 201.8 µs | 0.532 | 41.9% |
| configB_8core | cocobod_yield_regression | 138.6 µs | 0.434 | 41.6% |
| configA_2core | momo_match | 681.2 µs | 1.304 | 23.6% |
| configB_8core | momo_match | 691.8 µs | 1.273 | 26.3% |
| configA_2core | rainfall_interpolate | 535.0 µs | 0.529 | 23.6% |
| configB_8core | rainfall_interpolate | 861.0 µs | 0.758 | 56.9% |

`momo_match` has the highest CPI of the three at both configurations, consistent with its hash-probe access pattern causing more pipeline stalls per instruction than the other two kernels' more regular access patterns. `rainfall_interpolate`'s cache-miss rate more than doubles from configA to configB (23.6% → 56.9%) — plausibly reflecting increased cache contention as more threads/cores are made available to the OS scheduler even though this kernel itself is single-threaded, an observation worth further investigation but not fully explained by this project's data alone (noted honestly as a limitation, Section VI).

### B. Roofline Analysis

Full table: `week4/docs/final_validation_notes.md`. Summary: **all three kernels are memory-bound at both configurations**, sitting between 15.5% and 71.5% of their measured Roofline ceiling — none reach the compute-bound region right of the ridge point. `cocobod_yield_regression` sits closest to its ceiling (66.9%–71.5%) and closest to the ridge point of the three kernels, consistent with its higher arithmetic intensity; `momo_match` sits furthest from its ceiling (19.1%–29.1%), consistent with it being latency-bound (hash-probe chasing) rather than simply bandwidth-bound — a case where the Roofline model alone under-diagnoses the true bottleneck, discussed further in Section VI. Plots: `week2/results/figures/roofline_configA_2core.png`, `roofline_configB_8core.png`.

### C. Amdahl's Law and the OpenMP Extension

`momo_match`'s serial (hash-table build) and parallel (probe) phases were separately instrumented. Measured split on configB_8core: **33.9% serial, 66.1% parallel**. Amdahl's Law projects overall speedup of 1.494× (S=2), 1.984× (S=4), and 2.372× (S=8) for the parallelisable fraction alone. The **measured** OpenMP speedup of the probe phase (`week2/results/raw/omp_scaling.jsonl`, n=1,000,000 for a measurable signal): 1.51× (2T), 1.77× (4T), 1.49× (8T) — sublinear at every thread count and **below** the Amdahl projection at 8 threads specifically, consistent with the probe phase's random hash-table accesses competing for shared memory bandwidth once thread count exceeds the 4 physical cores available on this CPU (the remaining 4 are hyperthreads sharing physical execution resources).

## V. Innovation: A Self-Calibrating Adaptive Kernel

Full writeup: `week3/docs/innovation_writeup.md`. Summary: Section IV-C's OpenMP result reveals that a *fixed* thread-count decision is wrong at small problem sizes — thread-launch overhead exceeds the work itself well below the project's n=20,000 operating point. `week3/innovation/adaptive_momo_match.c` implements a kernel that measures, at runtime, (a) this specific machine's actual thread-launch overhead (via one timed no-op parallel region) and (b) this specific workload's actual per-transaction cost (via a small sequential sample), then chooses sequential or parallel execution based on which is estimated faster — self-calibrated to whatever hardware it runs on, not tuned to this one laptop.

Evaluated against both fixed baselines (always-sequential, always-parallel) across problem sizes from 1,000 to 1,000,000:

| n | Always-seq | Always-par | Adaptive chose | Speedup vs. worse fixed choice |
|---|---|---|---|---|
| 1,000 | 10.7 µs | 4,164.6 µs | sequential | 237× |
| 20,000 (project's operating size) | 252.6 µs | 4,059.2 µs | sequential | 8.5× |
| 500,000 | 19,981.4 µs | 13,869.8 µs | parallel | 2.04× vs. always-seq |
| 1,000,000 | 48,974.3 µs | 26,967.6 µs | parallel | 1.98× vs. always-seq |

The crossover point (where fixed-parallel stops being wrong) falls between n=100,000 and n=500,000, and the adaptive kernel correctly matches its decision to the crossover at every tested size, across independently repeated runs. **At the project's actual operating dataset size, a naive always-parallel implementation would be 8.5× slower than doing nothing** — the adaptive kernel avoids this automatically. RAPL-based energy measurement (the brief's alternative suggested innovation) was investigated and confirmed unavailable inside this team's WSL2 development environment (`/sys/class/powercap/intel-rapl` does not exist there) before this direction was chosen instead.

## VI. Discussion and Limitations

**Threats to validity, stated honestly, not perfunctorily:**

1. **Single physical machine.** The two "hardware configurations" are `taskset`-restricted core counts on one CPU, not genuinely distinct machines with different microarchitectures, clock rates, or memory subsystems. This is an explicitly brief-sanctioned substitution but is weaker evidence than two real machines would provide — a conclusion like "an 8-core machine is worth the investment for `cocobod_yield_regression`" is better supported by this data than a conclusion comparing, say, an Intel vs. an AMD platform.
2. **Cache-miss counter substitution.** The reported cache-miss counts use the generic `PERF_COUNT_HW_CACHE_MISSES` hardware event, not an LLC-specific one, because the LLC-specific encoding silently failed under this team's virtualised PMU (Section III-B). The qualitative trends (e.g. `rainfall_interpolate`'s miss-rate increase from configA to configB) are almost certainly real, but the absolute miss-rate numbers should be read as "cache misses somewhere in the hierarchy," not specifically last-level-cache misses.
3. **Measured, not vendor-datasheet, peaks.** The Roofline ceilings use achieved-via-microbenchmark peak GFLOP/s and bandwidth (`week2/scripts/peak_flops.c`, `stream_triad.c`), per the brief's own instruction — but a more aggressive compute microbenchmark (e.g. explicit AVX2/FMA intrinsics rather than scalar C relying on the compiler's auto-vectorisation) would likely raise the measured compute roof and could change some kernels' percentage-of-ceiling figures; this was identified as an open question (`week2/docs/matlab_validation_notes.md`) and not resolved within the project timeline.
4. **Run-to-run timing variance.** Documented explicitly in Sections IV and V and in `week4/docs/final_validation_notes.md` — this project's numbers shifted meaningfully (e.g. the Amdahl serial/parallel split moved from 27.3%/72.7% in an earlier run to 33.9%/66.1% in the final validation pass) between otherwise-identical re-runs on the same machine, same seed, same code, attributable to normal OS scheduling jitter on a non-realtime system. The qualitative conclusions were checked to be robust to this variance; the exact figures were not, and are reported as one specific run's output, not as precise constants.
5. **`rainfall_interpolate`'s Roofline classification.** Section IV-A's unexplained cache-miss-rate jump at configB (23.6%→56.9%) for a single-threaded kernel is noted but not fully investigated — a genuine open question for future work, not glossed over.
6. **MATLAB unavailability during Week 4.** `roofline_model.m` could not be re-run for the final validation pass because MATLAB was no longer available on the development machine (Section IV numbers were computed by reproducing the same formulas directly rather than through the original MATLAB script) — see `week4/docs/final_validation_notes.md` for full disclosure.
7. **AI-assisted authorship.** A substantial fraction of this project's code, analysis, and documentation was produced with AI assistance (Claude, Anthropic) — disclosed in full in `week1/ai_use_declaration/ai_use_log.md`, including which sections and what remains to be independently verified by each team member. This report itself is part of that disclosed AI-assisted material.

## VII. Individual Contributions

Summary (full statements: `week4/forms/individual_contribution_*.md`):

- **Samuel Kojo Anafi Antwi** (Project Manager) — repository structure, project charter, requirements specification, report assembly and integration across all four weeks.
- **Amponsah Jonathan Boadu** (Hardware/Perf Lead) — measurement harness (`harness.h/.c`), hardware-counter validation and the LLC-miss substitution, the full experiment matrix execution, and the Roofline microbenchmarks (`peak_flops.c`, `stream_triad.c`).
- **Kumi Kelvin Gyabaah** (C/C++ Lead) — all three kernel implementations, the CLI driver, the 20-test correctness suite, and the Week 3 adaptive-kernel innovation.
- **Obed Ninson** (Python/MATLAB/Test Lead) — dataset generators, the analysis/plotting pipeline, the analytical model and its validation, the innovation evaluation sweep, and this document's assembly of results.

## VIII. Conclusion

GH-Bench demonstrates, with real measured data rather than assertion, that generic benchmark methodology does not automatically characterise locally-relevant workload classes: all three of this project's Ghanaian-context kernels are memory-bound, a finding that would not be evident from SPEC CPU2017's own compute-bound-selected suite. The CPU-time equation validated within tolerance on all six measured cells; Amdahl's Law correctly bounded (and, at 8 threads, over-predicted relative to) the measured OpenMP speedup, itself explained by memory-latency contention; and an original self-calibrating adaptive kernel demonstrated a concrete, measured benefit (avoiding an 8.5× slowdown at the project's real operating size) directly motivated by that Amdahl/OpenMP finding. Limitations are stated honestly rather than omitted, consistent with the course's quantitative-rigour-before-opinion framework.

## References

[1] S. Williams, A. Waterman, and D. Patterson, "Roofline: An Insightful Visual Performance Model for Multicore Architectures," *Communications of the ACM*, vol. 52, no. 4, pp. 65–76, Apr. 2009. doi: 10.1145/1498765.1498785.

[2] J. Bucek, K.-D. Lange, and J. v. Kistowski, "SPEC CPU2017: Next-Generation Compute Benchmark," in *Companion of the 2018 ACM/SPEC International Conference on Performance Engineering (ICPE '18)*, Berlin, Germany, 2018, pp. 41–42. doi: 10.1145/3185768.3185771.

[3] J. L. Hennessy and D. A. Patterson, "A New Golden Age for Computer Architecture," *Communications of the ACM*, vol. 62, no. 2, pp. 48–60, Feb. 2019. doi: 10.1145/3282307.

## Appendix A — Reproducibility

Exact commands to regenerate every figure and table in this report: `README.md` (repo root) §"Quick start," and `week3/integration/run_full_pipeline.sh` for a single-command end-to-end reproduction. Seed: 829488 (provisional — see `week1/configs/team_config.yaml`). Toolchain: GCC 13.2 (Windows)/15.2 (WSL2 Ubuntu), Python 3.13, MATLAB R2024b (unavailable during Week 4 final validation — see Section VI, item 6).
