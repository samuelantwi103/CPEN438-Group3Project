# Week 3 — Amponsah Jonathan Boadu (Computer Architecture & Hardware/Perf Lead)

## Your Week 3 role
Verify that the integrated pipeline — everything built across all three weeks, by all four of you — actually runs cleanly end-to-end as one system, not just as separately-working pieces.

## Files in this folder

| File | Canonical location | What it is |
|---|---|---|
| `run_full_pipeline.sh` | `gh-bench/week3/integration/run_full_pipeline.sh` | The six-stage script that runs the whole project from a clean state: datasets → Level-1 demo → unit tests → experiment matrix → OpenMP sweep → innovation evaluation |

## Run it (WSL2 Ubuntu)

```bash
bash run_full_pipeline.sh
```
Takes under a minute. Watch for all six `STAGE n/6` headers to print and for the final `PIPELINE COMPLETE` banner — if anything errors out partway, that's a real integration bug to fix, not something to work around.

## What you personally need to verify before the demo

- [ ] **Actually run this script yourself**, on your own checkout, not just read that it worked once. Integration bugs are exactly the kind of thing that "works on one machine" and silently doesn't elsewhere.
- [ ] Time how long the full run takes on your machine — the demo script assumes it's fast enough to run live during Segment 4; if it's slow on your setup, flag that to Samuel before the actual presentation.
- [ ] Be ready to explain, live, why the script cleans up (`rm`) each compiled binary after each stage — this connects to `.gitignore`'s policy of never tracking build artifacts.
- [ ] Re-confirm your Week 1/2 finding still holds: does `perf_event_open` still return real (non-zero) counters when run through this integrated script, not just standalone?

## Individual Contribution Form (fill in before submission)

- **Name & role:** Amponsah Jonathan Boadu — Hardware/Perf Lead
- **Specific artefacts I personally authored:** _______________________
- **Specific tests I personally ran:** _______________________
- **Specific results I personally interpreted:** _______________________
- **Git commit hashes attributable to me:** _______________________
- **Report section(s) I personally wrote:** _______________________
- **Live defence component I am prepared to present/defend:** _______________________
- **Estimated % of total team effort (self-assessed):** _______________________
