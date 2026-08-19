/*
 * harness.h — GH-Bench measurement harness (Project 1: Roofline Reckoning)
 * CPEN 315/733, Group 3
 *
 * Owner: Amponsah Jonathan Boadu (Hardware/Perf Lead)
 *
 * Thin, low-overhead wrapper around a kernel call that reports:
 *   - wall-clock time (clock_gettime(CLOCK_MONOTONIC))
 *   - user/system CPU time (getrusage), so CPU time can be isolated from
 *     wall-clock contention on a shared machine (see Instructor Notes,
 *     Project 1 brief)
 *   - hardware instructions, cycles and LLC cache misses, read directly via
 *     the perf_event_open(2) syscall — this build targets Linux/WSL2, where
 *     perf_event_open works without root and without the `perf` CLI being
 *     installed (verified on this team's dev machine: WSL2 Ubuntu on an
 *     Intel i5-8350U passes through the vPMU correctly).
 *
 * IMPORTANT — timed region discipline: the harness times ONLY the kernel
 * call itself. Do not put file I/O, log-writing or data generation inside
 * gh_harness_run(); a common student error (flagged explicitly in the
 * project brief, section M) is measuring wall-clock time that includes
 * disk I/O for log-writing inside the timed region.
 */
#ifndef GH_HARNESS_H
#define GH_HARNESS_H

#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Common kernel interface (Project 1 brief, section H: "input pointer,
 * size, output pointer, seed"). Each kernel additionally reports the
 * floating-point operation count and estimated bytes moved from memory,
 * which the harness cannot infer generically — these feed arithmetic
 * intensity (AI = FLOPs / bytes) for the Roofline plot. */
typedef struct {
    const void  *input;       /* kernel-specific input buffer               */
    size_t       n;           /* problem size (element count)               */
    void        *output;      /* kernel-specific output buffer              */
    unsigned int seed;        /* seed used to generate `input` (provenance) */
    uint64_t     flops;       /* OUT: floating-point ops performed          */
    uint64_t     bytes_moved; /* OUT: bytes read/written from/to memory     */
    double       phase1_seconds; /* OUT: optional sub-phase timing (e.g.
                                     momo_match's serial hash-table build),
                                     used for the Amdahl's-Law serial-
                                     fraction estimate. 0 if unused. */
    double       phase2_seconds; /* OUT: optional sub-phase timing (e.g.
                                     momo_match's parallelisable probe
                                     phase). 0 if unused. */
} gh_kernel_args_t;

typedef void (*gh_kernel_fn_t)(gh_kernel_args_t *args);

/* Result of one measured kernel invocation. */
typedef struct {
    double   wall_seconds;
    double   user_seconds;
    double   sys_seconds;
    uint64_t instructions;   /* 0 if hardware counters unavailable */
    uint64_t cycles;         /* 0 if hardware counters unavailable */
    uint64_t cache_refs;     /* LL cache references, 0 if unavailable */
    uint64_t cache_misses;   /* LL cache misses, 0 if unavailable */
    uint64_t flops;
    uint64_t bytes_moved;
    double   phase1_seconds;
    double   phase2_seconds;
    int      counters_valid; /* 1 if HW counters were successfully read */
} gh_result_t;

/* Opens the perf_event_open file descriptors for this process. Call once
 * at program start. Returns 0 on success, -1 if hardware counters are
 * unavailable (harness still works, counters_valid will be 0 and timing
 * still applies — this is the documented fallback path referenced in the
 * project brief's Instructor Notes for perf-unavailable environments). */
int gh_harness_init(void);

/* Runs `kernel(args)` exactly once, measuring only the call itself.
 * `args->flops` and `args->bytes_moved` must be set to 0 by the caller
 * before invocation; the kernel fills them in during the call. */
gh_result_t gh_harness_run(gh_kernel_fn_t kernel, gh_kernel_args_t *args);

/* Releases perf_event_open file descriptors. Call once at program end. */
void gh_harness_shutdown(void);

#ifdef __cplusplus
}
#endif

#endif /* GH_HARNESS_H */
