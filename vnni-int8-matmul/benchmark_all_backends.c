/*
 * benchmark_all_backends.c — Comprehensive multi-backend benchmark suite
 * 
 * Tests CPU, GPU, and NPU backends in all combinations:
 * - Single backend: CPU, GPU, NPU
 * - Two backends: CPU+GPU, CPU+NPU, GPU+NPU
 * - All three: CPU+GPU+NPU
 * 
 * Core counts: 1 to limit (CPU:32, GPU:CUs, NPU:16 tiles)
 * Workload: 15 model shapes × all tensor types
 * Duration: Configurable (default 30s per config)
 * 
 * Output: CSV log + real-time progress + thermal monitoring
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>
#include <math.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>

#include "cpu/vnni_cpu_backend.h"
#include "gpu/vulkan_backend.h"
#include "npu/xdna2_backend.h"
#include "sched/backend_placement.h"
#include "sched/moe_schedule.h"

/* ── Configuration ──────────────────────────────────────────────────────── */

#define MAX_BACKENDS 3
#define MAX_CORES 64
#define MAX_SHAPES 100
#define MAX_TESTS 10000

/* Test duration in seconds */
#define DEFAULT_DURATION_S 30
#define STRESS_DURATION_S 120
#define THERMAL_DURATION_S 300

/* Thermal monitoring interval in seconds */
#define THERMAL_INTERVAL_S 1

/* ── Data Structures ────────────────────────────────────────────────────── */

typedef enum {
    BACKEND_CPU = 0,
    BACKEND_GPU = 1,
    BACKEND_NPU = 2,
    BACKEND_COUNT = 3
} backend_t;

typedef struct {
    const char *name;
    int cores;
    int max_cores;
} backend_config_t;

typedef struct {
    int rows;
    int inner_dim;
    int out_cols;
    const char *model_type;
    const char *tensor_type;
} workload_shape_t;

typedef struct {
    backend_t backends[MAX_BACKENDS];
    int backend_count;
    int cores[MAX_BACKENDS];
    int duration_s;
    int test_id;
    
    /* Results */
    double elapsed_ms;
    int success;
    int errors;
    
    /* Thermal data */
    double cpu_temp_avg;
    double gpu_temp_avg;
    double max_cpu_temp;
    double max_gpu_temp;
    int thermal_throttled;
} test_config_t;

typedef struct {
    int test_id;
    backend_t backends[MAX_BACKENDS];
    int backend_count;
    int cores[MAX_BACKENDS];
    workload_shape_t shape;
    double elapsed_ms;
    int success;
    double cpu_temp_avg;
    double gpu_temp_avg;
    double max_cpu_temp;
    double max_gpu_temp;
} test_result_t;

/* ── Global State ───────────────────────────────────────────────────────── */

static volatile int g_running = 1;
static FILE *g_csv_log = NULL;
static FILE *g_thermal_log = NULL;
static test_result_t g_results[MAX_TESTS];
static int g_result_count = 0;

/* ── Workload Definitions ──────────────────────────────────────────────── */

static workload_shape_t g_workloads[] = {
    /* NPU shapes (10) */
    {256, 4096, 1024, "expert", "gate_proj"},
    {256, 1024, 4096, "expert", "down_proj"},
    {256, 4096, 16384, "attention", "q_proj"},
    {256, 4096, 512, "attention", "k_proj"},
    {256, 8192, 4096, "attention", "o_proj"},
    {32, 4096, 1024, "expert", "gate_proj"},
    {32, 1024, 4096, "expert", "down_proj"},
    {32, 4096, 16384, "attention", "q_proj"},
    {32, 4096, 512, "attention", "k_proj"},
    {32, 8192, 4096, "attention", "o_proj"},
    
    /* CPU/GPU equivalents (5 more for variety) */
    {512, 2048, 1024, "dense", "linear"},
    {1024, 4096, 2048, "dense", "linear"},
    {256, 8192, 4096, "attention", "v_proj"},
    {128, 1024, 512, "norm", "layer_norm"},
    {64, 512, 256, "embed", "embedding"}
};

static int g_workload_count = sizeof(g_workloads) / sizeof(g_workloads[0]);

/* ── Backend Configuration ─────────────────────────────────────────────── */

static backend_config_t g_backend_configs[] = {
    {"CPU", 32, 32},
    {"GPU", 60, 60},  /* Radeon 8060S has ~60 CUs */
    {"NPU", 16, 16}
};

/* ── Thermal Monitoring ────────────────────────────────────────────────── */

static double read_cpu_temperature(void) {
    FILE *f = fopen("/sys/class/thermal/thermal_zone0/temp", "r");
    if (!f) return -1.0;
    
    double temp = -1.0;
    int raw;
    if (fscanf(f, "%d", &raw) == 1) {
        temp = raw / 1000.0;
    }
    fclose(f);
    return temp;
}

static double read_gpu_temperature(void) {
    /* Try rocm-smi first */
    FILE *p = popen("rocm-smi --showtemp --json 2>/dev/null", "r");
    if (!p) return -1.0;
    
    double temp = -1.0;
    char line[256];
    while (fgets(line, sizeof(line), p)) {
        if (strstr(line, "\"temp":')) {
            char *colon = strchr(line, ':');
            if (colon) {
                temp = atof(colon + 1);
                break;
            }
        }
    }
    pclose(p);
    return temp;
}

static void monitor_thermal(test_config_t *config, double *cpu_temp_avg, 
                           double *gpu_temp_avg, double *max_cpu_temp, 
                           double *max_gpu_temp, int *throttled) {
    *cpu_temp_avg = 0.0;
    *gpu_temp_avg = 0.0;
    *max_cpu_temp = 0.0;
    *max_gpu_temp = 0.0;
    *throttled = 0;
    
    double cpu_sum = 0.0, gpu_sum = 0.0;
    int cpu_count = 0, gpu_count = 0;
    
    time_t start = time(NULL);
    time_t end = start + config->duration_s;
    
    while (time(NULL) < end && g_running) {
        double cpu_temp = read_cpu_temperature();
        double gpu_temp = read_gpu_temperature();
        
        if (cpu_temp >= 0) {
            cpu_sum += cpu_temp;
            cpu_count++;
            if (cpu_temp > *max_cpu_temp) *max_cpu_temp = cpu_temp;
            
            /* Thermal throttling threshold (90°C for AMD) */
            if (cpu_temp > 90.0) {
                *throttled = 1;
            }
        }
        
        if (gpu_temp >= 0) {
            gpu_sum += gpu_temp;
            gpu_count++;
            if (gpu_temp > *max_gpu_temp) *max_gpu_temp = gpu_temp;
            
            /* Thermal throttling threshold (110°C for AMD GPU) */
            if (gpu_temp > 110.0) {
                *throttled = 1;
            }
        }
        
        /* Log thermal data */
        if (g_thermal_log) {
            fprintf(g_thermal_log, "%ld,%s,%.2f,%.2f\n", 
                    (long)time(NULL), 
                    config->backends[0] == BACKEND_CPU ? "CPU" : 
                    config->backends[0] == BACKEND_GPU ? "GPU" : "NPU",
                    cpu_temp >= 0 ? cpu_temp : -1.0,
                    gpu_temp >= 0 ? gpu_temp : -1.0);
        }
        
        sleep(THERMAL_INTERVAL_S);
    }
    
    if (cpu_count > 0) *cpu_temp_avg = cpu_sum / cpu_count;
    if (gpu_count > 0) *gpu_temp_avg = gpu_sum / gpu_count;
}

/* ── Benchmark Execution ───────────────────────────────────────────────── */

static void run_cpu_benchmark(test_config_t *config, workload_shape_t *shape, 
                             double *elapsed_ms) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Run CPU VNNI benchmark */
    int success = benchmark_vnni_cpu(shape->rows, shape->inner_dim, 
                                    shape->out_cols, config->cores[0]);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    *elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                  (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    config->success = success;
}

static void run_gpu_benchmark(test_config_t *config, workload_shape_t *shape, 
                             double *elapsed_ms) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Run Vulkan/GPU benchmark */
    int success = benchmark_vulkan_gpu(shape->rows, shape->inner_dim, 
                                      shape->out_cols, config->cores[1]);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    *elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                  (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    config->success = success;
}

static void run_npu_benchmark(test_config_t *config, workload_shape_t *shape, 
                             double *elapsed_ms) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Run NPU benchmark */
    int success = benchmark_npu_xdna2(shape->rows, shape->inner_dim, 
                                     shape->out_cols, config->cores[2]);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    *elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                  (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    config->success = success;
}

static void run_combined_benchmark(test_config_t *config, workload_shape_t *shape, 
                                  double *elapsed_ms) {
    struct timespec start, end;
    clock_gettime(CLOCK_MONOTONIC, &start);
    
    /* Run combined backend benchmark */
    int success = benchmark_combined(config->backends, config->backend_count,
                                    config->cores, shape->rows, 
                                    shape->inner_dim, shape->out_cols);
    
    clock_gettime(CLOCK_MONOTONIC, &end);
    *elapsed_ms = (end.tv_sec - start.tv_sec) * 1000.0 + 
                  (end.tv_nsec - start.tv_nsec) / 1000000.0;
    
    config->success = success;
}

/* ── Test Generation ───────────────────────────────────────────────────── */

static void generate_test_configs(test_config_t *configs, int *count) {
    *count = 0;
    
    /* Single backend tests */
    backend_t single_backends[][1] = {{BACKEND_CPU}, {BACKEND_GPU}, {BACKEND_NPU}};
    int single_counts[] = {1, 1, 1};
    
    for (int b = 0; b < 3; b++) {
        int max_cores = g_backend_configs[b].max_cores;
        int core_counts[] = {1, 2, 4, 8, 16, 32};
        int core_limits[] = {1, 2, 4, 8, 16, max_cores};
        
        for (int c = 0; c < 6 && core_counts[c] <= max_cores; c++) {
            test_config_t *config = &configs[(*count)++];
            config->backends[0] = single_backends[b][0];
            config->backend_count = 1;
            config->cores[0] = core_counts[c];
            config->duration_s = DEFAULT_DURATION_S;
            config->test_id = *count;
        }
    }
    
    /* Two backend tests */
    backend_t dual_backends[][2] = {
        {BACKEND_CPU, BACKEND_GPU},
        {BACKEND_CPU, BACKEND_NPU},
        {BACKEND_GPU, BACKEND_NPU}
    };
    
    for (int b = 0; b < 3; b++) {
        int max_cores_0 = g_backend_configs[dual_backends[b][0]].max_cores;
        int max_cores_1 = g_backend_configs[dual_backends[b][1]].max_cores;
        
        for (int c0 = 1; c0 <= 16; c0++) {
            for (int c1 = 1; c1 <= 16; c1++) {
                if (c0 > max_cores_0 || c1 > max_cores_1) continue;
                
                test_config_t *config = &configs[(*count)++];
                config->backends[0] = dual_backends[b][0];
                config->backends[1] = dual_backends[b][1];
                config->backend_count = 2;
                config->cores[0] = c0;
                config->cores[1] = c1;
                config->duration_s = DEFAULT_DURATION_S;
                config->test_id = *count;
            }
        }
    }
    
    /* All three backends test */
    for (int c0 = 1; c0 <= 16; c0++) {
        for (int c1 = 1; c1 <= 16; c1++) {
            for (int c2 = 1; c2 <= 16; c2++) {
                test_config_t *config = &configs[(*count)++];
                config->backends[0] = BACKEND_CPU;
                config->backends[1] = BACKEND_GPU;
                config->backends[2] = BACKEND_NPU;
                config->backend_count = 3;
                config->cores[0] = c0;
                config->cores[1] = c1;
                config->cores[2] = c2;
                config->duration_s = DEFAULT_DURATION_S;
                config->test_id = *count;
            }
        }
    }
}

/* ── Main Benchmark Loop ───────────────────────────────────────────────── */

static void run_benchmark_suite(test_config_t *configs, int config_count) {
    printf("=== Multi-Backend Benchmark Suite ===\n");
    printf("Total configurations: %d\n", config_count);
    printf("Duration per test: %d seconds\n\n", DEFAULT_DURATION_S);
    
    for (int i = 0; i < config_count && g_running; i++) {
        test_config_t *config = &configs[i];
        
        printf("[%d/%d] Testing: ", i+1, config_count);
        for (int b = 0; b < config->backend_count; b++) {
            if (b > 0) printf("+");
            printf("%s(%d)", g_backend_configs[config->backends[b]].name, 
                   config->cores[b]);
        }
        printf("\n");
        
        /* Monitor thermal during test */
        double cpu_temp_avg, gpu_temp_avg, max_cpu_temp, max_gpu_temp;
        int thermal_throttled;
        
        pthread_t thermal_thread;
        pthread_create(&thermal_thread, NULL, 
                      (void*(*)(void*))monitor_thermal, config);
        
        /* Run benchmark for each workload shape */
        for (int w = 0; w < g_workload_count && g_running; w++) {
            workload_shape_t *shape = &g_workloads[w];
            double elapsed_ms = 0.0;
            
            printf("  Shape: %s/%s/%dx%dx%d\n", 
                   shape->model_type, shape->tensor_type,
                   shape->rows, shape->inner_dim, shape->out_cols);
            
            /* Run benchmark based on backend configuration */
            if (config->backend_count == 1) {
                switch (config->backends[0]) {
                    case BACKEND_CPU:
                        run_cpu_benchmark(config, shape, &elapsed_ms);
                        break;
                    case BACKEND_GPU:
                        run_gpu_benchmark(config, shape, &elapsed_ms);
                        break;
                    case BACKEND_NPU:
                        run_npu_benchmark(config, shape, &elapsed_ms);
                        break;
                }
            } else {
                run_combined_benchmark(config, shape, &elapsed_ms);
            }
            
            /* Log result */
            if (g_csv_log) {
                fprintf(g_csv_log, "%d,%s,%s,%d,%d,%s,%s,%d,%d,%.2f,%d,%.2f,%.2f,%.2f,%.2f,%d\n",
                        config->test_id,
                        g_backend_configs[config->backends[0]].name,
                        config->backend_count == 1 ? g_backend_configs[config->backends[0]].name :
                        config->backend_count == 2 ? (config->backends[0] == BACKEND_CPU ? "CPU+GPU" : 
                                                       config->backends[0] == BACKEND_CPU ? "CPU+NPU" : "GPU+NPU") :
                        "CPU+GPU+NPU",
                        config->cores[0],
                        config->cores[1],
                        shape->model_type,
                        shape->tensor_type,
                        shape->rows,
                        shape->inner_dim,
                        elapsed_ms,
                        config->success,
                        0.0, 0.0, 0.0, 0.0, 0);
            }
            
            printf("    Time: %.2f ms, Success: %d\n", elapsed_ms, config->success);
        }
        
        /* Wait for thermal monitoring to complete */
        pthread_join(thermal_thread, NULL);
        
        printf("    Thermal: CPU=%.1f°C, GPU=%.1f°C, Max CPU=%.1f°C, Max GPU=%.1f°C, Throttled=%d\n",
               cpu_temp_avg, gpu_temp_avg, max_cpu_temp, max_gpu_temp, thermal_throttled);
        
        printf("\n");
    }
}

/* ── Signal Handler ────────────────────────────────────────────────────── */

static void signal_handler(int sig) {
    printf("\n\nReceived signal %d, stopping gracefully...\n", sig);
    g_running = 0;
}

/* ── Main ──────────────────────────────────────────────────────────────── */

int main(int argc, char *argv[]) {
    int duration_s = DEFAULT_DURATION_S;
    const char *csv_file = "benchmark_results.csv";
    const char *thermal_file = "thermal_log.csv";
    
    /* Parse arguments */
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--duration") == 0 && i+1 < argc) {
            duration_s = atoi(argv[++i]);
        } else if (strcmp(argv[i], "--csv") == 0 && i+1 < argc) {
            csv_file = argv[++i];
        } else if (strcmp(argv[i], "--thermal") == 0 && i+1 < argc) {
            thermal_file = argv[++i];
        } else if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: %s [options]\n", argv[0]);
            printf("  --duration SECONDS  Test duration (default: %d)\n", DEFAULT_DURATION_S);
            printf("  --csv FILE          CSV output file (default: %s)\n", csv_file);
            printf("  --thermal FILE      Thermal log file (default: %s)\n", thermal_file);
            return 0;
        }
    }
    
    /* Setup signal handlers */
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    /* Open log files */
    g_csv_log = fopen(csv_file, "w");
    if (!g_csv_log) {
        fprintf(stderr, "Error: Cannot open %s for writing\n", csv_file);
        return 1;
    }
    
    g_thermal_log = fopen(thermal_file, "w");
    if (!g_thermal_log) {
        fprintf(stderr, "Error: Cannot open %s for writing\n", thermal_file);
        fclose(g_csv_log);
        return 1;
    }
    
    /* Write CSV headers */
    fprintf(g_csv_log, "test_id,backend,backend_combo,cores_0,cores_1,model_type,tensor_type,"
            "rows,inner_dim,out_cols,elapsed_ms,success,cpu_temp_avg,gpu_temp_avg,"
            "max_cpu_temp,max_gpu_temp,thermal_throttled\n");
    
    fprintf(g_thermal_log, "timestamp,backend,cpu_temp,gpu_temp\n");
    
    /* Generate test configurations */
    test_config_t configs[MAX_TESTS];
    int config_count = 0;
    generate_test_configs(configs, &config_count);
    
    printf("Generated %d test configurations\n\n", config_count);
    
    /* Run benchmark suite */
    run_benchmark_suite(configs, config_count);
    
    /* Cleanup */
    fclose(g_csv_log);
    fclose(g_thermal_log);
    
    printf("\n=== Benchmark Complete ===\n");
    printf("Results saved to: %s\n", csv_file);
    printf("Thermal data saved to: %s\n", thermal_file);
    
    return 0;
}