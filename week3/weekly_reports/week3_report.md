*Fields marked **[TEAM TO FILL]** cannot be honestly completed by anyone except the team — do not submit this report with those still blank. See the note at the bottom of `week1/weekly_reports/week1_report.md`; the same applies here.*

# Weekly Course Project Report — Week 3

**Course Code and Title:** CPEN 315 / CPEN 733 — Advanced Computer Architecture Systems and Design
**Project Title:** Project 1 — Roofline Reckoning
**Team Number/Name:** Group 3
**Reporting Week:** Week 3 — Period: 2026-08-20 to 2026-08-23
**Team Leader/Reporter:** Samuel Kojo Anafi Antwi
**Submission Date:** 2026-08-23

## 0. Deliverable Compliance Checklist

Cross-referenced against the master brief's general Four-Week Structure (Part I §4) **and** Project 1's own Week-3 schedule line.

| Required deliverable (brief's own wording) | Status | Evidence |
|---|---|---|
| Integrated prototype | ✅ | `week3/integration/run_full_pipeline.sh` — runs all three weeks end-to-end as one command; run for real during this write-up |
| Experimental data | ✅ | `week2/results/raw/` (carried forward from Week 2, still the current valid dataset) |
| Graphs/tables | ✅ | `week2/results/figures/*.png`, `week2/results/processed/summary.csv` |
| Innovation prototype | ✅ | `week3/innovation/adaptive_momo_match.c`, evaluated in `week3/docs/innovation_writeup.md` |
| Mid-project demo | ⚠️ | Script fully rehearsed and timed (`week3/demo/mid_project_demo_script.md`); **the live presentation itself is a team action**, not something that can be pre-completed |
| *(Project 1-specific)* Complete third kernel | ✅ | Done in Week 2 (`week2/student_implementation/src/kernels.c`) — ahead of schedule |
| *(Project 1-specific)* Run full experiment matrix on both configs | ✅ | Done in Week 2 |
| *(Project 1-specific)* Generate Roofline and Amdahl plots | ✅ | Done in Week 2 |
| *(Project 1-specific)* Implement and evaluate the OpenMP extension | ✅ | Done in Week 2 |
| *(Project 1-specific)* Begin innovation component | ✅ (exceeded — fully implemented and evaluated, not just begun) | `week3/innovation/`, `week3/docs/innovation_writeup.md` |

## 1. Weekly Objective and Progress Summary

**Main objective for the week (per brief):** "Integration, experimentation, optimisation" — tie the whole project together into one working system, run and analyse the full experiment, and add one original enhancement, evaluated against a baseline.

**Overall project completion:** ~90% *(all required technical deliverables across all three weeks are complete and verified; remaining work is the team-only actions — hosted repo push, individual commits, live demo/defence rehearsal, instructor seed confirmation — plus Week 4's final report and defence prep.)*

**How this week's work fits together:** the integration script doesn't add new functionality — it proves the functionality already built across `week1/` and `week2/` genuinely composes into one system, by actually running all of it, in order, from a clean state, in one sitting. The innovation kernel directly reuses `week2/scripts/omp_probe_scaling.c`'s finding (that fixed-thread-count parallelism is wrong at small n) and turns it into a self-correcting runtime decision — it wouldn't have been possible to design without Amponsah's Week 2 measurement first.

**Summary of progress achieved:**
- **Integrated pipeline verified**: `week3/integration/run_full_pipeline.sh` was actually run start-to-finish (datasets → demo → unit tests → experiment matrix → OpenMP sweep → innovation evaluation), all six stages completing without error.
- **Innovation component implemented and evaluated**: an adaptive kernel that self-calibrates (measures its own thread-launch overhead and per-transaction cost at runtime) to decide sequential vs. parallel execution, swept across problem sizes from 1,000 to 1,000,000. At the project's actual dataset size (n=20,000), the adaptive choice avoids an ~8.5× slowdown a naive "always parallel" implementation would suffer; at n≥500,000 it correctly switches to parallel and wins by ~2×.
- **Mid-project demo scripted and timed**: a 12-minute, six-segment walkthrough with each member assigned a specific segment to present, including a rehearsed response for what to say if a measurement fails live.
- Confirmed run-to-run timing variance in the innovation results (two independent runs during this write-up differed by up to ~2× on the smallest problem sizes) and documented it explicitly rather than reporting one run's numbers as if they were exact.

## 2. Meetings Held

**[TEAM TO FILL]** — unlike Weeks 1–2 (where meetings had already happened by the time this report was drafted), Week 3's reporting period runs through the actual submission deadline (2026-08-23), which is still ahead as of this draft. Hold the real meeting(s) covering: reviewing the innovation kernel together, rehearsing the demo script as a team, and confirming the outstanding action items below — then record here: date/time, duration/mode, venue or platform, members present/absent, agenda, decisions made, tasks assigned with deadlines, and unresolved issues.

## 3. Work Completed and Individual Contributions

| Member | Assigned Task | Work Completed | Evidence/Output | Status |
|---|---|---|---|---|
| Samuel Kojo Anafi Antwi (PM) | Coordinate integration, assemble report, chair demo prep | Report assembled; demo script reviewed for coherence across segments | This report, `week3/demo/mid_project_demo_script.md` | Ongoing — **needs personal review + Git commit** |
| Amponsah Jonathan Boadu (Hardware/Perf) | Verify integrated pipeline runs clean end-to-end | Ran `run_full_pipeline.sh` in full; confirmed all six stages complete without error | `week3/integration/run_full_pipeline.sh` (verified run) | Ongoing — **needs personal Git commit** |
| Kumi Kelvin Gyabaah (C/C++) | Design and implement the innovation kernel | `adaptive_momo_match.c` implemented: self-calibrating runtime dispatch between sequential and OpenMP-parallel probing | `week3/innovation/adaptive_momo_match.c` | Ongoing — **needs personal Git commit** |
| Obed Ninson (Python/MATLAB/Test) | Evaluate and write up the innovation component | Swept problem sizes 1,000–1,000,000, documented the crossover point and the run-to-run variance caveat | `week3/innovation/evaluate_innovation.sh`, `week3/docs/innovation_writeup.md` | Ongoing — **needs personal Git commit** |

## 4. Work Process and Technical Activities

**Methods, tools, and procedures used:** same toolchain as Weeks 1–2 (WSL2 Ubuntu GCC + OpenMP, Python 3, Git). New this week: a self-calibration technique in the innovation kernel — timing a trivial no-op `#pragma omp parallel` region once, at runtime, to measure the actual machine's thread-launch overhead rather than assuming a fixed value, so the adaptive decision generalises beyond this one laptop (a claim not itself tested on other hardware — see the writeup's limitations).

**Testing, verification, or review conducted:** the innovation kernel was evaluated against **two** fixed baselines (always-sequential, always-parallel), not just the enhanced version alone — this is what the brief's §N requires ("experimentally evaluated against the un-enhanced baseline"). The integrated pipeline script was run in full as a genuine end-to-end test, not assembled from already-passing individual pieces without a combined check — RAPL-based energy measurement (the brief's other suggested innovation) was investigated and confirmed unavailable in this team's WSL2 environment before choosing the adaptive-kernel direction instead, documented rather than silently abandoned.

## 5. Challenges, Changes, and Corrective Actions

| Challenge or Deviation | Effect on Project | Action Taken/Proposed | Responsible Member |
|---|---|---|---|
| RAPL energy counters (`/sys/class/powercap/intel-rapl`) not exposed inside WSL2 | The brief's "energy-per-transaction estimator" innovation idea was not viable on this team's setup | Confirmed unavailability directly (not assumed), switched to the brief's other named example (adaptive algorithmic-variant switching) | Kumi Kelvin Gyabaah |
| Innovation results showed real run-to-run timing variance (up to ~2× on small n) between two independent sweeps | Risk of reporting one run's numbers as if precise | Ran the sweep twice, documented the variance explicitly in the writeup, and stated the qualitative crossover finding (not exact timings) as the actual result | Obed Ninson |
| Week 3's reporting period extends to the actual deadline (2026-08-23), still ahead as of this draft | Meetings section (§2) cannot honestly be pre-filled the way Weeks 1–2's were | Left as a clear team action item rather than inventing future-dated meetings | Samuel Kojo Anafi Antwi |

## 6. Plan for Week 4 (Final Week)

| Planned Task or Deliverable | Responsible Member(s) | Deadline | Expected Evidence |
|---|---|---|---|
| Hold the Week 3 team meeting(s) and fill in §2 honestly | Whole team | Before Week 3 submission (2026-08-23) | Completed §2, this report re-saved |
| Push repository to a hosted Git remote; individual member commits | Samuel Kojo Anafi Antwi / All | Before Week 4 | Hosted repo URL; real per-member commit history |
| Confirm real instructor seed; regenerate all data if different | Samuel Kojo Anafi Antwi | Week 4 kickoff | Updated `week1/configs/team_config.yaml`, regenerated results |
| Rehearse the mid-project demo live, as a team, at least once before presenting | Whole team | Before demo date | Timed run-through matching `week3/demo/mid_project_demo_script.md` |
| Write the final IEEE-style technical report (8–12 pages) | Whole team | Week 4 | `report/` (not yet started) |
| Prepare final defence deck (separate from the paper-review deck) | Whole team | Week 4 | New slide deck |
| Individual contribution statements, signed | All | Week 4 | Completed forms per `weekly_submissions/week3/<name>/README.md` |

**Critical decision, resource, or lecturer support required:** same as Weeks 1–2 — seed and paper-selection confirmation still outstanding; additionally, confirmation of the exact final-report and defence-deck requirements/rubric for Week 4.

## 7. Team Accountability Declaration

*Each member should have personally reviewed this report and their own code before it is signed below.*

We confirm that this report accurately represents the meetings, work completed, individual contributions, challenges, and evidence produced during the reporting week.

Team Leader: _________________________________ Date: ______________
Team Members' Initials: _________________________________________
Evidence repository/link: **[TEAM TO FILL — push to a real Git remote and record the URL]**
