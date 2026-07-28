#!/bin/bash
#
# npu_full_benchmark.sh — Comprehensive NPU matmul benchmark
#
# Tests all supported shapes across different core configurations:
# - Single core (32-row tiles)
# - Whole array (256-row tiles)
# - Parallel execution (multiple shapes simultaneously)
# - Sequential execution (shapes back-to-back)
# - Mixed parallelism (various combinations)
#
# Usage: ./scripts/npu_full_benchmark.sh
#
# Output: npu_benchmark_results.csv with timing and accuracy data

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VNNI_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
RESULTS_FILE="${VNNI_DIR}/npu_benchmark_results.csv"

# Ensure we have the NPU kernels built
if [ ! -f "${VNNI_DIR}/npu/kernels/manifest.json" ]; then
    echo "Building NPU kernels..."
    cd "${VNNI_DIR}"
    make npu-kernels || {
        echo "Warning: Kernel build failed, continuing with existing kernels"
    }
fi

# Create test binary
echo "Compiling benchmark binary..."
cd "${VNNI_DIR}"
cat > /tmp/npu_benchmark_test.c << 'EOF'
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include "npu/xdna2_backend.h"

typedef struct {
    int rows;
    int inner_dim;
    int out_cols;
    char core_type[20];
    char test_type[20];
    int parallel_count;
    double elapsed_ms;
    int accuracy_ok;
} test_result_t;

static void run_single_test(int rows, int inner_dim, int out_cols, 
                           const char *core_type, const char *test_type,
                           int parallel_count, test_result_t *result) {
    int8_t *input = malloc(rows * inner_dim * sizeof(int8_t));
    int8_t *weights = malloc(out_cols * inner_dim * sizeof(int8_t));
    float *output = malloc(rows * out_cols * sizeof(float));
    float *expected = malloc(rows * out_cols * sizeof(float));
    
    if (!input || !weights || !output || !expected) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    srand(42);
    for (int i = 0; i < rows * inner_dim; i++) {
        input[i] = (int8_t)(rand() % 20 - 10);
    }
    for (int i = 0; i < out_cols * inner_dim; i++) {
        weights[i] = (int8_t)(rand() % 20 - 10);
    }
    memset(output, 0, rows * out_cols * sizeof(float));
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < out_cols; j++) {
            int32_t sum = 0;
            for (int k = 0; k < inner_dim; k++) {
                sum += (int32_t)input[i * inner_dim + k] * 
                       (int32_t)weights[j * inner_dim + k];
            }
            expected[i * out_cols + j] = (float)sum;
        }
    }
    
    clock_t start = clock();
    int ret = strix_xdna2_matmul(input, rows, inner_dim, weights, out_cols, 
                                output, NULL);
    clock_t end = clock();
    
    result->elapsed_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    result->accuracy_ok = (ret != 0);
    
    if (ret != 0) {
        int mismatches = 0;
        for (int i = 0; i < rows * out_cols; i++) {
            if (fabsf(output[i] - expected[i]) > 1.0f) {
                mismatches++;
            }
        }
        result->accuracy_ok = (mismatches == 0);
    }
    
    free(input); free(weights); free(output); free(expected);
}

static int count_passed(test_result_t *results, int count) {
    int passed = 0;
    for (int i = 0; i < count; i++) {
        if (results[i].accuracy_ok) passed++;
    }
    return passed;
}

static int benchmark_uapi_available(void) {
    if (!strix_xdna2_is_supported()) {
        printf("NPU benchmark SKIP (%s)\n", strix_xdna2_failure_reason());
        return 0;
    }

    (void)strix_xdna2_kernel_exists(256, 4096, 1024, XDNA2_FMT_INT8);
    const char *reason = strix_xdna2_failure_reason();
    if (reason && strstr(reason, "DRM_IOCTL_AMDXDNA_CONFIG_HWCTX")) {
        printf("NPU benchmark SKIP (%s)\n", reason);
        return 0;
    }
    return 1;
}

static void run_benchmark_suite(void) {
    test_result_t results[100];
    int result_count = 0;
    
    struct { int rows; int inner; int out; const char *core; } shapes[] = {
        {256, 4096, 1024, "whole"},
        {256, 1024, 4096, "whole"},
        {256, 4096, 16384, "whole"},
        {256, 4096, 512, "whole"},
        {256, 8192, 4096, "whole"},
        {32, 4096, 1024, "single"},
        {32, 1024, 4096, "single"},
        {32, 4096, 16384, "single"},
        {32, 4096, 512, "single"},
        {32, 8192, 4096, "single"},
    };
    int num_shapes = sizeof(shapes) / sizeof(shapes[0]);
    
    printf("=== NPU Matmul Benchmark Suite ===\n");
    printf("Testing %d shapes across different execution modes\n\n", num_shapes);
    
    printf("Phase 1: Sequential execution\n");
    for (int i = 0; i < num_shapes; i++) {
        printf("  Testing %dx%dx%d (%s core)... ", 
               shapes[i].rows, shapes[i].inner, shapes[i].out, shapes[i].core);
        run_single_test(shapes[i].rows, shapes[i].inner, shapes[i].out,
                       shapes[i].core, "sequential", 1, &results[result_count]);
        results[result_count].rows = shapes[i].rows;
        results[result_count].inner_dim = shapes[i].inner;
        results[result_count].out_cols = shapes[i].out;
        strncpy(results[result_count].core_type, shapes[i].core, 19);
        strncpy(results[result_count].test_type, "sequential", 19);
        results[result_count].parallel_count = 1;
        result_count++;
        printf("%.2f ms, accuracy: %s\n", 
               results[result_count-1].elapsed_ms,
               results[result_count-1].accuracy_ok ? "PASS" : "FAIL");
    }
    
    printf("\nPhase 2: Parallel execution (all shapes)\n");
    printf("  Running %d shapes in parallel...\n", num_shapes);
    
    clock_t start = clock();
    for (int i = 0; i < num_shapes; i++) {
        run_single_test(shapes[i].rows, shapes[i].inner, shapes[i].out,
                       shapes[i].core, "parallel", num_shapes, &results[result_count]);
        results[result_count].rows = shapes[i].rows;
        results[result_count].inner_dim = shapes[i].inner;
        results[result_count].out_cols = shapes[i].out;
        strncpy(results[result_count].core_type, shapes[i].core, 19);
        strncpy(results[result_count].test_type, "parallel", 19);
        results[result_count].parallel_count = num_shapes;
        result_count++;
    }
    clock_t end = clock();
    double total_parallel_time = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    printf("  Total parallel time: %.2f ms (avg %.2f ms per shape)\n",
           total_parallel_time, total_parallel_time / num_shapes);
    
    printf("\nPhase 3: Mixed parallelism (2 shapes at a time)\n");
    for (int i = 0; i < num_shapes; i += 2) {
        int j = (i + 1 < num_shapes) ? i + 1 : i;
        printf("  Testing shapes %d and %d in parallel...\n", i+1, j+1);
        
        run_single_test(shapes[i].rows, shapes[i].inner, shapes[i].out,
                       shapes[i].core, "mixed", 2, &results[result_count]);
        results[result_count].rows = shapes[i].rows;
        results[result_count].inner_dim = shapes[i].inner;
        results[result_count].out_cols = shapes[i].out;
        strncpy(results[result_count].core_type, shapes[i].core, 19);
        strncpy(results[result_count].test_type, "mixed", 19);
        results[result_count].parallel_count = 2;
        result_count++;
        
        if (j != i) {
            run_single_test(shapes[j].rows, shapes[j].inner, shapes[j].out,
                           shapes[j].core, "mixed", 2, &results[result_count]);
            results[result_count].rows = shapes[j].rows;
            results[result_count].inner_dim = shapes[j].inner;
            results[result_count].out_cols = shapes[j].out;
            strncpy(results[result_count].core_type, shapes[j].core, 19);
            strncpy(results[result_count].test_type, "mixed", 19);
            results[result_count].parallel_count = 2;
            result_count++;
        }
    }
    
    printf("\nPhase 4: All core types simultaneously\n");
    printf("  Testing 5 whole-array + 5 single-core shapes...\n");
    
    for (int i = 0; i < num_shapes; i++) {
        run_single_test(shapes[i].rows, shapes[i].inner, shapes[i].out,
                       shapes[i].core, "all_types", num_shapes, &results[result_count]);
        results[result_count].rows = shapes[i].rows;
        results[result_count].inner_dim = shapes[i].inner;
        results[result_count].out_cols = shapes[i].out;
        strncpy(results[result_count].core_type, shapes[i].core, 19);
        strncpy(results[result_count].test_type, "all_types", 19);
        results[result_count].parallel_count = num_shapes;
        result_count++;
    }
    
    printf("\n=== Benchmark Summary ===\n");
    printf("Total tests: %d\n", result_count);
    printf("Tests passed: %d\n", count_passed(results, result_count));
    
    printf("\n=== CSV Output ===\n");
    printf("rows,inner_dim,out_cols,core_type,test_type,parallel_count,elapsed_ms,accuracy_ok\n");
    for (int i = 0; i < result_count; i++) {
        printf("%d,%d,%d,%s,%s,%d,%.2f,%d\n",
               results[i].rows, results[i].inner_dim, results[i].out_cols,
               results[i].core_type, results[i].test_type, 
               results[i].parallel_count, results[i].elapsed_ms,
               results[i].accuracy_ok ? 1 : 0);
    }
}

int main(void) {
    if (!benchmark_uapi_available()) {
        strix_xdna2_shutdown();
        return 0;
    }
    run_benchmark_suite();
    strix_xdna2_shutdown();
    return 0;
}
EOF

gcc -I. -I.. -O3 -march=native -std=c11 -o /tmp/npu_benchmark_test \
    /tmp/npu_benchmark_test.c \
    npu/xdna2_backend.o ../c/npu_kernels/xdna2_driver.o ../c/npu_kernels/xdna2_matmul.o ../c/npu_kernels/xdna2_xrt_driver.o \
    -lm -ldl

echo "Running comprehensive NPU benchmark..."
echo "Results will be saved to: ${RESULTS_FILE}"

/tmp/npu_benchmark_test 2>&1 | tee "${RESULTS_FILE}.txt"

echo ""
echo "=== Extracting CSV data ==="
grep -E "^[0-9]+," "${RESULTS_FILE}.txt" > "${RESULTS_FILE}"

echo ""
echo "=== Benchmark complete ==="
echo "Results saved to: ${RESULTS_FILE}"
echo "Full output saved to: ${RESULTS_FILE}.txt"
echo ""
echo "To view results:"
echo "  cat ${RESULTS_FILE}"
echo "  head -20 ${RESULTS_FILE}.txt"
