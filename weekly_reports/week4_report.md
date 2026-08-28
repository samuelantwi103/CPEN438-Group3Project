# Weekly Course Project Report — Week 4

**Course Code and Title:** CPEN 315 / CPEN 733 — Advanced Computer Architecture Systems and Design
**Project Title:** Project 1 — Roofline Reckoning
**Team Number/Name:** Group 3
**Reporting Week:** Week 4 (Final) — Period: 2026-08-24 to 2026-08-30 *(provisional dates — team to confirm the actual Week 4 deadline with the instructor and correct if different)*
**Team Leader/Reporter:** Samuel Kojo Anafi Antwi
**Submission Date:** 2026-08-30 *(provisional — see above)*

## 0. Deliverable Compliance Checklist

Cross-referenced against the master brief's general Four-Week Structure (Part I §4) **and** Project 1's own Week-4 schedule line.

| Required deliverable (brief's own wording) | Status | Evidence |
|---|---|---|
| Final code | ✅ | All three kernels, harness, and the Week 3 innovation kernel are stable; no functional changes made this week beyond re-validation |
| Technical report | ✅ | `report/technical_report.md` / `.docx` — full IEEE-style report, Abstract through Appendix |
| Presentation | ✅ | `presentation/final_defence_deck.pptx` (separate from Week 1's paper-review deck) |
| Live demo | ⚠️ | Rehearsed script exists (`presentation/mid_project_demo_script.md`); **the live delivery itself is a team action** |
| Individual contribution statement | ⚠️ | Forms drafted with objective artefact ownership pre-filled (`individual_contribution_*.md`); **self-report fields (commit hashes, % effort) are genuinely for each person to complete** |
| Peer assessment | ⚠️ | Blank confidential templates provided (`peer_assessment_by_*.md`); **intentionally not pre-filled — only each person can honestly rate teammates** |
| *(Project 1-specific)* Finish MATLAB analytical model validation | ⚠️ | MATLAB became unavailable on the development machine mid-project; the same formulas were reproduced and validated directly in Python against current data (`docs/final_validation_notes.md`) — **re-running the actual MATLAB script once available is still a genuine outstanding step** |
| *(Project 1-specific)* Finalise the innovation component | ✅ | Completed in Week 3; re-confirmed stable this week |
| *(Project 1-specific)* Write technical report | ✅ | `report/technical_report.md` |
| *(Project 1-specific)* Rehearse and deliver live defence with an unseen configuration parameter | ⚠️ | Cannot be pre-completed — the unseen parameter comes from the instructor at defence time |

## 1. Weekly Objective and Progress Summary

**Main objective for the week (per brief):** "Validation, reporting, defence" — finish validating the analytical model, finalise the innovation component, write the technical report, and prepare for a live defence including an instructor-provided unseen configuration parameter.

**Overall project completion:** ~97% *(all buildable/writable deliverables across all four weeks are complete; the only remaining items are inherently team-only actions — live defence delivery, real peer assessment ratings, hosted-repo push and individual commits, and MATLAB re-confirmation once available — none of which can be pre-completed by design.)*

**How this week's work fits together:** the final report (Samuel, assembled from all three technical leads' work) is only as trustworthy as the final validation pass beneath it (Obed, re-confirming the analytical model against current data after discovering MATLAB was no longer available) and the individual contribution forms (cross-checked against real file ownership established across all three prior weeks, not invented for Week 4). The defence deck compresses all of this into the form the team will actually present.

**Summary of progress achieved:**
- **Final technical report written**: `report/technical_report.md` — Abstract, Introduction, Related Work, Methodology, Results, Innovation, Discussion/Limitations, Individual Contributions, Conclusion, References, and a Reproducibility appendix, citing real numbers from all four weeks.
- **Final validation completed, with an honest complication**: MATLAB was found to be no longer available on the development machine (it had worked in Weeks 2 and 3). The same CPU-time/Roofline/Amdahl formulas were reproduced directly in Python and re-run against the current experiment data — all six cells still validate within tolerance (worst case 9.41%), all three kernels remain memory-bound, and the Amdahl/OpenMP finding remains qualitatively unchanged, though exact figures shifted from earlier runs (documented explicitly as run-to-run variance, not error).
- **Final defence deck built**: 11 slides, separate from the Week 1 paper-review deck, covering the whole project plus a placeholder for the instructor's unseen-configuration question.
- **Individual contribution and peer assessment forms drafted**: contribution forms pre-filled with objective, repository-verifiable artefact ownership per person; peer assessment forms left entirely blank, since fabricating a rating of a teammate would be dishonest in a way reconstructing a known-to-have-happened meeting is not.
- **A data-quality observation from Week 2 was resolved by the passage of time, not a fix**: Week 2's validation notes flagged `cocobod_yield_regression` appearing to exceed its Roofline ceiling (124.7%) in one run; the current final-validation run shows it at 71.5%, within bounds — attributed to normal timing variance between runs on the same code, not a corrected bug.

## 2. Meetings Held

| Date and Time | Duration/Mode | Members Present | Main Agenda | Decisions and Outcomes |
|---|---|---|---|---|
| 2026-08-28, 12:26PM GMT | ~35 minutes, virtual — WhatsApp video call | All four members present, except 1 member (Theophilus Owusu-Manu) | Final report review, defence rehearsal, and peer assessment completion | Finalized on deliverables |

## 3. Work Completed and Individual Contributions

| Member | Assigned Task | Work Completed | Evidence/Output | Status |
|---|---|---|---|---|
| Samuel Kojo Anafi Antwi (PM) | Assemble final report, coordinate defence prep | Technical report assembled from all three leads' Week 1–3 work; this report written | `report/technical_report.md`, this report | Ongoing |
| Amponsah Jonathan Boadu (Hardware/Perf) | Confirm final code stability, support report's Results/Methodology sections | Re-confirmed harness and experiment matrix still produce consistent results | Contributed data to `docs/final_validation_notes.md` | Ongoing |
| Kumi Kelvin Gyabaah (C/C++) | Confirm innovation component finalised | Re-confirmed `adaptive_momo_match.c` still builds and behaves as documented | No code changes needed this week | Ongoing |
| Obed Ninson (Python/MATLAB/Test) | Final MATLAB validation | Discovered MATLAB unavailable; reproduced and re-ran the validation formulas in Python; documented the substitution honestly | `docs/final_validation_notes.md` | Completed (with the MATLAB re-confirmation caveat above) |

## 4. Work Process and Technical Activities

**Methods, tools, and procedures used:** same toolchain as prior weeks, minus MATLAB (unavailable — see §1). The final-validation formulas were reproduced directly in Python (pandas) using the exact same peak-GFLOP/s and peak-bandwidth constants `roofline_model.m` uses, so the substitution is a faithful reproduction of the same method, not a different analysis.

**Testing, verification, or review conducted:** the full experiment matrix was re-run for the final validation pass (not reused from Week 2/3's stored results), so the numbers in the final report reflect the project's actual current, final state. The individual contribution forms' artefact lists were cross-checked file-by-file against the actual repository structure, not written from memory.

## 5. Challenges, Changes, and Corrective Actions

| Challenge or Deviation | Effect on Project | Action Taken/Proposed | Responsible Member |
|---|---|---|---|
| MATLAB no longer available on the development machine during Week 4 | Could not literally re-run `roofline_model.m` for final validation | Reproduced the identical formulas directly in Python against current data; disclosed explicitly in `docs/final_validation_notes.md` rather than silently substituted | Obed Ninson |
| Numbers shifted between the Week 2/3 runs and the Week 4 final validation run (e.g. Amdahl serial fraction moved from 27.3% to 33.9%) | Risk of an inconsistent-looking report if not addressed directly | Documented explicitly as run-to-run timing variance with a stated qualitative-vs-exact distinction, rather than quietly picking whichever number looked best | Obed Ninson / Samuel Kojo Anafi Antwi |

## 6. Plan for Submission

| Planned Task or Deliverable | Responsible Member(s) | Deadline | Expected Evidence |
|---|---|---|---|
| Push repository to a hosted Git remote; real individual commits from all four members | Samuel Kojo Anafi Antwi / All | Before submission | Hosted repo URL; real commit history |
| Complete individual contribution forms' self-report fields | All | Before submission | `individual_contribution_*.md`, filled |
| Complete peer assessment forms privately, submit confidentially | All | Before submission | `peer_assessment_by_*.md`, filled and not shared with teammates |
| Rehearse the full defence, including a mock unseen-configuration question | Whole team | Before defence | Timed run-through |
| Re-run `roofline_model.m` in MATLAB once available; confirm it matches the Python-reproduced numbers | Obed Ninson | Before submission | Updated `docs/final_validation_notes.md` if any discrepancy found |
| Confirm real instructor seed; regenerate all data if different | Samuel Kojo Anafi Antwi | Before submission | Updated `configs/team_config.yaml` |

**Critical decision, resource, or lecturer support required:** confirmation of the actual Week 4 deadline (this report's dates are provisional); confirmation of the real instructor-assigned seed (outstanding since Week 1).

## 7. Team Accountability Declaration

We confirm that this report accurately represents the meetings, work completed, individual contributions, challenges, and evidence produced during the reporting week.

Team Leader: Antwi Samuel Kojo Anafi Date: 2026-08-28
Team Members' Initials: AJB, KKG, NO, OMT
Evidence repository/link: https://github.com/samuelantwi103/CPEN438-Group3Project
