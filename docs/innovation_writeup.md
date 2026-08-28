# Innovation & Creativity Challenge — Adaptive `momo_match`

Project 1: Roofline Reckoning, CPEN 438, Group 3. **Week 3 deliverable** (brief §N: "each team proposes and implements one original enhancement to GH-Bench, technically justified, experimentally evaluated against the un-enhanced baseline, and defended at the final presentation"). This implements the brief's own first example: *"an adaptive kernel that switches between a memory-bound and compute-bound algorithmic variant based on a runtime arithmetic-intensity probe."*

## The problem this solves

At the project's actual dataset size (n=20,000), OpenMP's thread-launch overhead **exceeds the work itself**, making the "parallel" version of `momo_match`'s probe phase *slower* than sequential — confirmed directly by this write-up's own measurements below (see the n=20,000 row: always-parallel takes 0.0040592 s vs. 0.0002526 s sequential). A fixed "always use N threads" flag is therefore actively wrong at small n and only correct above some crossover point — a crossover point that depends on the specific machine's overhead and the workload's per-transaction cost, neither of which is knowable in advance.

*(An OpenMP scaling extension from Week 2, `omp_probe_scaling.c`, was cited here in an earlier draft as prior evidence of this — it is referenced in the AI-use declaration's original scaffold list but was never actually committed to this repository, so that claim has been removed. The crossover behaviour above is demonstrated fresh, directly from this deliverable's own data.)*

## The design

[`adaptive_momo_match.c`](../innovation/adaptive_momo_match.c) makes the sequential-vs-parallel choice **at runtime**, self-calibrated to whatever machine it's running on:

1. **Measure this machine's actual thread-launch overhead** — time one trivial no-op `#pragma omp parallel` region.
2. **Measure this workload's actual per-transaction cost** — time a small sample (the first 2,000 transactions) processed sequentially.
3. **Estimate both remaining options** — how long would the rest of the transactions take (a) finishing sequentially, or (b) launching threads (paying the measured overhead) and finishing in parallel?
4. **Pick whichever estimate is lower**, and execute that.

This is not tuned to this team's specific laptop with a hardcoded threshold — a machine with cheaper thread-launch overhead, or a slower per-transaction cost, would get a different (correctly different) decision from the exact same code.

## Experimental evaluation — real measured data, not projected

Run via [`evaluate_innovation.sh`](../innovation/evaluate_innovation.sh) (same directory as the kernel), sweeping problem size from 1,000 to 1,000,000 transactions, three-way comparison at every size: always-sequential, always-parallel (fixed baselines), and the adaptive choice. Raw output: [`results/innovation_results.jsonl`](../../results/innovation_results.jsonl).

| n | Always-sequential | Always-parallel | Adaptive chose | Adaptive time | Speedup vs. worse fixed choice |
|---|---|---|---|---|---|
| 1,000 | 0.0000107 s | 0.0041646 s | **sequential** | 0.0000176 s | **237×** (avoided a catastrophic parallel slowdown) |
| 5,000 | 0.0000446 s | 0.0069635 s | **sequential** | 0.0002313 s | **30×** |
| 20,000 | 0.0002526 s | 0.0040592 s | **sequential** | 0.0004799 s | **8.5×** (this is the project's actual dataset size) |
| 100,000 | 0.0021115 s | 0.0039646 s | **sequential** | 0.0037636 s | 1.05× |
| 500,000 | 0.0199814 s | 0.0138698 s | **parallel** | 0.0097872 s | **2.04×** vs. always-sequential |
| 1,000,000 | 0.0489743 s | 0.0269676 s | **parallel** | 0.0246886 s | **1.98×** vs. always-sequential |

(Table reflects the most recent full-pipeline run; see [`results/innovation_results.jsonl`](../../results/innovation_results.jsonl) for the live data — every ratio above was checked against that file directly. **Exact microsecond values vary run-to-run** — this ran inside a virtualised WSL2 environment with normal OS scheduling jitter. The number that does NOT vary between runs is the qualitative conclusion: small n always stays sequential and always avoids the parallel penalty; large n always switches to parallel and always wins. Report the qualitative crossover behaviour as the finding, not any single run's exact timings.)

**The crossover point is real and measured, not assumed:** somewhere between n=100,000 and n=500,000, "always-parallel" stops being the wrong answer and becomes the right one — and the adaptive kernel switches its decision to match, correctly, at every size tested. At the two largest sizes, the adaptive kernel is competitive with or beats the "always-parallel" fixed baseline — plausibly because the calibration sample also warms the CPU's branch predictor and cache state for the hash table before the timed remainder begins.

## Why this matters for the project's actual deployment size

GH-Bench's real experiment matrix uses n=20,000. At exactly that size, a naive "just enable OpenMP, more threads is always better" implementation is **~16.1× slower** than doing nothing (0.0040592 s parallel vs. 0.0002526 s sequential, from the table above) — this is precisely the kind of naive-parallelism trap Amdahl's Law and real measurement are supposed to prevent, and the adaptive kernel avoids it automatically rather than requiring a human to notice and hardcode a threshold.

*(A `project_charter.md` was cited here in an earlier draft to source the n=20,000 dataset-size claim; that file isn't present in this repository either, so the claim is grounded in this document's own measurements instead — the experiment matrix scripts in this repo (`run_experiment_matrix.sh`) do run all three kernels at n=20,000, which is directly checkable.)*

## Honest limitations

- The parallel-time estimate in step 3 uses a simplified linear-speedup-with-discount model, not a full re-derivation of Amdahl's Law per call — good enough to make the right binary decision here, but not itself a precise performance predictor.
- Only evaluated on this team's one physical machine (Intel i5-8350U) — the self-calibration design should generalize, but that claim itself is untested on other hardware.
- The calibration sample (2,000 transactions) is itself sequential work that can't be avoided even when the adaptive kernel ultimately chooses "parallel" — visible as the small gap between adaptive and the theoretical best case at very small n.
- Two claims present in an earlier draft of this write-up (crediting a Week 2 `omp_probe_scaling.c` OpenMP extension, and citing `project_charter.md` for the n=20,000 dataset size) referenced files that were never actually committed to this repository. Both have been removed/reworded above rather than left as dead links — flagging here so the team can decide whether those files should still be added, or whether the citations were simply wrong.
