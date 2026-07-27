/**
 * test_npu_execution.c — Test actual NPU matmul execution on supported shapes
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "../c/npu_kernels/xdna2_matmul.h"

#define TEST_SHAPE(M, K, N) do { \
    test_shape(M, K, N); \
} while(0)

static void test_shape(int rows, int inner_dim, int out_cols) {
    printf("\n=== Testing shape: %dx%dx%d ===\n", rows, inner_dim, out_cols);
    
    int8_t *input = malloc(rows * inner_dim * sizeof(int8_t));
    int8_t *weights = malloc(out_cols * inner_dim * sizeof(int8_t));
    float *output = malloc(rows * out_cols * sizeof(float));
    float *expected = malloc(rows * out_cols * sizeof(float));
    
    if (!input || !weights || !output || !expected) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    
    // Initialize with test data
    for (int i = 0; i < rows * inner_dim; i++) {
        input[i] = (int8_t)(i % 10 - 5);
    }
    for (int i = 0; i < out_cols * inner_dim; i++) {
        weights[i] = (int8_t)((i + 1) % 10 - 5);
    }
    memset(output, 0, rows * out_cols * sizeof(float));
    
    // Compute expected result on CPU
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < out_cols; j++) {
            int32_t sum = 0;
            for (int k = 0; k < inner_dim; k++) {
                sum += (int32_t)input[i * inner_dim + k] * (int32_t)weights[j * inner_dim + k];
            }
            expected[i * out_cols + j] = (float)sum;
        }
    }
    
    // Try NPU execution
    clock_t start = clock();
    int ret = strix_xdna2_matmul(input, rows, inner_dim, weights, out_cols, output, NULL);
    clock_t end = clock();
    
    if (ret != 0) {
        printf("NPU execution FAILED (error %d)\n", ret);
        free(input); free(weights); free(output); free(expected);
        return;
    }
    
    double elapsed_ms = (double)(end - start) / CLOCKS_PER_SEC * 1000.0;
    printf("NPU execution SUCCESS in %.2f ms\n", elapsed_ms);
    
    // Compare results
    int mismatches = 0;
    for (int i = 0; i < rows * out_cols; i++) {
        if (fabsf(output[i] - expected[i]) > 1.0f) {
            if (mismatches < 5) {
                printf("  Mismatch at [%d]: got=%f expected=%f\n", 
                       i, output[i], expected[i]);
            }
            mismatches++;
        }
    }
    
    if (mismatches == 0) {
        printf("✓ Results match CPU reference\n");
    } else {
        printf("✗ %d mismatches found\n", mismatches);
    }
    
    free(input); free(weights); free(output); free(expected);
}

int main(void) {
    printf("NPU Matmul Execution Test\n");
    printf("=========================\n");
    
    // Test supported shapes from manifest
    printf("\nTesting 256-row tile shapes (whole array):\n");
    TEST_SHAPE(256, 4096, 1024);  // expert gate_proj/up_proj
    TEST_SHAPE(256, 1024, 4096);  // expert down_proj
    TEST_SHAPE(256, 4096, 16384); // self_attn q_proj
    TEST_SHAPE(256, 4096, 512);   // self_attn k_proj/v_proj
    TEST_SHAPE(256, 8192, 4096);  // self_attn o_proj
    
    printf("\nTesting 32-row tile shapes (single core):\n");
    TEST_SHAPE(32, 4096, 1024);   // expert gate_proj/up_proj
    TEST_SHAPE(32, 1024, 4096);   // expert down_proj
    TEST_SHAPE(32, 4096, 16384);  // self_attn q_proj
    TEST_SHAPE(32, 4096, 512);    // self_attn k_proj/v_proj
    TEST_SHAPE(32, 8192, 4096);   // self_attn o_proj
    
    printf("\n=== Test Complete ===\n");
    return 0;
}
