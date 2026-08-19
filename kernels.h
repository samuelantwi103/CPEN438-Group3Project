/*
 * kernels.h — GH-Bench kernel signatures (Project 1: Roofline Reckoning)
 * CPEN 315/733, Group 3
 *
 * Owner: Kumi Kelvin Gyabaah (C/C++ Implementation Lead)
 *
 * Three architecturally distinct Ghanaian-context kernels, each conforming
 * to the common gh_kernel_fn_t signature from harness.h (input pointer,
 * size, output pointer, seed). Data layouts are documented per kernel so
 * gh_bench.c and the Python generators agree on the wire format.
 */
#ifndef GH_KERNELS_H
#define GH_KERNELS_H

#include "harness.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Kernel 1: MoMo transaction matching -------------------------------
 * Models settlement reconciliation between a "sent" log and a "received"
 * log for a mobile-money network: for every sent transaction, determine
 * whether a matching received transaction exists (same id, amount equal
 * within GH_MOMO_EPS). Hash-join style: integer/branch-heavy, low
 * arithmetic intensity, small working set per lookup -> expected close to
 * the memory-bound side for large n, but with a meaningfully different
 * access pattern from the interpolation kernel (hash probes, not stencil).
 *
 * args->input  -> const gh_momo_input_t*, with args->n = number of sent
 *                 transactions (== number of received transactions).
 * args->output -> uint8_t[args->n], 1 if sent[i] matched, 0 otherwise.
 */
#define GH_MOMO_EPS 1e-6

typedef struct {
    uint64_t id;
    double   amount;
    uint32_t timestamp;
} gh_momo_txn_t;

typedef struct {
    const gh_momo_txn_t *sent;      /* size n */
    const gh_momo_txn_t *received;  /* size n */
} gh_momo_input_t;

void momo_match(gh_kernel_args_t *args);

/* ---- Kernel 2: COCOBOD yield regression --------------------------------
 * Fits yield_hat = b0 + b1*rainfall + b2*fertiliser by ordinary least
 * squares (normal equations over m regional records), then computes
 * predicted yield and residual for every record. Reduction-heavy,
 * arithmetic-dense relative to bytes read -> expected closer to /
 * right of the Roofline ridge point (compute-bound side).
 *
 * args->input  -> const gh_cocobod_record_t*, args->n = m records.
 * args->output -> gh_cocobod_result_t (single struct: fitted coefficients,
 *                 R^2, and a residuals[m] array owned by the caller).
 */
typedef struct {
    double rainfall_mm;
    double fertiliser_kg_ha;
    double yield_tonnes_ha;
} gh_cocobod_record_t;

typedef struct {
    double b0, b1, b2;     /* fitted OLS coefficients */
    double r_squared;
    double *residuals;     /* caller-allocated, size n */
} gh_cocobod_result_t;

void cocobod_yield_regression(gh_kernel_args_t *args);

/* ---- Kernel 3: Volta-basin rainfall interpolation -----------------------
 * A rows x cols raster with some cells marked missing (GH_RAIN_MISSING
 * sentinel). Every missing cell is filled by inverse-distance-weighted
 * (IDW) interpolation over up to K valid neighbours found by an expanding
 * ring search (irregular, pointer-chasing access pattern) -> expected
 * memory-bound / left of the Roofline ridge point.
 *
 * args->input  -> const gh_rainfall_input_t*, args->n = rows * cols.
 * args->output -> double[rows*cols], the filled grid (valid cells copied
 *                 through unchanged).
 */
#define GH_RAIN_MISSING -9999.0
#define GH_RAIN_MAX_NEIGHBOURS 8
#define GH_RAIN_MAX_RADIUS 6

typedef struct {
    const double *grid; /* rows*cols, row-major, GH_RAIN_MISSING = missing */
    int rows;
    int cols;
} gh_rainfall_input_t;

void rainfall_interpolate(gh_kernel_args_t *args);

#ifdef __cplusplus
}
#endif

#endif /* GH_KERNELS_H */
