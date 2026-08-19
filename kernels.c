/*
 * kernels.c — implementation of the three GH-Bench kernels declared in
 * kernels.h. See kernels.h for the data-layout contracts and the
 * expected Roofline placement of each kernel.
 *
 * Owner: Kumi Kelvin Gyabaah (C/C++ Implementation Lead)
 */
#include "kernels.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

/* =====================================================================
 * Kernel 1: momo_match
 *
 * Open-addressing hash table over `received`, keyed by transaction id,
 * so each `sent` lookup is expected O(1). This keeps the kernel's
 * dominant cost in pointer-chasing/branch-heavy probes rather than
 * floating-point work, matching the memory-bound expectation in
 * kernels.h.
 * ===================================================================*/

/* Table size: next power of two >= 2*n, so load factor stays <= 0.5
 * and linear-probe chains stay short. */
static size_t next_pow2(size_t v) {
    size_t p = 1;
    while (p < v) p <<= 1;
    return p;
}

static uint64_t hash_id(uint64_t id) {
    /* splitmix64 finalizer — cheap, good avalanche for sequential ids. */
    uint64_t x = id;
    x ^= x >> 30; x *= 0xbf58476d1ce4e5b9ULL;
    x ^= x >> 27; x *= 0x94d049bb133111ebULL;
    x ^= x >> 31;
    return x;
}

void momo_match(gh_kernel_args_t *args) {
    const gh_momo_input_t *in = (const gh_momo_input_t *) args->input;
    size_t n = args->n;
    uint8_t *out = (uint8_t *) args->output;

    uint64_t flops = 0;
    uint64_t bytes = 0;

    size_t cap = next_pow2(n * 2 > 0 ? n * 2 : 1);
    size_t mask = cap - 1;

    /* slot_used[i]: table slot occupied; slot holds an index into
     * in->received so we can read id/amount back for the compare. */
    int *slot_used = calloc(cap, sizeof(int));
    size_t *slot_idx = malloc(cap * sizeof(size_t));
    if (!slot_used || !slot_idx) {
        free(slot_used);
        free(slot_idx);
        return; /* allocation failure: leave output untouched */
    }

    /* ---- phase 1: build hash table over received[] ---- */
    for (size_t i = 0; i < n; i++) {
        uint64_t h = hash_id(in->received[i].id) & mask;
        while (slot_used[h]) h = (h + 1) & mask; /* linear probe */
        slot_used[h] = 1;
        slot_idx[h] = i;
        bytes += sizeof(gh_momo_txn_t); /* read received[i] */
    }

    /* ---- phase 2: probe with sent[] ---- */
    for (size_t i = 0; i < n; i++) {
        uint64_t id = in->sent[i].id;
        double amt = in->sent[i].amount;
        bytes += sizeof(gh_momo_txn_t); /* read sent[i] */

        uint64_t h = hash_id(id) & mask;
        uint8_t matched = 0;
        while (slot_used[h]) {
            size_t ri = slot_idx[h];
            if (in->received[ri].id == id) {
                double diff = amt - in->received[ri].amount;
                flops += 1; /* the subtraction */
                if (fabs(diff) < GH_MOMO_EPS) matched = 1;
                break; /* ids are unique in received[], stop on first hit */
            }
            h = (h + 1) & mask;
        }
        out[i] = matched;
        bytes += sizeof(uint8_t); /* write output[i] */
    }

    free(slot_used);
    free(slot_idx);

    args->flops = flops;
    args->bytes_moved = bytes;
}

/* =====================================================================
 * Kernel 2: cocobod_yield_regression
 *
 * Ordinary least squares for yield = b0 + b1*rainfall + b2*fertiliser,
 * fit via the 3x3 normal equations (X^T X) beta = X^T y, solved by
 * Gaussian elimination with partial pivoting. Reduction-heavy and
 * arithmetic-dense relative to bytes read, per kernels.h.
 * ===================================================================*/

/* Solve a 3x3 linear system A x = b in place (A is row-major 3x3,
 * b/x are length-3). Returns 0 on success, -1 if singular. */
static int solve3x3(double A[3][3], double b[3], double x[3]) {
    for (int col = 0; col < 3; col++) {
        int piv = col;
        double best = fabs(A[col][col]);
        for (int r = col + 1; r < 3; r++) {
            if (fabs(A[r][col]) > best) { best = fabs(A[r][col]); piv = r; }
        }
        if (best < 1e-12) return -1;
        if (piv != col) {
            for (int c = 0; c < 3; c++) { double t = A[col][c]; A[col][c] = A[piv][c]; A[piv][c] = t; }
            double t = b[col]; b[col] = b[piv]; b[piv] = t;
        }
        for (int r = col + 1; r < 3; r++) {
            double f = A[r][col] / A[col][col];
            for (int c = col; c < 3; c++) A[r][c] -= f * A[col][c];
            b[r] -= f * b[col];
        }
    }
    for (int r = 2; r >= 0; r--) {
        double s = b[r];
        for (int c = r + 1; c < 3; c++) s -= A[r][c] * x[c];
        x[r] = s / A[r][r];
    }
    return 0;
}

void cocobod_yield_regression(gh_kernel_args_t *args) {
    const gh_cocobod_record_t *rec = (const gh_cocobod_record_t *) args->input;
    size_t m = args->n;
    gh_cocobod_result_t *out = (gh_cocobod_result_t *) args->output;

    uint64_t flops = 0;
    uint64_t bytes = 0;

    /* ---- build normal equations: X^T X (3x3) and X^T y (3x1) ----
     * Row of X is [1, rainfall, fertiliser]. */
    double XtX[3][3] = {{0}};
    double Xty[3] = {0};

    for (size_t i = 0; i < m; i++) {
        double r = rec[i].rainfall_mm;
        double f = rec[i].fertiliser_kg_ha;
        double y = rec[i].yield_tonnes_ha;
        double row[3] = {1.0, r, f};

        for (int a = 0; a < 3; a++) {
            for (int b = 0; b < 3; b++) {
                XtX[a][b] += row[a] * row[b]; /* 1 mul + 1 add */
                flops += 2;
            }
            Xty[a] += row[a] * y; /* 1 mul + 1 add */
            flops += 2;
        }
        bytes += sizeof(gh_cocobod_record_t); /* read rec[i] */
    }

    double beta[3] = {0, 0, 0};
    solve3x3(XtX, Xty, beta);
    out->b0 = beta[0];
    out->b1 = beta[1];
    out->b2 = beta[2];

    /* ---- residuals + R^2 ---- */
    double y_sum = 0.0;
    for (size_t i = 0; i < m; i++) y_sum += rec[i].yield_tonnes_ha;
    double y_mean = (m > 0) ? y_sum / (double) m : 0.0;

    double ss_res = 0.0, ss_tot = 0.0;
    for (size_t i = 0; i < m; i++) {
        double y_hat = out->b0 + out->b1 * rec[i].rainfall_mm + out->b2 * rec[i].fertiliser_kg_ha;
        double resid = rec[i].yield_tonnes_ha - y_hat;
        out->residuals[i] = resid;
        ss_res += resid * resid;
        double dy = rec[i].yield_tonnes_ha - y_mean;
        ss_tot += dy * dy;
        flops += 8; /* 2 mul + 3 add for y_hat, 1 sub + 1 mul for resid^2, 1 sub+mul for dy^2 (approx) */
        bytes += sizeof(double); /* write residuals[i] */
    }
    out->r_squared = (ss_tot > 0.0) ? (1.0 - ss_res / ss_tot) : 1.0;

    args->flops = flops;
    args->bytes_moved = bytes;
}

/* =====================================================================
 * Kernel 3: rainfall_interpolate
 *
 * Inverse-distance-weighted interpolation over an expanding ring search
 * (irregular, pointer-chasing access pattern), per kernels.h.
 * ===================================================================*/

void rainfall_interpolate(gh_kernel_args_t *args) {
    const gh_rainfall_input_t *in = (const gh_rainfall_input_t *) args->input;
    int rows = in->rows, cols = in->cols;
    double *out = (double *) args->output;

    uint64_t flops = 0;
    uint64_t bytes = 0;

    for (int r = 0; r < rows; r++) {
        for (int c = 0; c < cols; c++) {
            double v = in->grid[r * cols + c];
            bytes += sizeof(double); /* read grid[r][c] */

            if (v != GH_RAIN_MISSING) {
                out[r * cols + c] = v;
                bytes += sizeof(double); /* write output[r][c] */
                continue;
            }

            /* Expanding ring search for up to GH_RAIN_MAX_NEIGHBOURS
             * valid cells, radius 1..GH_RAIN_MAX_RADIUS. */
            double wsum = 0.0, vsum = 0.0;
            int found = 0;
            for (int radius = 1; radius <= GH_RAIN_MAX_RADIUS && found < GH_RAIN_MAX_NEIGHBOURS; radius++) {
                for (int dr = -radius; dr <= radius && found < GH_RAIN_MAX_NEIGHBOURS; dr++) {
                    for (int dc = -radius; dc <= radius && found < GH_RAIN_MAX_NEIGHBOURS; dc++) {
                        /* only the ring boundary of this radius, so each
                         * cell is visited exactly once across radii */
                        if (abs(dr) != radius && abs(dc) != radius) continue;
                        int rr = r + dr, cc = c + dc;
                        if (rr < 0 || rr >= rows || cc < 0 || cc >= cols) continue;
                        double nv = in->grid[rr * cols + cc];
                        bytes += sizeof(double); /* read candidate neighbour */
                        if (nv == GH_RAIN_MISSING) continue;

                        double dist2 = (double) (dr * dr + dc * dc);
                        double w = 1.0 / dist2; /* 1 div */
                        wsum += w;
                        vsum += w * nv; /* 1 mul + 1 add */
                        flops += 3;
                        found++;
                    }
                }
            }

            double filled = (found > 0) ? (vsum / wsum) : 0.0; /* fallback: no valid neighbour in radius */
            if (found > 0) flops += 1; /* final division */
            out[r * cols + c] = filled;
            bytes += sizeof(double); /* write output[r][c] */
        }
    }

    args->flops = flops;
    args->bytes_moved = bytes;
}
