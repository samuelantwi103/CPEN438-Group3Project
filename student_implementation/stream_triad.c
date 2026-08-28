/*
 * stream_triad.c — STREAM-style Triad microbenchmark: measures achieved
 * memory bandwidth for a given configuration (core count via taskset,
 * thread count via OMP_NUM_THREADS). Provides the memory-bandwidth "roof"
 * for the Roofline model (Project 1 brief, section H).
 *
 * Triad: a[i] = b[i] + scalar * c[i]   (2 reads + 1 write per element,
 * 2 FLOPs per element -> AI = 2/24 FLOP/byte, deliberately far to the
 * memory-bound side, which is the point of a bandwidth benchmark).
 *
 * Owner: Amponsah Jonathan Boadu (Hardware/Perf Lead)
 * Usage: ./stream_triad N_ELEMENTS REPS
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _OPENMP
#include <omp.h>
#endif

int main(int argc, char **argv) {
    size_t n = (argc > 1) ? (size_t) atoll(argv[1]) : (1UL << 26); /* 64M doubles = 512MB/array */
    int reps = (argc > 2) ? atoi(argv[2]) : 10;

    double *a = malloc(n * sizeof(double));
    double *b = malloc(n * sizeof(double));
    double *c = malloc(n * sizeof(double));
    #pragma omp parallel for
    for (size_t i = 0; i < n; i++) { b[i] = 1.0; c[i] = 2.0; a[i] = 0.0; }

    double scalar = 3.0;
    double best_seconds = 1e18;
    for (int r = 0; r < reps; r++) {
        struct timespec t0, t1;
        clock_gettime(CLOCK_MONOTONIC, &t0);
        #pragma omp parallel for
        for (size_t i = 0; i < n; i++) {
            a[i] = b[i] + scalar * c[i];
        }
        clock_gettime(CLOCK_MONOTONIC, &t1);
        double dt = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;
        if (r > 0 && dt < best_seconds) best_seconds = dt; /* skip run 0 as warmup */
    }

    double bytes = 3.0 * (double) n * sizeof(double); /* 2 reads + 1 write */
    double gb_s = (bytes / best_seconds) / 1e9;
    int nthreads = 1;
#ifdef _OPENMP
    #pragma omp parallel
    { nthreads = omp_get_num_threads(); }
#endif
    printf("{\"benchmark\":\"stream_triad\",\"n\":%zu,\"threads\":%d,"
           "\"best_seconds\":%.9f,\"bandwidth_gbs\":%.6f}\n",
           n, nthreads, best_seconds, gb_s);

    free(a); free(b); free(c);
    return 0;
}
