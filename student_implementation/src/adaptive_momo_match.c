/*
 * adaptive_momo_match.c — Week 3 Innovation & Creativity Challenge
 * Project 1: Roofline Reckoning, CPEN 438, Group 3
 *
 * Brief §N names two example innovations; this implements the first:
 * "an adaptive kernel that switches between a memory-bound and
 * compute-bound algorithmic variant based on a runtime arithmetic-
 * intensity probe." Here the "variant switch" is sequential vs.
 * OpenMP-parallel probing for momo_match's probe phase (week2's
 * omp_probe_scaling.c measured that at small n, OpenMP's thread-launch
 * overhead exceeds the work itself and makes parallel SLOWER, not
 * faster — this program fixes that by deciding at runtime instead of
 * via a fixed flag).
 *
 * How the runtime probe works (self-calibrating, not a hardcoded
 * threshold tuned to one machine):
 *   1. Time a trivial "#pragma omp parallel" no-op region once, to
 *      measure THIS machine's actual thread-launch overhead.
 *   2. Time processing a small sample of the real workload (the first
 *      SAMPLE_SIZE transactions) sequentially, to measure THIS
 *      workload's per-transaction cost on THIS machine.
 *   3. Estimate: would finishing the remaining (n - SAMPLE_SIZE)
 *      transactions sequentially cost more than the parallel overhead
 *      plus a parallel-estimated remaining cost? If yes, go parallel;
 *      if no, stay sequential.
 *
 * Compared against two fixed baselines (always-sequential,
 * always-parallel) across a sweep of problem sizes, so the
 * crossover point where the adaptive choice actually helps is visible
 * in the data, not just claimed.
 *
 * Usage: ./adaptive_momo_match INPUT.csv >> results.jsonl
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <omp.h>

#define GH_MOMO_EPS 1e-6
#define SAMPLE_SIZE 2000

typedef struct { uint64_t id; double amount; uint32_t timestamp; } txn_t;

static double now_seconds(void) {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (double) ts.tv_sec + (double) ts.tv_nsec / 1e9;
}

static uint64_t hash_u64(uint64_t x) {
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

static size_t next_pow2(size_t x) { size_t p = 8; while (p < x) p <<= 1; return p; }

/* --- data loading (same CSV format as gh_bench.c's load_momo) --- */
static void load_momo(const char *path, txn_t **sent_out, txn_t **recv_out, size_t *n_out) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen"); exit(1); }
    char line[512];
    size_t cap = 1024, n = 0;
    txn_t *sent = malloc(cap * sizeof(txn_t));
    txn_t *recv = malloc(cap * sizeof(txn_t));
    if (!fgets(line, sizeof(line), f)) { fprintf(stderr, "empty input\n"); exit(1); }
    while (fgets(line, sizeof(line), f)) {
        if (n == cap) { cap *= 2; sent = realloc(sent, cap*sizeof(txn_t)); recv = realloc(recv, cap*sizeof(txn_t)); }
        unsigned long long sid, rid; double samt, ramt; unsigned int sts, rts;
        if (sscanf(line, "%llu,%lf,%u,%llu,%lf,%u", &sid,&samt,&sts,&rid,&ramt,&rts) != 6) continue;
        sent[n].id=sid; sent[n].amount=samt; sent[n].timestamp=sts;
        recv[n].id=rid; recv[n].amount=ramt; recv[n].timestamp=rts;
        n++;
    }
    fclose(f);
    *sent_out = sent; *recv_out = recv; *n_out = n;
}

/* --- hash table build (sequential, same as kernels.c's momo_match) --- */
static size_t *build_table(const txn_t *recv, size_t n, size_t *table_size_out) {
    size_t table_size = next_pow2(n * 2);
    size_t mask = table_size - 1;
    size_t *table = malloc(table_size * sizeof(size_t));
    for (size_t i = 0; i < table_size; i++) table[i] = SIZE_MAX;
    for (size_t i = 0; i < n; i++) {
        uint64_t h = hash_u64(recv[i].id) & mask;
        while (table[h] != SIZE_MAX) h = (h + 1) & mask;
        table[h] = i;
    }
    *table_size_out = table_size;
    return table;
}

static inline int probe_one(const txn_t *sent, const txn_t *recv, const size_t *table, size_t mask, size_t i) {
    uint64_t h = hash_u64(sent[i].id) & mask;
    while (table[h] != SIZE_MAX) {
        size_t j = table[h];
        if (recv[j].id == sent[i].id) {
            double delta = recv[j].amount - sent[i].amount;
            return fabs(delta) < GH_MOMO_EPS;
        }
        h = (h + 1) & mask;
    }
    return 0;
}

/* Sequential probe over [start, end). */
static void probe_seq(const txn_t *sent, const txn_t *recv, const size_t *table, size_t mask,
                       uint8_t *matched, size_t start, size_t end) {
    for (size_t i = start; i < end; i++) matched[i] = (uint8_t) probe_one(sent, recv, table, mask, i);
}

/* Parallel probe over [start, end). */
static void probe_parallel(const txn_t *sent, const txn_t *recv, const size_t *table, size_t mask,
                            uint8_t *matched, size_t start, size_t end) {
    #pragma omp parallel for schedule(static)
    for (size_t i = start; i < end; i++) matched[i] = (uint8_t) probe_one(sent, recv, table, mask, i);
}

/* --- one no-op timed parallel region: measures THIS run's thread-launch overhead --- */
static double measure_omp_overhead(void) {
    double t0 = now_seconds();
    #pragma omp parallel
    { volatile int x = 0; (void) x; }
    return now_seconds() - t0;
}

int main(int argc, char **argv) {
    if (argc < 2) { fprintf(stderr, "usage: %s INPUT.csv\n", argv[0]); return 1; }
    txn_t *sent, *recv; size_t n;
    load_momo(argv[1], &sent, &recv, &n);

    size_t table_size; size_t *table = build_table(recv, n, &table_size);
    size_t mask = table_size - 1;
    uint8_t *matched = malloc(n * sizeof(uint8_t));

    /* ---- Baseline 1: always sequential ---- */
    double t0 = now_seconds();
    probe_seq(sent, recv, table, mask, matched, 0, n);
    double baseline_seq_s = now_seconds() - t0;

    /* ---- Baseline 2: always parallel ---- */
    t0 = now_seconds();
    probe_parallel(sent, recv, table, mask, matched, 0, n);
    double baseline_par_s = now_seconds() - t0;

    /* ---- Adaptive: self-calibrating runtime probe ---- */
    double overhead_s = measure_omp_overhead();

    size_t sample = (n < SAMPLE_SIZE) ? n : SAMPLE_SIZE;
    double ts0 = now_seconds();
    probe_seq(sent, recv, table, mask, matched, 0, sample);
    double sample_s = now_seconds() - ts0;
    double per_txn_seq_s = (sample > 0) ? sample_s / (double) sample : 0.0;

    size_t remaining = n - sample;
    double est_seq_remaining_s = per_txn_seq_s * (double) remaining;
    /* crude parallel-throughput estimate: assume linear speedup capped at
     * available threads, discounted 50% for realism (matches the
     * sub-linear speedup actually measured in week2/scripts/omp_probe_scaling.c) */
    int nthreads = omp_get_max_threads();
    double est_par_remaining_s = overhead_s + (per_txn_seq_s * (double) remaining) / (nthreads * 0.5);

    int chose_parallel = (est_par_remaining_s < est_seq_remaining_s);

    double tr0 = now_seconds();
    if (chose_parallel) probe_parallel(sent, recv, table, mask, matched, sample, n);
    else                probe_seq(sent, recv, table, mask, matched, sample, n);
    double remaining_actual_s = now_seconds() - tr0;
    double adaptive_total_s = sample_s + remaining_actual_s;

    printf("{\"n\":%zu,\"nthreads\":%d,"
           "\"baseline_seq_s\":%.9f,\"baseline_parallel_s\":%.9f,"
           "\"omp_overhead_s\":%.9f,\"sample_size\":%zu,\"per_txn_seq_ns\":%.3f,"
           "\"est_seq_remaining_s\":%.9f,\"est_parallel_remaining_s\":%.9f,"
           "\"adaptive_chose\":\"%s\",\"adaptive_total_s\":%.9f,"
           "\"adaptive_speedup_vs_seq\":%.4f,\"adaptive_speedup_vs_worst_choice\":%.4f}\n",
           n, nthreads, baseline_seq_s, baseline_par_s,
           overhead_s, sample, per_txn_seq_s * 1e9,
           est_seq_remaining_s, est_par_remaining_s,
           chose_parallel ? "parallel" : "sequential", adaptive_total_s,
           baseline_seq_s / adaptive_total_s,
           (baseline_seq_s > baseline_par_s ? baseline_seq_s : baseline_par_s) / adaptive_total_s);

    free(sent); free(recv); free(table); free(matched);
    return 0;
}
