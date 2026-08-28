/*
 * test_kernels.c — correctness unit tests for kernels.c
 * Project 1: Roofline Reckoning, CPEN 438, Group 3
 *
 * Owner: Kumi Kelvin Gyabaah (C/C++ Implementation Lead)
 *
 * No external test framework: each test is a function returning 1 (pass)
 * or 0 (fail) with a printed reason on failure. run_tests.sh compiles and
 * runs this binary and checks its exit code.
 */
#include "../harness.h"
#include "../kernels.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_pass = 0, g_fail = 0;

#define CHECK(cond, msg) do { \
    if (!(cond)) { printf("    FAIL: %s\n", msg); return 0; } \
} while (0)

#define RUN(fn) do { \
    printf("  %-42s ", #fn); \
    if (fn()) { printf("PASS\n"); g_pass++; } \
    else { g_fail++; } \
} while (0)

/* ===================== momo_match tests ===================== */

static gh_kernel_args_t make_momo_args(gh_momo_input_t *in, size_t n, uint8_t *out) {
    gh_kernel_args_t a = { .input = in, .n = n, .output = out, .seed = 1,
                            .flops = 0, .bytes_moved = 0 };
    return a;
}

static int test_momo_exact_match(void) {
    gh_momo_txn_t sent[1]     = {{ .id = 1, .amount = 100.0, .timestamp = 0 }};
    gh_momo_txn_t received[1] = {{ .id = 1, .amount = 100.0, .timestamp = 5 }};
    gh_momo_input_t in = { .sent = sent, .received = received };
    uint8_t out[1] = {0};
    gh_kernel_args_t a = make_momo_args(&in, 1, out);
    momo_match(&a);
    CHECK(out[0] == 1, "exact id+amount match should set 1");
    return 1;
}

static int test_momo_amount_beyond_eps(void) {
    gh_momo_txn_t sent[1]     = {{ .id = 1, .amount = 100.0, .timestamp = 0 }};
    gh_momo_txn_t received[1] = {{ .id = 1, .amount = 100.5, .timestamp = 5 }};
    gh_momo_input_t in = { .sent = sent, .received = received };
    uint8_t out[1] = {0};
    gh_kernel_args_t a = make_momo_args(&in, 1, out);
    momo_match(&a);
    CHECK(out[0] == 0, "amount diff beyond GH_MOMO_EPS should not match");
    return 1;
}

static int test_momo_amount_within_eps(void) {
    gh_momo_txn_t sent[1]     = {{ .id = 1, .amount = 100.0, .timestamp = 0 }};
    gh_momo_txn_t received[1] = {{ .id = 1, .amount = 100.0 + GH_MOMO_EPS / 10.0, .timestamp = 5 }};
    gh_momo_input_t in = { .sent = sent, .received = received };
    uint8_t out[1] = {0};
    gh_kernel_args_t a = make_momo_args(&in, 1, out);
    momo_match(&a);
    CHECK(out[0] == 1, "amount diff within GH_MOMO_EPS should match");
    return 1;
}

static int test_momo_id_not_present(void) {
    gh_momo_txn_t sent[1]     = {{ .id = 1, .amount = 100.0, .timestamp = 0 }};
    gh_momo_txn_t received[1] = {{ .id = 2, .amount = 100.0, .timestamp = 5 }};
    gh_momo_input_t in = { .sent = sent, .received = received };
    uint8_t out[1] = {0};
    gh_kernel_args_t a = make_momo_args(&in, 1, out);
    momo_match(&a);
    CHECK(out[0] == 0, "id absent from received[] should not match");
    return 1;
}

static int test_momo_mixed_batch(void) {
    gh_momo_txn_t sent[4] = {
        { .id = 1, .amount = 10.0, .timestamp = 0 },
        { .id = 2, .amount = 20.0, .timestamp = 0 },
        { .id = 3, .amount = 30.0, .timestamp = 0 },
        { .id = 4, .amount = 40.0, .timestamp = 0 },
    };
    gh_momo_txn_t received[4] = {
        { .id = 1, .amount = 10.0, .timestamp = 5 },  /* match */
        { .id = 2, .amount = 99.0, .timestamp = 5 },  /* amount mismatch */
        { .id = 9, .amount = 30.0, .timestamp = 5 },  /* id mismatch */
        { .id = 4, .amount = 40.0, .timestamp = 5 },  /* match */
    };
    gh_momo_input_t in = { .sent = sent, .received = received };
    uint8_t out[4] = {0};
    gh_kernel_args_t a = make_momo_args(&in, 4, out);
    momo_match(&a);
    CHECK(out[0] == 1 && out[1] == 0 && out[2] == 0 && out[3] == 1,
          "mixed batch should match only truly-matching pairs");
    return 1;
}

static int test_momo_reports_bytes_moved(void) {
    gh_momo_txn_t sent[2]     = {{1,10.0,0},{2,20.0,0}};
    gh_momo_txn_t received[2] = {{1,10.0,5},{2,20.0,5}};
    gh_momo_input_t in = { .sent = sent, .received = received };
    uint8_t out[2] = {0};
    gh_kernel_args_t a = make_momo_args(&in, 2, out);
    momo_match(&a);
    CHECK(a.bytes_moved > 0, "kernel must report nonzero bytes_moved for n>0");
    return 1;
}

/* ===================== cocobod_yield_regression tests ===================== */

static int test_cocobod_recovers_noiseless_line(void) {
    /* yield = 1.0 + 0.002*rainfall + 0.005*fert, exactly, no noise. */
    const size_t m = 6;
    gh_cocobod_record_t rec[6];
    double rainfalls[6] = {800, 1000, 1200, 1500, 1800, 2200};
    double ferts[6]     = {0, 50, 100, 200, 300, 400};
    for (size_t i = 0; i < m; i++) {
        rec[i].rainfall_mm = rainfalls[i];
        rec[i].fertiliser_kg_ha = ferts[i];
        rec[i].yield_tonnes_ha = 1.0 + 0.002 * rainfalls[i] + 0.005 * ferts[i];
    }
    gh_cocobod_result_t result;
    double residuals[6];
    result.residuals = residuals;
    gh_kernel_args_t a = { .input = rec, .n = m, .output = &result, .seed = 1 };
    cocobod_yield_regression(&a);

    CHECK(fabs(result.b0 - 1.0) < 1e-6, "b0 should recover to 1.0");
    CHECK(fabs(result.b1 - 0.002) < 1e-6, "b1 should recover to 0.002");
    CHECK(fabs(result.b2 - 0.005) < 1e-6, "b2 should recover to 0.005");
    return 1;
}

static int test_cocobod_perfect_fit_r_squared_is_one(void) {
    const size_t m = 5;
    gh_cocobod_record_t rec[5];
    for (size_t i = 0; i < m; i++) {
        double r = 900.0 + 100.0 * (double) i;
        double f = 10.0 * (double) i;
        rec[i].rainfall_mm = r;
        rec[i].fertiliser_kg_ha = f;
        rec[i].yield_tonnes_ha = 0.5 + 0.001 * r + 0.003 * f;
    }
    gh_cocobod_result_t result;
    double residuals[5];
    result.residuals = residuals;
    gh_kernel_args_t a = { .input = rec, .n = m, .output = &result, .seed = 1 };
    cocobod_yield_regression(&a);
    CHECK(fabs(result.r_squared - 1.0) < 1e-6, "noiseless data should give R^2 ~= 1.0");
    return 1;
}

static int test_cocobod_residuals_sum_near_zero(void) {
    /* OLS with an intercept term always has residuals summing to ~0. */
    const size_t m = 5;
    gh_cocobod_record_t rec[5] = {
        {900, 10, 1.10}, {1000, 50, 1.30}, {1200, 0, 1.05},
        {1500, 200, 1.80}, {2000, 300, 2.10},
    };
    gh_cocobod_result_t result;
    double residuals[5];
    result.residuals = residuals;
    gh_kernel_args_t a = { .input = rec, .n = m, .output = &result, .seed = 1 };
    cocobod_yield_regression(&a);
    double sum = 0.0;
    for (size_t i = 0; i < m; i++) sum += residuals[i];
    CHECK(fabs(sum) < 1e-6, "OLS residuals with intercept should sum to ~0");
    return 1;
}

static int test_cocobod_r_squared_in_unit_range(void) {
    const size_t m = 5;
    gh_cocobod_record_t rec[5] = {
        {900, 10, 1.40}, {1000, 50, 0.90}, {1200, 0, 1.60},
        {1500, 200, 1.00}, {2000, 300, 1.95},
    }; /* noisy / non-perfectly-linear on purpose */
    gh_cocobod_result_t result;
    double residuals[5];
    result.residuals = residuals;
    gh_kernel_args_t a = { .input = rec, .n = m, .output = &result, .seed = 1 };
    cocobod_yield_regression(&a);
    CHECK(result.r_squared <= 1.0 + 1e-9, "R^2 should not exceed 1.0");
    return 1;
}

static int test_cocobod_constant_yield_gives_zero_slopes(void) {
    const size_t m = 4;
    gh_cocobod_record_t rec[4] = {
        {900, 10, 2.0}, {1200, 100, 2.0}, {1500, 200, 2.0}, {2000, 300, 2.0},
    };
    gh_cocobod_result_t result;
    double residuals[4];
    result.residuals = residuals;
    gh_kernel_args_t a = { .input = rec, .n = m, .output = &result, .seed = 1 };
    cocobod_yield_regression(&a);
    CHECK(fabs(result.b0 - 2.0) < 1e-6, "constant yield: b0 should recover the constant");
    CHECK(fabs(result.b1) < 1e-6 && fabs(result.b2) < 1e-6,
          "constant yield: slopes should be ~0");
    return 1;
}

/* ===================== rainfall_interpolate tests ===================== */

static int test_rainfall_no_missing_passes_through(void) {
    int rows = 3, cols = 3;
    double grid[9] = {1,2,3,4,5,6,7,8,9};
    gh_rainfall_input_t in = { .grid = grid, .rows = rows, .cols = cols };
    double out[9];
    gh_kernel_args_t a = { .input = &in, .n = 9, .output = out, .seed = 1 };
    rainfall_interpolate(&a);
    for (int i = 0; i < 9; i++) {
        CHECK(fabs(out[i] - grid[i]) < 1e-12, "valid cells must pass through unchanged");
    }
    return 1;
}

static int test_rainfall_single_missing_uniform_neighbours(void) {
    /* All neighbours of the centre cell equal 5.0 -> IDW average must be 5.0. */
    int rows = 3, cols = 3;
    double grid[9] = {5,5,5, 5,GH_RAIN_MISSING,5, 5,5,5};
    gh_rainfall_input_t in = { .grid = grid, .rows = rows, .cols = cols };
    double out[9];
    gh_kernel_args_t a = { .input = &in, .n = 9, .output = out, .seed = 1 };
    rainfall_interpolate(&a);
    CHECK(fabs(out[1 * cols + 1] - 5.0) < 1e-9,
          "uniform-valued neighbourhood must interpolate to that value");
    return 1;
}

static int test_rainfall_weighted_by_distance(void) {
    /* Missing cell at (0,0); two valid neighbours: (0,1)=10 at dist 1,
     * (0,2)=20 at dist 2. IDW: w1=1, w2=0.25 -> (10*1+20*0.25)/1.25 = 12.0 */
    int rows = 1, cols = 3;
    double grid[3] = {GH_RAIN_MISSING, 10.0, 20.0};
    gh_rainfall_input_t in = { .grid = grid, .rows = rows, .cols = cols };
    double out[3];
    gh_kernel_args_t a = { .input = &in, .n = 3, .output = out, .seed = 1 };
    rainfall_interpolate(&a);
    CHECK(fabs(out[0] - 12.0) < 1e-9, "IDW result should match hand-computed weighted average");
    return 1;
}

static int test_rainfall_no_valid_neighbour_falls_back_to_zero(void) {
    /* Every cell missing -> no valid neighbour exists anywhere in range. */
    int rows = 2, cols = 2;
    double grid[4] = {GH_RAIN_MISSING, GH_RAIN_MISSING, GH_RAIN_MISSING, GH_RAIN_MISSING};
    gh_rainfall_input_t in = { .grid = grid, .rows = rows, .cols = cols };
    double out[4];
    gh_kernel_args_t a = { .input = &in, .n = 4, .output = out, .seed = 1 };
    rainfall_interpolate(&a);
    for (int i = 0; i < 4; i++) {
        CHECK(out[i] == 0.0, "cell with no valid neighbour in radius should fall back to 0.0");
    }
    return 1;
}

static int test_rainfall_corner_missing_does_not_crash(void) {
    int rows = 2, cols = 2;
    double grid[4] = {GH_RAIN_MISSING, 4.0, 6.0, 8.0};
    gh_rainfall_input_t in = { .grid = grid, .rows = rows, .cols = cols };
    double out[4];
    gh_kernel_args_t a = { .input = &in, .n = 4, .output = out, .seed = 1 };
    rainfall_interpolate(&a);
    CHECK(out[0] > 0.0, "corner cell (fewer in-bounds neighbours) should still interpolate");
    return 1;
}

int main(void) {
    printf("momo_match:\n");
    RUN(test_momo_exact_match);
    RUN(test_momo_amount_beyond_eps);
    RUN(test_momo_amount_within_eps);
    RUN(test_momo_id_not_present);
    RUN(test_momo_mixed_batch);
    RUN(test_momo_reports_bytes_moved);

    printf("cocobod_yield_regression:\n");
    RUN(test_cocobod_recovers_noiseless_line);
    RUN(test_cocobod_perfect_fit_r_squared_is_one);
    RUN(test_cocobod_residuals_sum_near_zero);
    RUN(test_cocobod_r_squared_in_unit_range);
    RUN(test_cocobod_constant_yield_gives_zero_slopes);

    printf("rainfall_interpolate:\n");
    RUN(test_rainfall_no_missing_passes_through);
    RUN(test_rainfall_single_missing_uniform_neighbours);
    RUN(test_rainfall_weighted_by_distance);
    RUN(test_rainfall_no_valid_neighbour_falls_back_to_zero);
    RUN(test_rainfall_corner_missing_does_not_crash);

    printf("\n%d passed, %d failed (of %d)\n", g_pass, g_fail, g_pass + g_fail);
    return g_fail == 0 ? 0 : 1;
}
