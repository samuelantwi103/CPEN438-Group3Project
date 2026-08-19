/*
 * gh_bench.c — GH-Bench CLI driver
 * Project 1: Roofline Reckoning, CPEN 315/733, Group 3
 *
 * Owner: Kumi Kelvin Gyabaah (C/C++ Implementation Lead)
 *
 * Loads one of the three CSV datasets Obed's generators produce, runs the
 * matching kernel through Amponsah's harness for REPS+1 iterations (run 0
 * is a warm-up, matching demo_roofline.c's convention), and prints one
 * JSON line per run to stdout — same schema family as demo_results.jsonl
 * so the same analysis/plotting scripts can consume either.
 *
 * Usage:
 *   ./gh_bench momo     <momo_log.csv>     <reps> <seed>
 *   ./gh_bench cocobod  <cocobod_data.csv> <reps> <seed>
 *   ./gh_bench rainfall <rainfall_grid.csv><reps> <seed>
 */
#include "harness.h"
#include "kernels.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ---- dataset loaders --------------------------------------------------
 * Formats documented in datasets/gen_*.py. All three parsers skip/consume
 * their header line(s) as written by the generators. */

static int load_momo(const char *path, size_t *n_out, gh_momo_txn_t **sent_out,
                      gh_momo_txn_t **received_out) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen momo"); return -1; }

    char line[512];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return -1; } /* header */

    size_t cap = 1024, n = 0;
    gh_momo_txn_t *sent = malloc(cap * sizeof(gh_momo_txn_t));
    gh_momo_txn_t *received = malloc(cap * sizeof(gh_momo_txn_t));

    while (fgets(line, sizeof(line), f)) {
        if (n == cap) {
            cap *= 2;
            sent = realloc(sent, cap * sizeof(gh_momo_txn_t));
            received = realloc(received, cap * sizeof(gh_momo_txn_t));
        }
        unsigned long long sid, rid;
        double samt, ramt;
        unsigned int sts, rts;
        int got = sscanf(line, "%llu,%lf,%u,%llu,%lf,%u",
                          &sid, &samt, &sts, &rid, &ramt, &rts);
        if (got != 6) continue; /* skip malformed/blank lines */
        sent[n].id = sid; sent[n].amount = samt; sent[n].timestamp = sts;
        received[n].id = rid; received[n].amount = ramt; received[n].timestamp = rts;
        n++;
    }
    fclose(f);
    *n_out = n; *sent_out = sent; *received_out = received;
    return 0;
}

static int load_cocobod(const char *path, size_t *n_out, gh_cocobod_record_t **rec_out) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen cocobod"); return -1; }

    char line[512];
    if (!fgets(line, sizeof(line), f)) { fclose(f); return -1; } /* header */

    size_t cap = 1024, n = 0;
    gh_cocobod_record_t *rec = malloc(cap * sizeof(gh_cocobod_record_t));

    while (fgets(line, sizeof(line), f)) {
        if (n == cap) { cap *= 2; rec = realloc(rec, cap * sizeof(gh_cocobod_record_t)); }
        double rainfall, fert, yield;
        int got = sscanf(line, "%lf,%lf,%lf", &rainfall, &fert, &yield);
        if (got != 3) continue;
        rec[n].rainfall_mm = rainfall;
        rec[n].fertiliser_kg_ha = fert;
        rec[n].yield_tonnes_ha = yield;
        n++;
    }
    fclose(f);
    *n_out = n; *rec_out = rec;
    return 0;
}

static int load_rainfall(const char *path, int *rows_out, int *cols_out, double **grid_out) {
    FILE *f = fopen(path, "r");
    if (!f) { perror("fopen rainfall"); return -1; }

    int rows, cols;
    if (fscanf(f, "%d,%d\n", &rows, &cols) != 2) { fclose(f); return -1; }

    double *grid = malloc((size_t) rows * cols * sizeof(double));
    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            if (fscanf(f, "%lf,", &grid[r * cols + c]) != 1) {
                fclose(f); free(grid); return -1;
            }
        }
    }
    fclose(f);
    *rows_out = rows; *cols_out = cols; *grid_out = grid;
    return 0;
}

/* ---- shared run/print loop ---------------------------------------- */

static void print_result(const char *kernel_name, size_t n, int run, int warmup,
                          const gh_result_t *r) {
    double gflops = (r->wall_seconds > 0 && r->flops > 0)
                     ? (r->flops / r->wall_seconds) / 1e9 : 0.0;
    double ai = (r->bytes_moved > 0) ? (double) r->flops / (double) r->bytes_moved : 0.0;

    printf("{\"kernel\":\"%s\",\"n\":%zu,\"run\":%d,\"warmup\":%s,"
           "\"wall_seconds\":%.9f,\"user_seconds\":%.9f,\"sys_seconds\":%.9f,"
           "\"instructions\":%llu,\"cycles\":%llu,"
           "\"cache_refs\":%llu,\"cache_misses\":%llu,\"counters_valid\":%s,"
           "\"flops\":%llu,\"bytes_moved\":%llu,"
           "\"phase1_seconds\":%.9f,\"phase2_seconds\":%.9f,"
           "\"gflops\":%.9f,\"arithmetic_intensity\":%.6f}\n",
           kernel_name, n, run, warmup ? "true" : "false",
           r->wall_seconds, r->user_seconds, r->sys_seconds,
           (unsigned long long) r->instructions, (unsigned long long) r->cycles,
           (unsigned long long) r->cache_refs, (unsigned long long) r->cache_misses,
           r->counters_valid ? "true" : "false",
           (unsigned long long) r->flops, (unsigned long long) r->bytes_moved,
           r->phase1_seconds, r->phase2_seconds, gflops, ai);
}

static int run_reps(const char *name, gh_kernel_fn_t fn, gh_kernel_args_t *args,
                     size_t n, int reps) {
    for (int run = 0; run < reps + 1; run++) { /* run 0 = warm-up */
        gh_result_t r = gh_harness_run(fn, args);
        print_result(name, n, run, run == 0, &r);
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc < 5) {
        fprintf(stderr,
            "usage: %s <momo|cocobod|rainfall> <dataset.csv> <reps> <seed>\n",
            argv[0]);
        return 1;
    }
    const char *kernel = argv[1];
    const char *path = argv[2];
    int reps = atoi(argv[3]);
    unsigned int seed = (unsigned int) atoi(argv[4]);
    if (reps < 0) { fprintf(stderr, "reps must be >= 0\n"); return 1; }

    int have_counters = (gh_harness_init() == 0);
    if (!have_counters) {
        fprintf(stderr, "[gh_bench] running in timing-only mode\n");
    }

    int rc = 0;
    if (strcmp(kernel, "momo") == 0) {
        size_t n; gh_momo_txn_t *sent, *received;
        if (load_momo(path, &n, &sent, &received) != 0) { rc = 1; goto done; }
        gh_momo_input_t in = { .sent = sent, .received = received };
        uint8_t *out = malloc(n * sizeof(uint8_t));
        gh_kernel_args_t args = { .input = &in, .n = n, .output = out, .seed = seed };
        run_reps("momo_match", momo_match, &args, n, reps);
        free(sent); free(received); free(out);

    } else if (strcmp(kernel, "cocobod") == 0) {
        size_t n; gh_cocobod_record_t *rec;
        if (load_cocobod(path, &n, &rec) != 0) { rc = 1; goto done; }
        gh_cocobod_result_t result;
        result.residuals = malloc(n * sizeof(double));
        gh_kernel_args_t args = { .input = rec, .n = n, .output = &result, .seed = seed };
        run_reps("cocobod_yield_regression", cocobod_yield_regression, &args, n, reps);
        fprintf(stderr, "[gh_bench] fitted b0=%.6f b1=%.6f b2=%.6f R^2=%.6f\n",
                result.b0, result.b1, result.b2, result.r_squared);
        free(rec); free(result.residuals);

    } else if (strcmp(kernel, "rainfall") == 0) {
        int rows, cols; double *grid;
        if (load_rainfall(path, &rows, &cols, &grid) != 0) { rc = 1; goto done; }
        gh_rainfall_input_t in = { .grid = grid, .rows = rows, .cols = cols };
        size_t n = (size_t) rows * cols;
        double *out = malloc(n * sizeof(double));
        gh_kernel_args_t args = { .input = &in, .n = n, .output = out, .seed = seed };
        run_reps("rainfall_interpolate", rainfall_interpolate, &args, n, reps);
        free(grid); free(out);

    } else {
        fprintf(stderr, "unknown kernel '%s' (expected momo|cocobod|rainfall)\n", kernel);
        rc = 1;
    }

done:
    gh_harness_shutdown();
    return rc;
}
