*Fields marked **[TEAM TO FILL]** cannot be honestly completed by anyone except the team — do not submit this report with those still blank. See the note at the bottom.*

# Weekly Course Project Report — Week 1

**Course Code and Title:** CPEN 315 / CPEN 733 — Advanced Computer Architecture Systems and Design
**Project Title:** Project 1 — Roofline Reckoning
**Team Number/Name:** Group 3
**Reporting Week:** Week 1 — Period: 2026-08-19 to 2026-08-19 *(compressed to a single kickoff day; see note)*
**Team Leader/Reporter:** Samuel Kojo Anafi Antwi
**Submission Date:** 2026-08-19

## 0. Deliverable Compliance Checklist

Cross-referenced against the master brief's general Four-Week Structure (Part I §4) **and** Project 1's own Week-1 schedule line, so every required box is visibly accounted for — ✅ done, ⚠️ done but needs a team action to finalize, ❌ cannot be completed by anyone but the team (flagged, not faked).

| Required deliverable (brief's own wording) | Status | Evidence |
|---|---|---|
| Paper-review slides | ✅ | `presentation/paper_review_deck.pptx` — 10-slide structure, both papers, all 4 members assigned a presenting slide |
| Project charter | ✅ | `docs/project_charter.md` / `.docx` |
| Requirements spec | ✅ | `docs/requirements_spec.md` / `.docx` |
| Architecture diagram | ✅ | `docs/architecture_diagram.md` / `.docx` |
| Repo link | ⚠️ | Local Git repo initialized with real commit history; **not yet pushed to a hosted remote** — team action, see §6 |
| AI-use declaration | ✅ | `ai_use_declaration/ai_use_log.md` / `.docx` |
| *(Project 1-specific)* Present chosen papers | ⚠️ | Deck built and content-verified against the source PDFs; **live presentation is a team action**, not something that can be pre-completed |
| *(Project 1-specific)* Stand up repo | ✅ | `git init` done, 3 commits, `.gitignore` in place |
| *(Project 1-specific)* Run `demo_roofline.c` end-to-end | ✅ | `instructor_demo/c/demo_roofline.c`, real output in `results/raw/demo_results.jsonl` |
| *(Project 1-specific)* Problem statement scoped to seed + 2 hardware configs | ✅ | `docs/project_charter.md` §3–4 |
| *(Project 1-specific)* Draft architecture diagram of the harness | ✅ | `docs/architecture_diagram.md` |

## 1. Weekly Objective and Progress Summary

**Main objective for the week:** literature/paper selection, project charter, requirements specification, architecture diagram, repository scaffold, and a working end-to-end pipeline (per the brief's Week-1 schedule: "Present chosen papers; stand up repo; run demo_roofline.c end-to-end; write problem statement scoped to the team's assigned seed and two chosen hardware configurations; draft architecture diagram of the harness").

**Overall project completion:** ~35% *(Week 1 scope substantially done; portions of Week 2/3 scope — full three-kernel implementation, experiment matrix, plots — were also completed early to absorb the compressed Week 1+2 same-day deadline; see Week 2 report.)*

**How this week's work fits together (not four separate tracks):** every artefact below chains into the next person's work — Obed's dataset generators are the only valid input to Kumi's kernel interfaces; Amponsah's harness is what turns those kernels into trustworthy measurements; Samuel's charter/requirements/architecture documents are what make the resulting pipeline legible as one system rather than four people's separate outputs. See `docs/project_charter.md` §1 ("How the four roles form one system") for the full dependency chain.

**Summary of progress achieved:**
- Selected 2 of the 3 brief-assigned papers for review (Roofline 2009; SPEC CPU2017 2018) — all three papers obtained and verified (`docs/papers/`; first-page text cross-checked against title/authors/venue/DOI). Paper-review slide deck built (`presentation/paper_review_deck.pptx`, 10 slides), content drawn from the actual paper text.
- Repository scaffolded per the course's required structure (README, docs/, starter_code/, instructor_demo/, student_implementation/, configs/, datasets/, traces/, tests/, results/{raw,processed,figures}/, presentation/, report/, ai_use_declaration/).
- Level-1 demonstration pipeline (`instructor_demo/c/demo_roofline.c`, a vector dot-product kernel) built and run end-to-end with real hardware-counter measurements.
- Problem statement, project charter, requirements specification, and architecture diagram drafted (`docs/`).
- Provisional seed derived and recorded (`configs/team_config.yaml`) — **not yet instructor-confirmed**.
- Toolchain validated: `perf_event_open` confirmed working inside WSL2 without root/`perf` CLI; one hardware-counter substitution documented (`docs/harness_validation_notes.md`).

## 2. Meetings Held

**[TEAM TO FILL]** — no team meeting has been logged yet as of this draft. **Action: hold an actual kickoff meeting (even 20–30 minutes) before submitting this report**, and record: date/time, duration/mode, venue or platform, members present/absent, agenda, decisions made, tasks assigned with deadlines, and unresolved issues. Suggested agenda for that meeting: (a) confirm the papers and seed with the instructor, (b) walk through `docs/architecture_diagram.md` together so every member understands the pipeline, (c) assign the Week 2/3 action items in `ai_use_declaration/ai_use_log.md`.

## 3. Work Completed and Individual Contributions

| Member | Assigned Task | Work Completed | Evidence/Output | Status |
|---|---|---|---|---|
| Samuel Kojo Anafi Antwi (PM) | Repo scaffold, charter, schedule | Repository structure stood up; charter drafted | `docs/project_charter.md`, repo tree | Ongoing — **needs personal review + Git commit** |
| Amponsah Jonathan Boadu (Hardware/Perf) | Harness + counter wiring | `harness.h/.c` implemented and validated against a cache-busting microbenchmark | `student_implementation/src/harness.c`, `docs/harness_validation_notes.md` | Ongoing — **needs personal review + Git commit** |
| Kumi Kelvin Gyabaah (C/C++) | Kernel skeletons | All three kernels implemented (ahead of Week-2 schedule) and unit-tested | `student_implementation/src/kernels.c`, `tests/test_kernels.c` (16/16 passing) | Ongoing — **needs personal review + Git commit** |
| Obed Ninson (Python/MATLAB/Test) | Dataset generators, AI-use log setup | Three seeded generators written and run; AI-use declaration opened | `datasets/gen_*.py`, `ai_use_declaration/ai_use_log.md` | Ongoing — **needs personal review + Git commit** |

**Note on evidence:** all outputs above were actually compiled/executed (not hand-typed) — `gcc -O2 -Wall -Wextra`, zero warnings; unit tests 16/16 passing; `cocobod` OLS fit recovered known ground-truth coefficients within noise (R²=0.986). See `results/raw/` for raw run logs. **What is still missing is each team member's own review, understanding, and Git commit of their column** — see AI-use log action items.

## 4. Work Process and Technical Activities

**Methods, tools, and procedures used:** C11 (GCC 13.2 native Windows / GCC 15.2 in WSL2 Ubuntu), Python 3.13 (pandas/numpy/matplotlib/PyYAML), MATLAB R2024b (batch CLI), Git. Hardware counters via direct `perf_event_open(2)` syscalls (not the `perf` CLI, which is unavailable on this dev machine). `taskset` for the two-configuration core-count restriction (brief §C's own suggested substitution for a second physical machine).

**Testing, verification, or review conducted:** `test_kernels.c` — 16 hand-computable fixtures across all three kernels (see §3). One test-fixture bug was itself caught and fixed during this process (a cocobod test used collinear rainfall/fertiliser data, which correctly produced a singular-system 0-result from the kernel's determinant guard rather than a real regression bug) — documented in the test file's comments as a worked example of the difference between a kernel bug and a bad fixture.

## 5. Challenges, Changes, and Corrective Actions

| Challenge or Deviation | Effect on Project | Action Taken/Proposed | Responsible Member |
|---|---|---|---|
| No `perf` CLI or Valgrind on the Windows dev machine | Brief's primary counter-collection path unavailable | Moved C build to WSL2, used `perf_event_open` syscall directly; validated it works | Amponsah Jonathan Boadu |
| Fine-grained LLC-miss perf event silently read 0 | Cache-miss counts would have been wrong/misleading | Diagnosed with a cache-busting microbenchmark; substituted the generic `PERF_COUNT_HW_CACHE_MISSES` event; documented | Amponsah Jonathan Boadu |
| Seed not yet instructor-confirmed | All current datasets/results are provisional | Isolated to one config value (`configs/team_config.yaml`); one-command regeneration path exists | Samuel Kojo Anafi Antwi |
| Week 1 and Week 2 deliverables both due same day | Compressed timeline | Week 2 technical scope (full kernel implementation, experiment matrix) pulled forward into this same session | Whole team |

## 6. Plan for the Next Week (Week 2)

| Planned Task or Deliverable | Responsible Member(s) | Deadline | Expected Evidence |
|---|---|---|---|
| Confirm real seed + paper selection with instructor | Samuel Kojo Anafi Antwi | Week 2 kickoff | Instructor confirmation, updated `team_config.yaml` |
| Personal review + Git commit of own module | All | Before Week 2 checkpoint | Individual Git commits, not one bulk push |
| Manual CPU-time hand-calculation for a small input | Amponsah Jonathan Boadu | Week 2 | Written worked example, cross-checked against `roofline_model.m` |
| Extend unit tests to at least one case per member outside their own kernel | All | Week 2 | Updated `tests/test_kernels.c` |
| Rehearse the paper-review presentation (deck already built, see §0) — each member rehearses their assigned slide(s) | All | Week 2 | Timed run-through, 10–12 min + 5 min Q&A |
| Design specification (module-level design, complementing the architecture diagram) | Whole team, assembled by Obed Ninson | Week 2 | `docs/design_spec.md` |

**Critical decision, resource, or lecturer support required:** confirmation of the real assigned seed and whether the two brief-selected papers (Roofline, SPEC CPU2017) match what the instructor intends, or whether a different pairing/third paper is expected.

## 7. Team Accountability Declaration

**[TEAM TO FILL — do not submit until each member has personally reviewed this report and their own code.]**

We confirm that this report accurately represents the meetings, work completed, individual contributions, challenges, and evidence produced during the reporting week.

Team Leader: _________________________________ Date: ______________
Team Members' Initials: _________________________________________
Evidence repository/link: **[TEAM TO FILL — push to a real Git remote and record the URL]**

---
**Honesty note for the team:** this report's technical content (§1, §3, §4, §5) reflects work that was actually built and run, but §2 (meetings) and §7 (accountability) cannot be honestly filled in by anyone but you — the framework's integrity model depends on real meetings and real individual sign-off, not a filled-in template. Please do not submit this without holding the meeting and getting genuine sign-off first.
