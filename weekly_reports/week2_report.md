# Weekly Course Project Report — Week 2

**Course Code and Title:** CPEN 315 / CPEN 733 — Advanced Computer Architecture Systems and Design
**Project Title:** Project 1 — Roofline Reckoning
**Team Number/Name:** Group 3
**Reporting Week:** Week 2 — Period: 2026-08-19 to 2026-08-19 *(compressed same-day submission alongside Week 1; see note)*
**Team Leader/Reporter:** Samuel Kojo Anafi Antwi
**Submission Date:** 2026-08-19

## 0. Deliverable Compliance Checklist

Cross-referenced against the master brief's general Four-Week Structure (Part I §4) **and** Project 1's own Week-2 schedule line.

| Required deliverable (brief's own wording) | Status | Evidence |
|---|---|---|
| Design spec | ✅ | `docs/design_spec.md` / `.docx` — module-level design for all three kernels, JSON schema, CLI design |
| Diagrams | ✅ | `docs/design_spec.md` §7 (kernel-level data-flow diagram), plus Week 1's system-level diagram |
| First working modules | ✅ | `harness.c`, `kernels.c`, `gh_bench.c` — compiled clean (`-Wall -Wextra`, zero warnings), actually executed |
| Unit tests | ✅ | `tests/test_kernels.c`, 16/16 passing |
| Manual trace | ✅ | `docs/manual_cpu_time_worked_example.md` — the brief's own Week-2 line for this project is specifically the CPU-time hand calculation |
| Commit history | ❌ | Only the AI-scaffold commits exist. **This is a genuine gap only the team can close** — see §6 and the AI-use log's action items |
| *(Project 1-specific)* Implement `momo_match` + one more kernel | ✅ | All three implemented (exceeds the Week-2 minimum of two) |
| *(Project 1-specific)* Correctness unit tests against reference implementations | ✅ | `tests/test_kernels.c`; `cocobod` fit checked against generator's ground truth |
| *(Project 1-specific)* Manual CPU-time calculation, confirmed with code | ✅ | `docs/manual_cpu_time_worked_example.md`, cross-checked in `roofline_model.m` |

## 1. Weekly Objective and Progress Summary

**Main objective for the week (per brief):** "Implement `momo_match` and one of the remaining two kernels; write correctness unit tests against reference implementations; produce first manual CPU-time calculation by hand for a small input, then confirm with code."

**Overall project completion:** ~70% *(all three kernels implemented and tested — ahead of the minimum Week-2 scope of two — plus the full experiment matrix, Roofline/CPI/Amdahl plots, the MATLAB analytical model with validation write-up, the design spec, and the OpenMP extension were all pulled forward from Week 3; remaining Week-3 scope is the innovation component and the mid-project demo rehearsal; see §6.)*

**How this week's work fits together:** the design spec (Samuel/whole team) formalizes the internal contracts Kumi's kernels already implement; Amponsah's full experiment matrix is the only reason Obed's `analyze.py`/`plots.py`/`roofline_model.m` have real data to summarize; the MATLAB model's cross-validation is only meaningful because it checks Amponsah's measured numbers against Kumi's exact FLOP/byte counts. None of this week's individual outputs stands as a deliverable on its own — see `docs/project_charter.md` §1.

**Summary of progress achieved:**
- All three kernels (`momo_match`, `cocobod_yield_regression`, `rainfall_interpolate`) implemented in `student_implementation/src/kernels.c`, each with exact inline FLOP/byte accounting.
- **Design specification written** (`docs/design_spec.md`): data structures, algorithms, complexity, and interface contracts for all three kernels plus the harness — the Week-2 "design spec" deliverable.
- 16 unit tests written and passing (`tests/test_kernels.c`), including one caught-and-fixed test-fixture bug (documented in-line — a lesson in the difference between a kernel defect and a bad fixture).
- Manual CPU-time hand calculation completed and cross-checked against measured hardware counters — see `docs/manual_cpu_time_worked_example.md`. Finding: the naive nominal-clock (1.70 GHz) prediction over-predicts measured time by ~59%, consistent with Intel Turbo Boost pushing the effective clock to ~2.71 GHz on this short workload — a genuine, checked, explainable deviation, not a bug.
- Full experiment matrix executed (3 kernels × 2 configs × 5 reps + 1 warm-up = 36 real hardware-counter-backed runs), summarized (`results/processed/summary.csv`) and plotted (`results/figures/`).
- Roofline "roof" microbenchmarks (`stream_triad.c`, `peak_flops.c`) run per configuration; ridge points computed.
- **MATLAB analytical model built and run** (`roofline_model.m`): CPU-time equation, Roofline ceiling, and Amdahl projection, cross-validated against `results/processed/summary.csv` within the brief's ±15% tolerance on all six cells — see `docs/matlab_validation_notes.md` for the honest interpretation (including the important caveat that the CPU-time cross-check is self-consistent by construction, not independent validation — the independent check is the manual worked example above).
- OpenMP extension (Level 3 "Intermediate" task) implemented and measured: real (not projected) speedup of `momo_match`'s probe phase at 1/2/4/8 threads (1.0×, 1.80×, 2.01×, 2.21× — sublinear, consistent with a memory-latency-bound hash-probe workload plateauing against shared memory bandwidth on a 4-core/8-thread machine).

## 2. Meetings Held

Not yet recorded.

## 3. Work Completed and Individual Contributions

| Member | Assigned Task | Work Completed | Evidence/Output | Status |
|---|---|---|---|---|
| Samuel Kojo Anafi Antwi (PM) | Integration branch, progress report | Report assembly for Weeks 1–2; design spec assembled from all three technical leads' input; repo structure integrity check | This report, `docs/design_spec.md`; repo tree | Ongoing — **needs personal Git commit** |
| Amponsah Jonathan Boadu (Hardware/Perf) | Counter wiring, run experiment matrix | Full 36-run matrix executed with real `perf_event_open` counters; roof microbenchmarks run | `results/raw/results.jsonl`, `configs/team_config.yaml` (measured_roofs) | Ongoing — **needs personal Git commit** |
| Kumi Kelvin Gyabaah (C/C++) | Implement 2 kernels | All 3 kernels implemented; OpenMP probe-scaling extension | `kernels.c`, `scripts/omp_probe_scaling.c` | Ongoing — **needs personal Git commit** |
| Obed Ninson (Python/MATLAB/Test) | Experiment driver, unit tests | `analyze.py`, `plots.py`, `roofline_model.m` run and validated, `test_kernels.c` (16/16 passing) | `results/figures/*.png`, `docs/matlab_validation_notes.md`, `tests/test_kernels.c` | Ongoing — **needs personal Git commit** |

## 4. Work Process and Technical Activities

**Methods, tools, and procedures used:** see Week 1 report — unchanged toolchain. Additional this week: OpenMP (`-fopenmp`) for the probe-phase parallel extension; `phase1_seconds`/`phase2_seconds` instrumentation added to `gh_kernel_args_t` to separate `momo_match`'s serial hash-build phase from its parallelisable probe phase, feeding the Amdahl's-Law projection.

**Testing, verification, or review conducted:**
- Unit tests: 16/16 passing across all three kernels, including a boundary edge case (rainfall cell with zero valid neighbours in radius — verified no crash/divide-by-zero) and an epsilon-boundary case for `momo_match`.
- `cocobod_yield_regression`'s fitted coefficients on the real generated dataset (b0=0.660, b1=0.001287, b2=0.003856, R²=0.986) were checked against the generator's known ground-truth coefficients (b0=0.659, b1=0.001288, b2=0.003856) — recovered within noise, as expected.
- Roofline plot expected trends (brief §M) checked and matched: `rainfall_interpolate` sits well left of the ridge point (memory-bound, AI=0.237), `cocobod_yield_regression` sits closest to the ridge point of the three (AI=0.563 vs. ridge≈1.03–1.78 depending on config), `momo_match` is the most memory-bound (AI=0.036).

## 5. Challenges, Changes, and Corrective Actions

| Challenge or Deviation | Effect on Project | Action Taken/Proposed | Responsible Member |
|---|---|---|---|
| Naive nominal-clock CPU-time prediction over-predicted measured time by ~59% | Could be mistaken for a measurement bug | Diagnosed as Turbo Boost (effective clock ~2.71 GHz vs. 1.70 GHz nominal); documented with the full calculation | Amponsah Jonathan Boadu / Obed Ninson |
| OMP scaling test initially stalled for minutes reading a large CSV from `/mnt/c` (WSL2's slow 9P bridge to the Windows filesystem) | Blocked the OpenMP extension measurement | Staged the dataset into WSL2's native filesystem (`/tmp`) before the sweep; documented as a known WSL2 I/O characteristic | Amponsah Jonathan Boadu |
| All three kernels done in Week 2 instead of two | None negative — ahead of schedule | Week 3 scope shifted toward deeper validation, the innovation component, and defence prep | Whole team |

## 6. Plan for Week 3

| Planned Task or Deliverable | Responsible Member(s) | Deadline | Expected Evidence |
|---|---|---|---|
| Investigate raising the measured compute roof (AVX2/FMA intrinsics in `peak_flops.c`) per `docs/matlab_validation_notes.md`'s open question on the ridge-point position | Amponsah Jonathan Boadu | Week 3 | Updated `peak_flops.c`, re-measured roofs |
| Finalise and implement the innovation-challenge idea (adaptive `momo_match` strategy or energy estimator — confirm choice) | Whole team | Week 3 | New code + baseline-vs-enhanced comparison |
| Mid-project demo rehearsal | Whole team | Week 3 | Live run of `scripts/run_experiment_matrix.sh` end-to-end |
| Each member's personal Git commit of their own module (see Week 1/2 action items — not yet done) | All | **Before Week 3** | Individual commit history, no single bulk push |
| Push repository to a hosted Git remote | Samuel Kojo Anafi Antwi | **Before Week 3** | Real GitHub/GitLab URL recorded in `docs/project_charter.md` |
| Confirm real instructor seed and regenerate all data/results if different | Samuel Kojo Anafi Antwi | Week 3 kickoff | Updated `team_config.yaml`, regenerated `results/` |

**Critical decision, resource, or lecturer support required:** same as Week 1 — seed/paper confirmation still outstanding.

## 7. Team Accountability Declaration

We confirm that this report accurately represents the meetings, work completed, individual contributions, challenges, and evidence produced during the reporting week.

Team Leader: _________________________________ Date: ______________
Team Members' Initials: _________________________________________
Evidence repository/link: https://github.com/samuelantwi103/CPEN438-Group3Project
