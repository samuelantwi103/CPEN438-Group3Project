# Individual Contribution Statement — Obed Ninson

Project 1: Roofline Reckoning, CPEN 315/733, Group 3. Role: Python/MATLAB + Test/Docs Lead.

| Field | Content |
|---|---|
| Name & role | Obed Ninson — Python/MATLAB + Test/Docs Lead |
| Specific code/hardware artefacts personally authored | `datasets/gen_momo_log.py`, `gen_cocobod_data.py`, `gen_rainfall_grid.py`, `ai_use_declaration/ai_use_log.md`, `student_implementation/analyze.py`, `plots.py`, `roofline_model.m`, `docs/manual_cpu_time_worked_example.md`, `docs/matlab_validation_notes.md`, `student_implementation/evaluate_innovation.sh`, `docs/innovation_writeup.md`, `docs/final_validation_notes.md` |
| Specific tests personally written or run | Wrote and ran `evaluate_innovation.sh` (Week 3), sweeping problem size from 1,000 to 1,000,000 and producing `innovation_results.jsonl`. Personally re-ran the CPU-time/Roofline/Amdahl validation checks in Week 4 (Python reproduction of `roofline_model.m`'s formulas) against the current `results/processed/summary.csv`, covering all six kernel×config cells. Also worked the manual CPU-time calculation by hand in `docs/manual_cpu_time_worked_example.md`. |
| Specific results personally interpreted | Personally derived the ~59% Turbo Boost deviation in `manual_cpu_time_worked_example.md` (using the machine's nominal 1.70 GHz clock as an independent input, versus the self-consistent effective-clock check), and personally flagged/explained the cocobod-ceiling anomaly (124.7% of ceiling in the earlier Week 2 run vs. 71.5% in the final Week 4 run) in `final_validation_notes.md`, attributing it to run-to-run timing variance rather than a data error. |
| Git commit hashes attributable to me | `a213329` (seeded dataset generators), `d46f8f3` (AI-use declaration log), `6e4dec4` (innovation kernel evaluation sweep), `c1f18cb` (innovation results write-up), `ae5e920`/`d5ad9a8` (Week 3 submission packet), plus this week's `docs/final_validation_notes.md`, `individual_contribution_Obed_Ninson.md`, `peer_assessment_by_Obed_Ninson.md` commits. |
| Section(s) of the final report personally written | The validation/methodology section covering the CPU-time equation check, Roofline ceiling check, and Amdahl's Law projection — and the honesty note explaining the MATLAB-unavailability workaround and the cocobod-ceiling change between runs (source content in `docs/final_validation_notes.md`). |
| Live defence component prepared to present/defend | *"Explain one point that misses your model's tolerance band."* Prepared answer: no cell actually misses tolerance in the final pass (worst case is 9.41% on `cocobod_yield_regression` at configA, against a ±15% bound) — ready to explain why that cell is the closest to the edge (shortest wall-clock time of the six kernels, so most sensitive to fixed measurement overhead) and to walk through the cocobod ceiling-percentage change (124.7%→71.5%) as a run-to-run variance story, not a bug fix. |
| Estimated % of total team effort (self-assessed) | 90% — reflects the level of personal dedication and effort put into every task assigned to me, not a share of the team's combined output. |

Signature: Obed Ninson Date: 2026-08-28
