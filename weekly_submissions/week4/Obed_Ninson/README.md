# Week 4 (Final) — Obed Ninson (Python/MATLAB + Test/Docs Lead)

## Your Week 4 role
Finish the analytical-model validation — the brief's own Week-4 line for this project. This is the week where a real complication came up: MATLAB itself became unavailable.

## Files in this folder

| File | Canonical location | What it is |
|---|---|---|
| `final_validation_notes.md` / `.docx` | `gh-bench/week4/docs/final_validation_notes.md` | The final CPU-time/Roofline/Amdahl re-validation against current data, including the MATLAB-unavailability disclosure |
| `individual_contribution_Obed_Ninson.md` / `.docx` | `gh-bench/week4/forms/individual_contribution_Obed_Ninson.md` | Your contribution statement — self-report fields still need filling |
| `peer_assessment_by_Obed_Ninson.md` / `.docx` | `gh-bench/week4/forms/peer_assessment_by_Obed_Ninson.md` | Your confidential rating of your three teammates — entirely blank, fill privately |

## What you personally need to do before submission — this one matters most

- [ ] **Check whether MATLAB is actually available again on your own machine.** If it is, run `week2/student_implementation/roofline_model.m` for real and confirm it produces numbers matching what `final_validation_notes.md` reports from the Python reproduction. If they don't match, that's a real finding to investigate and correct, not something to paper over.
- [ ] Re-derive, by hand, at least one row of the CPU-time validation table in `final_validation_notes.md` — confirm the Python reproduction's formula matches `roofline_model.m`'s own documented formula exactly (see that file's comments), not just approximately.
- [ ] Read the honesty note about the `cocobod_yield_regression` ceiling percentage changing between runs (124.7% → 71.5%) and be ready to explain it live — this is exactly the kind of number a sharp defence question would probe.
- [ ] Fill in your individual contribution form's self-report fields.
- [ ] Complete your peer assessment privately.
- [ ] Be ready for the brief's own defence question for this role: *"Explain one point that misses your model's tolerance band."*

## Individual Contribution Form
See `individual_contribution_Obed_Ninson.md` in this folder.
