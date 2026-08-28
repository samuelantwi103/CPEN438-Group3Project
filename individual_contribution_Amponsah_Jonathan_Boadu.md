# Individual Contribution Statement — Amponsah Jonathan Boadu

Project 1: Roofline Reckoning, CPEN 315/733, Group 3. Role: Computer Architecture & Hardware/Perf Lead.

| Field | Content |
|---|---|
| Name & role | Amponsah Jonathan Boadu — Computer Architecture & Hardware/Perf Lead |
| Specific code/hardware artefacts personally authored | `student_implementation/include/harness.h`, `student_implementation/src/harness.c`, `docs/harness_validation_notes.md`, `docs/architecture_diagram.md`, `student_implementation/run_experiment_matrix.sh`, `student_implementation/stream_triad.c`, `student_implementation/peak_flops.c`, `student_implementation/run_full_pipeline.sh` (verification) |
| Specific tests personally written or run | Ran the cache-busting microbenchmark in `harness_validation_notes.md` §2 and confirmed the fine-grained LLC-miss counter read 0 while the generic `PERF_COUNT_HW_CACHE_MISSES` event responded correctly; ran `run_experiment_matrix.sh` end-to-end and confirmed the 36-record output; ran `run_full_pipeline.sh` in full and confirmed all six stages complete without error. |
| Specific results personally interpreted | The CPI and cache-miss-rate table (`report/technical_report.md` §IV-A) — including the open question of why `rainfall_interpolate`'s miss rate roughly doubles from configA to configB despite being single-threaded, noted as unresolved rather than explained away; the Amdahl-vs-measured-OpenMP gap at 8 threads, attributed to memory-latency contention beyond the CPU's 4 physical cores. |
| Git commit hashes attributable to me | See full commit history at [github.com/samuelantwi103/CPEN438-Group3Project](https://github.com/samuelantwi103/CPEN438-Group3Project), filtered by author. |
| Section(s) of the final report personally written | Section III-B (Measurement Harness) and Section IV-A/IV-C (Results — CPU-time/CPI table, Amdahl's Law and OpenMP extension). |
| Live defence component prepared to present/defend | *"Why does perf show fewer cycles than cycles-by-clock math predicts?"* — Answer: measured cycles reflect the CPU's actual achieved clock rate under Intel Turbo Boost (~2.71 GHz on this short workload), not the 1.70 GHz nominal/advertised clock; the naive nominal-clock calculation over-predicts CPU time by ~59% for exactly this reason (see `docs/manual_cpu_time_worked_example.md`). |
| Estimated % of total team effort (self-assessed) | 95% |

Signature: _________________________________ Date: ______________
