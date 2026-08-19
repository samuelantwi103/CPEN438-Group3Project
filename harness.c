/*
 * harness.c — implementation. See harness.h for the contract.
 * Owner: Amponsah Jonathan Boadu (Hardware/Perf Lead)
 */
#define _GNU_SOURCE
#include "harness.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/resource.h>

#if defined(__linux__)
#include <sys/syscall.h>
#include <sys/ioctl.h>
#include <linux/perf_event.h>
#include <asm/unistd.h>
#define GH_HAVE_PERF 1
#endif

#ifdef GH_HAVE_PERF
static int fd_instructions = -1;
static int fd_cycles       = -1;
static int fd_cache_refs   = -1;
static int fd_cache_miss   = -1;

static long perf_event_open(struct perf_event_attr *hw_event, pid_t pid,
                             int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, hw_event, pid, cpu, group_fd, flags);
}

static int open_hw_counter(uint32_t type, uint64_t config) {
    struct perf_event_attr pe;
    memset(&pe, 0, sizeof(pe));
    pe.type = type;
    pe.size = sizeof(pe);
    pe.config = config;
    pe.disabled = 1;
    pe.exclude_kernel = 1;
    pe.exclude_hv = 1;
    pe.exclude_idle = 1;
    return (int) perf_event_open(&pe, 0 /* self */, -1 /* any cpu */, -1, 0);
}
#endif

int gh_harness_init(void) {
#ifdef GH_HAVE_PERF
    fd_instructions = open_hw_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_INSTRUCTIONS);
    fd_cycles       = open_hw_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CPU_CYCLES);
    fd_cache_refs   = open_hw_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_REFERENCES);
    /* Generic hardware cache-miss event (PERF_TYPE_HARDWARE domain). We
     * tried the finer-grained PERF_TYPE_HW_CACHE | LL | READ | MISS
     * encoding first, but it silently reads 0 under this team's WSL2
     * virtualised PMU (confirmed with a cache-busting microbenchmark:
     * fd opens successfully but the counter never increments) — the
     * generic PERF_COUNT_HW_CACHE_MISSES event does increment correctly
     * on the same test, so we use it instead. Documented substitution
     * per the project brief's perf-unavailable troubleshooting note. */
    fd_cache_miss   = open_hw_counter(PERF_TYPE_HARDWARE, PERF_COUNT_HW_CACHE_MISSES);

    if (fd_instructions == -1 || fd_cycles == -1) {
        fprintf(stderr,
            "[harness] WARNING: perf_event_open unavailable "
            "(kernel/paranoid/VM restriction). Falling back to timing-only "
            "measurement; instructions/cycles/cache-misses will read 0. "
            "This substitution is documented per the project brief's "
            "Instructor Notes (perf-unavailable troubleshooting).\n");
        gh_harness_shutdown();
        return -1;
    }
    return 0;
#else
    fprintf(stderr, "[harness] Non-Linux build: hardware counters unavailable, "
                     "timing-only mode.\n");
    return -1;
#endif
}

gh_result_t gh_harness_run(gh_kernel_fn_t kernel, gh_kernel_args_t *args) {
    gh_result_t r;
    memset(&r, 0, sizeof(r));

    args->flops = 0;
    args->bytes_moved = 0;
    args->phase1_seconds = 0;
    args->phase2_seconds = 0;

    struct rusage ru_before, ru_after;
    struct timespec t_before, t_after;

#ifdef GH_HAVE_PERF
    int have_counters = (fd_instructions != -1 && fd_cycles != -1);
    if (have_counters) {
        ioctl(fd_instructions, PERF_EVENT_IOC_RESET, 0);
        ioctl(fd_cycles, PERF_EVENT_IOC_RESET, 0);
        if (fd_cache_refs != -1) ioctl(fd_cache_refs, PERF_EVENT_IOC_RESET, 0);
        if (fd_cache_miss != -1) ioctl(fd_cache_miss, PERF_EVENT_IOC_RESET, 0);
        ioctl(fd_instructions, PERF_EVENT_IOC_ENABLE, 0);
        ioctl(fd_cycles, PERF_EVENT_IOC_ENABLE, 0);
        if (fd_cache_refs != -1) ioctl(fd_cache_refs, PERF_EVENT_IOC_ENABLE, 0);
        if (fd_cache_miss != -1) ioctl(fd_cache_miss, PERF_EVENT_IOC_ENABLE, 0);
    }
#endif

    getrusage(RUSAGE_SELF, &ru_before);
    clock_gettime(CLOCK_MONOTONIC, &t_before);

    /* ---- timed region: kernel call only ---- */
    kernel(args);
    /* ----------------------------------------- */

    clock_gettime(CLOCK_MONOTONIC, &t_after);
    getrusage(RUSAGE_SELF, &ru_after);

#ifdef GH_HAVE_PERF
    if (have_counters) {
        ioctl(fd_instructions, PERF_EVENT_IOC_DISABLE, 0);
        ioctl(fd_cycles, PERF_EVENT_IOC_DISABLE, 0);
        if (fd_cache_refs != -1) ioctl(fd_cache_refs, PERF_EVENT_IOC_DISABLE, 0);
        if (fd_cache_miss != -1) ioctl(fd_cache_miss, PERF_EVENT_IOC_DISABLE, 0);

        uint64_t v;
        if (read(fd_instructions, &v, sizeof(v)) == sizeof(v)) r.instructions = v;
        if (read(fd_cycles, &v, sizeof(v)) == sizeof(v)) r.cycles = v;
        if (fd_cache_refs != -1 && read(fd_cache_refs, &v, sizeof(v)) == sizeof(v)) r.cache_refs = v;
        if (fd_cache_miss != -1 && read(fd_cache_miss, &v, sizeof(v)) == sizeof(v)) r.cache_misses = v;
        r.counters_valid = 1;
    }
#endif

    r.wall_seconds = (t_after.tv_sec - t_before.tv_sec)
                   + (t_after.tv_nsec - t_before.tv_nsec) / 1e9;
    r.user_seconds = (ru_after.ru_utime.tv_sec - ru_before.ru_utime.tv_sec)
                    + (ru_after.ru_utime.tv_usec - ru_before.ru_utime.tv_usec) / 1e6;
    r.sys_seconds  = (ru_after.ru_stime.tv_sec - ru_before.ru_stime.tv_sec)
                    + (ru_after.ru_stime.tv_usec - ru_before.ru_stime.tv_usec) / 1e6;

    r.flops = args->flops;
    r.bytes_moved = args->bytes_moved;
    r.phase1_seconds = args->phase1_seconds;
    r.phase2_seconds = args->phase2_seconds;
    return r;
}

void gh_harness_shutdown(void) {
#ifdef GH_HAVE_PERF
    if (fd_instructions != -1) { close(fd_instructions); fd_instructions = -1; }
    if (fd_cycles       != -1) { close(fd_cycles);       fd_cycles = -1; }
    if (fd_cache_refs   != -1) { close(fd_cache_refs);   fd_cache_refs = -1; }
    if (fd_cache_miss   != -1) { close(fd_cache_miss);   fd_cache_miss = -1; }
#endif
}
