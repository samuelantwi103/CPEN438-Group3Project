# Mid-Project Demo Script

Project 1: Roofline Reckoning, CPEN 438, Group 3. **Week 3 deliverable** (brief §4: "mid-project demo"). Rehearse this exact sequence before presenting — every command below has been run for real (see `week3/integration/run_full_pipeline.sh`, which is this script's automated form), so nothing here should surprise you live.

**Total time budget: ~12 minutes.** Practice with a timer.

## Before you start (do this the night before, not live)

- [ ] Confirm you're in WSL2 Ubuntu, at the `gh-bench/` repo root.
- [ ] Run `bash week3/integration/run_full_pipeline.sh` once, privately, so you know it works on the machine you'll actually demo on.
- [ ] Have `week2/results/figures/*.png` open in an image viewer, ready to switch to.
- [ ] Know which of you is presenting which segment (see assignments below).

## Segment 1 — What GH-Bench is (1 min) — Samuel

"We're measuring how three Ghana-context programs perform on two hardware setups, using the CPU-time equation, Amdahl's Law, and the Roofline model — not estimating, measuring, with real hardware counters." Point at `week1/docs/architecture_diagram.md`'s diagram on screen.

## Segment 2 — Prove the measurement tool is trustworthy (2 min) — Amponsah

Live-run the Level-1 demo:
```bash
cd week1/instructor_demo/c
gcc -O2 -Wall -Wextra -std=c11 demo_roofline.c -o demo_roofline
./demo_roofline 2000000 5 829488
rm demo_roofline
```
Point out: real instruction/cycle counts from the CPU's own hardware counters, arithmetic intensity comes out to exactly 0.125 — the mathematically correct value for a double-precision dot product, hand-verifiable on the spot.

## Segment 3 — The three real kernels pass correctness testing (2 min) — Kumi

```bash
bash week2/tests/run_tests.sh
```
Let it print all 20 `PASS` lines and the `ALL TESTS PASSED` line. Say what one or two of the tests actually check (e.g. "this one verifies a transaction matches only within our epsilon tolerance, not just any close number").

## Segment 4 — The real experiment, live (3 min) — Amponsah

```bash
bash week2/scripts/run_experiment_matrix.sh
```
While it runs (a few seconds), explain: 3 kernels × 2 hardware configs × 6 runs = 36 real measurements, on real hardware counters, `taskset`-restricting cores to fake the "two machines" comparison.

## Segment 5 — What the data shows (2 min) — Obed

Switch to the pre-generated plots (`week2/results/figures/`). Show the Roofline plot: point at where `rainfall_interpolate` sits (memory-bound, left of the ridge) vs `cocobod_yield_regression` (closer to the ridge). Show the Amdahl plot: explain the measured 27%/73% serial/parallel split for `momo_match`.

## Segment 6 — The Week 3 innovation, live (2 min) — Kumi + Amponsah

```bash
bash week3/innovation/evaluate_innovation.sh
```
While the small sizes run near-instantly, explain: this kernel decides for itself, at runtime, whether using multiple threads is worth it — and point at the n=1,000 and n=20,000 rows in the output where it correctly stays sequential (avoiding an ~8–355× slowdown a naive "always parallel" version would suffer), then the n=500,000/1,000,000 rows where it correctly switches to parallel and wins.

## If something goes wrong live

- **`perf_event_open` fails / counters read 0:** say so directly — "this is the exact failure mode we documented in `week1/docs/harness_validation_notes.md`; here's our fallback." Do not pretend it's fine.
- **A number differs from what's in the report:** say "we're seeing normal run-to-run timing variance — the report notes this explicitly for the innovation results; the qualitative pattern is what's being claimed, not the exact microsecond value."
- **Anyone asks "why does perf show different cycles than clock-rate math predicts":** this is the brief's own example checkpoint question for the Hardware/Perf role — Amponsah should have `docs/manual_cpu_time_worked_example.md`'s Turbo Boost explanation ready.
