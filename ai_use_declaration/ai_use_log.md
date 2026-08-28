# AI-Use Declaration — GH-Bench (Project 1: Roofline Reckoning)

CPEN 438, Group 3. Maintained per the course integrity framework (master brief §7, §8). One row per instance of AI use. **This log must be kept honest and updated weekly — do not delete or retroactively edit entries.**

## Summary (read this first)

A substantial fraction of this repository's first version — the harness, the three kernels, the Python analysis/plotting scripts, the MATLAB model, the dataset generators, and this documentation set — was produced by an AI assistant (Claude, Anthropic, in Claude Code) at team member Samuel Kojo Anafi Antwi's (rkaantwi@st.ug.edu.gh) direction, working from the course's project-portfolio brief. This is **more than the "permitted uses" listed in the framework** (clarifying a concept, explaining an error, suggesting test cases, improving documentation/grammar) and needs to be disclosed as such, not minimized.

**What this means for the team, concretely:**
- Every member must personally read, run, and be able to explain — unaided, live, from memory — every file attributed to their role in `configs/team_config.yaml`, before Week 2's checkpoint. The course's weekly checkpoint explicitly includes "Instructor selects ONE team member at random for an individual oral question... or a live code/circuit walkthrough" and spot-checks Git history for distributed, meaningful authorship.
- The measured results (hardware counters, timings, plots) are **real** — they came from actually compiling and running this code on the team's own machine via WSL2/perf_event_open, not fabricated — but the code that produced them was AI-authored, and the team is responsible for verifying correctness, not just trusting it.
- Before submission, discuss with the instructor whether this level of AI involvement is acceptable for this course's integrity policy, and how the team should present this honestly at the defence (the Bloom's-taxonomy "Create" question — "If you had one more week, what would you change?" — and the "Evaluate" question both assume genuine understanding, not recitation).

## Log

| AI tool | Date | Prompt/query (summary) | Output received (summary) | How verified | What was modified | Final section(s) containing AI-assisted material |
|---|---|---|---|---|---|---|
| Claude (Anthropic, Claude Code) | 2026-08-19 | "Read the course project-portfolio brief and weekly report template; scrutinize and do Project 1 (Roofline Reckoning) for Group 3, covering Week 1–3 deliverables." | Full repository scaffold: `harness.h/.c`, `kernels.h/.c`, `gh_bench.c`, three Python dataset generators, `analyze.py`, `plots.py`, `roofline_model.m`, `stream_triad.c`, `peak_flops.c`, `omp_probe_scaling.c`, `configs/team_config.yaml`, this documentation set. | Code was compiled (`gcc -O2 -Wall -Wextra`, zero warnings) and actually executed inside WSL2 against real hardware counters (`perf_event_open`); the OLS regression kernel's fitted coefficients were checked against the generator's known ground-truth coefficients (recovered within noise, R²=0.986); the Roofline/Amdahl plots were visually inspected against the brief's expected trends (§M) and matched (rainfall memory-bound, COCOBOD closer to ridge point). **Not yet verified:** line-by-line read-through by each team member; unit-test suite is still being written; no member has yet done a live, unaided walkthrough. | N/A (initial authorship, not a modification) | `student_implementation/`, `datasets/gen_*.py`, `scripts/`, `configs/team_config.yaml`, `docs/`, this file | All of Weeks 1–2's code and documentation deliverables; the experimental data itself is real, measured output, not AI-fabricated |

**Action items before Week 2 checkpoint (Project Manager to track):**
1. Each member reads and annotates (with their own comments) the files matching their `team_config.yaml` role.
2. Each member independently re-derives, by hand, the CPU-time/Amdahl worked example for their own kernel (brief §F pattern), and compares to `roofline_model.m`'s output.
3. Fill in the correctness unit tests in `tests/` — currently scaffolded by the same AI session; each member should extend at least one test for a kernel outside their primary ownership, to build cross-team understanding.
4. Commit history must show each member's own commits from their own machine/account — see `docs/project_charter.md` §2 note on this.
