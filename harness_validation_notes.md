# Harness Validation Notes — Hardware Counter Availability

Project 1: Roofline Reckoning, CPEN 315/733, Group 3.
Owner: Amponsah Jonathan Boadu (Hardware/Perf Lead)

The brief's §G requires cycle/time, cache-miss, and FLOP measurement "via perf or Cachegrind," and its Instructor Notes anticipate perf being unavailable in some environments, with Valgrind/Cachegrind as the documented fallback. Neither `perf` nor Valgrind exist on this team's native Windows/MinGW toolchain. Two things were checked and documented before deciding on an approach.

## 1. `perf_event_open` works inside WSL2 without root or the `perf` CLI

`perf_event_open(2)` is a Linux syscall, independent of whether the `perf` userspace tool is installed. A minimal C program calling it directly for `PERF_COUNT_HW_INSTRUCTIONS` succeeded on this team's WSL2 Ubuntu instance (Intel i5-8350U host), confirming the hypervisor passes through PMU access — this is not guaranteed on all WSL2/virtualised setups and should be re-verified on any machine this repository is cloned to (`gcc -O2 test_perf.c -o test_perf && ./test_perf`; success prints `perf_event_open SUCCESS`).

**Decision: build and run all C/C++ kernels and the harness inside WSL2**, not native Windows/MinGW, specifically to get real hardware counters rather than a timing-only fallback.

## 2. The fine-grained LLC-miss event silently read zero — substituted for the generic event

The first harness implementation used the detailed cache-event encoding `PERF_TYPE_HW_CACHE | (PERF_COUNT_HW_CACHE_LL) | (OP_READ<<8) | (RESULT_MISS<<16)`. The file descriptor opened successfully, but the counter never incremented (confirmed 0 across a cache-busting 64 MB stride-scan microbenchmark that should produce millions of misses).

Substituting the coarser, generic `PERF_TYPE_HARDWARE | PERF_COUNT_HW_CACHE_MISSES` event on the same microbenchmark produced a plausible non-zero count (4.1M misses out of 6.8M references on a 16M-element, stride-16 scan over a 4-array-touching loop). All subsequent harness runs use this generic event.

**This is a documented substitution, per the brief's explicit troubleshooting allowance**, made for a diagnosed reason (not a silent swap): the finer-grained event is not implemented/exposed correctly by this virtualised PMU path, while the generic hardware event is.

## Implication for report interpretation

"Cache misses" reported throughout this project are the generic `PERF_COUNT_HW_CACHE_MISSES` hardware event, not an LLC-specific breakdown. Where the report discusses cache behaviour, this should be stated explicitly (as it is here), and any claim about *which* cache level is missing should be treated as inference from access-pattern reasoning, not direct measurement.
