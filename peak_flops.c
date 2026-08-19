/*
 * peak_flops.c — achieved-peak compute microbenchmark: a tight,
 * register-resident FMA loop with no memory traffic once warmed into
 * cache, so it saturates compute throughput rather than bandwidth.
 * Provides the compute "roof" for the Roofline model (Project 1 brief,
 * section H — "measured peak GFLOP/s").
 *
 * Owner: Amponsah Jonathan Boadu (Hardware/Perf Lead)
 * Usage: ./peak_flops ITERATIONS
 */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _OPENMP
#include <omp.h>
#endif

/* Each call performs `unroll * 2` FLOPs (mul+add) per loop iteration on
 * data small enough to stay in registers/L1, repeated `iters` times. */
static double flop_loop(long iters) {
    double x0=1.0000001, x1=1.0000002, x2=1.0000003, x3=1.0000004;
    double x4=1.0000005, x5=1.0000006, x6=1.0000007, x7=1.0000008;
    const double m = 1.0000001;
    for (long i = 0; i < iters; i++) {
        x0 = x0 * m + m; x1 = x1 * m + m; x2 = x2 * m + m; x3 = x3 * m + m;
        x4 = x4 * m + m; x5 = x5 * m + m; x6 = x6 * m + m; x7 = x7 * m + m;
    }
    return x0+x1+x2+x3+x4+x5+x6+x7;
}

int main(int argc, char **argv) {
    long iters = (argc > 1) ? atol(argv[1]) : 200000000L;
    int nthreads = 1;
    double total = 0.0;

    struct timespec t0, t1;
    clock_gettime(CLOCK_MONOTONIC, &t0);
    #pragma omp parallel reduction(+:total)
    {
#ifdef _OPENMP
        #pragma omp single
        nthreads = omp_get_num_threads();
#endif
        total += flop_loop(iters);
    }
    clock_gettime(CLOCK_MONOTONIC, &t1);
    double dt = (t1.tv_sec - t0.tv_sec) + (t1.tv_nsec - t0.tv_nsec) / 1e9;

    double flops_per_thread = (double) iters * 8.0 * 2.0; /* 8 chains * (mul+add) */
    double total_flops = flops_per_thread * nthreads;
    double gflops = (total_flops / dt) / 1e9;

    printf("{\"benchmark\":\"peak_flops\",\"iters\":%ld,\"threads\":%d,"
           "\"seconds\":%.9f,\"gflops\":%.6f,\"checksum\":%.6f}\n",
           iters, nthreads, dt, gflops, total);
    return 0;
}
