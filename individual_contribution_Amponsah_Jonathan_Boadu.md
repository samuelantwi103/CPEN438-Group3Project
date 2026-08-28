# Individual Contribution Statement — Amponsah Jonathan Boadu

Project 1: Roofline Reckoning, CPEN 315/733, Group 3. Role: Computer Architecture & Hardware/Perf Lead. Cross-checked against Git history and the four weekly reports, per the brief's Part V §10 template. **The artefact list is objective (which files this role owns) and can be verified against the repository. Rows marked `DRAFT` are suggested language grounded in the real project work — read each one and either confirm it's true for you as written, edit it to match what you actually did, or replace it — before you sign. Do not sign this with a DRAFT row you have not personally checked; that would defeat the entire point of the form.**

| Field | Content |
|---|---|
| Name & role | Amponsah Jonathan Boadu — Computer Architecture & Hardware/Perf Lead |
| Specific code/hardware artefacts personally authored | `week1/student_implementation/include/harness.h`, `week1/student_implementation/src/harness.c`, `week1/docs/harness_validation_notes.md`, `week1/docs/architecture_diagram.md`, `week2/scripts/run_experiment_matrix.sh`, `week2/scripts/stream_triad.c`, `week2/scripts/peak_flops.c`, `week2/scripts/omp_probe_scaling.c`, `week2/scripts/run_omp_scaling.sh`, `week3/integration/run_full_pipeline.sh` (verification) |
| Specific tests personally written or run | **DRAFT — confirm or edit:** Ran the cache-busting microbenchmark in `harness_validation_notes.md` §2 and confirmed the fine-grained LLC-miss counter read 0 while the generic `PERF_COUNT_HW_CACHE_MISSES` event responded correctly; ran `week2/scripts/run_experiment_matrix.sh` end-to-end and confirmed the 36-record output; ran `week3/integration/run_full_pipeline.sh` in full and confirmed all six stages complete without error. |
| Specific results personally interpreted | **DRAFT — confirm or edit:** The CPI and cache-miss-rate table (`week4/report/technical_report.md` §IV-A) — including the open question of why `rainfall_interpolate`'s miss rate roughly doubles from configA to configB despite being single-threaded, noted as unresolved rather than explained away; the Amdahl-vs-measured-OpenMP gap at 8 threads, attributed to memory-latency contention beyond the CPU's 4 physical cores. |
| Git commit hashes attributable to me | See full commit history at [github.com/samuelantwi103/CPEN438-Group3Project](https://github.com/samuelantwi103/CPEN438-Group3Project), filtered by author. |
| Section(s) of the final report personally written | **DRAFT — confirm or edit:** Section III-B (Measurement Harness) and Section IV-A/IV-C (Results — CPU-time/CPI table, Amdahl's Law and OpenMP extension). |
| Live defence component prepared to present/defend | **DRAFT — confirm or edit:** *"Why does perf show fewer cycles than cycles-by-clock math predicts?"* — Answer: measured cycles reflect the CPU's actual achieved clock rate under Intel Turbo Boost (~2.71 GHz on this short workload), not the 1.70 GHz nominal/advertised clock; the naive nominal-clock calculation over-predicts CPU time by ~59% for exactly this reason (see `week2/docs/manual_cpu_time_worked_example.md`). |
| Estimated % of total team effort (self-assessed) | 95% |

**Before signing:** re-run the perf_event_open smoke test and the full experiment matrix yourself, on your own checkout — this role's credibility at the defence depends on you being able to reproduce and explain every number attributed to your column, not just having read about it.

Signature: _________________________________ Date: ______________
