# Week 3 — Obed Ninson (Python/MATLAB + Test/Docs Lead)

## Your Week 3 role
Evaluate Kumi's innovation kernel against real baselines, and write up the required technical justification (brief §N: "experimentally evaluated against the un-enhanced baseline").

## Files in this folder

| File | Canonical location | What it is |
|---|---|---|
| `evaluate_innovation.sh` | `gh-bench/week3/innovation/evaluate_innovation.sh` | Sweeps problem size (1,000 to 1,000,000), runs the adaptive kernel against both fixed baselines at each size |
| `innovation_results.jsonl` | `gh-bench/week3/results/innovation_results.jsonl` | The real measured output from that sweep |
| `innovation_writeup.md` / `.docx` | `gh-bench/week3/docs/innovation_writeup.md` | The write-up: the problem, the design, the results table, and an honest limitations section |

## Run it (WSL2 Ubuntu)

```bash
bash evaluate_innovation.sh
```

## What you personally need to verify before the demo

- [ ] **Re-run the sweep yourself** and compare your numbers to `innovation_results.jsonl` — the writeup explicitly notes run-to-run variance of up to ~2× on small problem sizes; confirm you get the same *qualitative* pattern (small n stays sequential, large n switches to parallel) even if exact numbers differ.
- [ ] Be ready to explain the "adaptive_speedup_vs_worst_choice" column precisely — it compares against whichever fixed baseline was worse at that size, which flips between "always-parallel" (at small n) and "always-sequential" (at large n). Make sure you can explain why that's the right comparison, not a cherry-picked one.
- [ ] Double-check the writeup's honesty note about variance is still accurate if you re-run and get different numbers — update the table if needed rather than leaving stale numbers in.
- [ ] Understand why RAPL (the brief's *other* suggested innovation) was ruled out, and be ready to explain that investigation, not just the kernel that was eventually built.

## Individual Contribution Form (fill in before submission)

- **Name & role:** Obed Ninson — Python/MATLAB + Test/Docs Lead
- **Specific artefacts I personally authored:** _______________________
- **Specific tests I personally ran:** _______________________
- **Specific results I personally interpreted:** _______________________
- **Git commit hashes attributable to me:** _______________________
- **Report section(s) I personally wrote:** _______________________
- **Live defence component I am prepared to present/defend:** _______________________ (suggestion: "Explain one point that misses the reported pattern." — matches the brief's own example checkpoint question style for this role)
- **Estimated % of total team effort (self-assessed):** _______________________
