# Multi-Backend Benchmark Suite — Complete Guide

**Date**: July 28, 2026  
**Status**: Ready for execution  
**Hardware**: AMD Strix Halo (Ryzen AI Max+ 395)

---

## Overview

This comprehensive benchmark suite tests **CPU, GPU, and NPU** backends in all combinations to find optimal performance, identify thermal limitations, and gather extensive statistics for decision-making.

### What It Tests

- **Single backends**: CPU, GPU, NPU (1 to max cores)
- **Two backends**: CPU+GPU, CPU+NPU, GPU+NPU (all core combinations)
- **All three**: CPU+GPU+NPU (full pipeline)
- **Workloads**: 15 model shapes × all tensor types
- **Thermal monitoring**: Real-time CPU/GPU temperature tracking
- **Stress testing**: Configurable duration (default 30s, up to 300s)

---

## Test Matrix

### Backend Combinations

| Configuration | Backends | Core Counts | Tests |
|---------------|----------|-------------|-------|
| **Single** | CPU, GPU, NPU | 1, 2, 4, 8, 16, 32 | ~50 |
| **Dual** | CPU+GPU, CPU+NPU, GPU+NPU | 1-16 each | ~500 |
| **Triple** | CPU+GPU+NPU | 1-16 each | ~4000 |
| **Total** | All combinations | Various | **~4550** |

### Workload Shapes

**NPU Shapes (10)**:
- Expert: gate_proj, down_proj (256-row and 32-row)
- Attention: q_proj, k_proj, o_proj (256-row and 32-row)

**CPU/GPU Shapes (5)**:
- Dense: linear layers
- Attention: v_proj
- Norm: layer_norm
- Embed: embedding

**Total**: 15 unique workload configurations

### Core Count Progression

Tests start at **1 core** and increase to the maximum:
- **CPU**: 1, 2, 4, 8, 16, 32 cores
- **GPU**: 1, 2, 4, 8, 16 CUs (detect actual count)
- **NPU**: 1, 2, 4, 8, 16 tiles

This allows you to identify:
- Where each backend excels
- Scaling characteristics
- Optimal core count for each workload
- Thermal throttling thresholds

---

## Execution

### Quick Test (30 seconds per config)

```bash
cd /home/leite/colibri/vnni-int8-matmul
./scripts/run_all_backends_benchmark.sh
```

**Duration**: ~2-3 hours total  
**Use case**: Quick validation, identify obvious bottlenecks

### Stress Test (2 minutes per config)

```bash
./scripts/run_all_backends_benchmark.sh --duration 120
```

**Duration**: ~8-10 hours total  
**Use case**: Comprehensive performance data, thermal analysis

### Thermal Soak (5 minutes per config)

```bash
./scripts/run_all_backends_benchmark.sh --duration 300
```

**Duration**: ~20+ hours total  
**Use case**: Find thermal limits, sustained performance

---

## Output Files

### 1. `benchmark_results.csv`

Structured data for analysis:

```csv
test_id,backend,backend_combo,cores_0,cores_1,model_type,tensor_type,
rows,inner_dim,out_cols,elapsed_ms,success,cpu_temp_avg,gpu_temp_avg,
max_cpu_temp,max_gpu_temp,thermal_throttled

1,CPU,CPU,1,,,,256,4096,1024,1.62,1,0.0,0.0,0.0,0.0,0
2,CPU,CPU,2,,,,256,4096,1024,1.58,1,0.0,0.0,0.0,0.0,0
...
100,GPU+CPU,GPU+CPU,4,8,,,,256,4096,1024,2.15,1,65.2,72.1,68.5,75.3,0
...
```

**Columns**:
- `test_id`: Sequential test number
- `backend`: Primary backend name
- `backend_combo`: Backend combination (CPU, GPU, NPU, CPU+GPU, etc.)
- `cores_0`, `cores_1`: Core counts for each backend
- `model_type`, `tensor_type`: Workload classification
- `rows`, `inner_dim`, `out_cols`: Matrix dimensions
- `elapsed_ms`: Execution time in milliseconds
- `success`: 1 = success, 0 = failure
- `cpu_temp_avg`, `gpu_temp_avg`: Average temperatures during test
- `max_cpu_temp`, `max_gpu_temp`: Peak temperatures
- `thermal_throttled`: 1 if thermal throttling detected

### 2. `thermal_log.csv`

Time-series thermal data:

```csv
timestamp,backend,cpu_temp,gpu_temp
1722163200,CPU,62.5,68.3
1722163201,CPU,63.1,68.7
1722163202,CPU,64.2,69.1
...
```

**Columns**:
- `timestamp`: Unix timestamp (seconds)
- `backend`: Backend being tested
- `cpu_temp`: CPU temperature (°C)
- `gpu_temp`: GPU temperature (°C)

### 3. Real-time Progress

The benchmark prints progress to stdout:

```
=== Multi-Backend Benchmark Suite ===
Total configurations: 4550
Duration per test: 30 seconds

[1/4550] Testing: CPU(1)
  Shape: expert/gate_proj/256x4096x1024
    Time: 1.62 ms, Success: 1
  Shape: expert/down_proj/256x1024x4096
    Time: 2.63 ms, Success: 1
  ...
    Thermal: CPU=62.5°C, GPU=68.3°C, Max CPU=64.2°C, Max GPU=69.1°C, Throttled=0

[2/4550] Testing: CPU(2)
  ...
```

---

## Analysis Examples

### Performance by Backend

```bash
# Average time per backend
python3 -c "
import pandas as pd
df = pd.read_csv('benchmark_results.csv')
print(df.groupby('backend')['elapsed_ms'].mean())
"
```

### Scaling Analysis

```bash
# How does performance scale with cores?
python3 -c "
import pandas as pd
df = pd.read_csv('benchmark_results.csv')
single = df[df['backend_combo'] == 'CPU']
print(single.groupby('cores_0')['elapsed_ms'].mean())
"
```

### Thermal Limits

```bash
# Find thermal throttling events
python3 -c "
import pandas as pd
df = pd.read_csv('benchmark_results.csv')
throttled = df[df['thermal_throttled'] == 1]
print(f'Throttled tests: {len(throttled)}')
print(throttled[['backend', 'cores_0', 'elapsed_ms', 'max_cpu_temp']].head())
"
```

### Optimal Core Count

```bash
# Find best core count for each workload
python3 -c "
import pandas as pd
df = pd.read_csv('benchmark_results.csv')
best = df.groupby(['model_type', 'tensor_type'])['elapsed_ms'].idxmin()
print(df.loc[best])
"
```

### Combined Backend Performance

```bash
# Compare single vs combined backends
python3 -c "
import pandas as pd
df = pd.read_csv('benchmark_results.csv')
combined = df[df['backend_combo'].str.contains('CPU\+GPU\|CPU\+NPU\|GPU\+NPU')]
print(combined.groupby('backend_combo')['elapsed_ms'].mean())
"
```

---

## Expected Results

### Single Backend Performance

| Backend | Fastest Shape | Slowest Shape | Avg Time |
|---------|---------------|---------------|----------|
| **CPU** | 32×4096×512 (0.5 ms) | 256×4096×16384 (20 ms) | ~5 ms |
| **GPU** | 32×4096×512 (0.3 ms) | 256×4096×16384 (15 ms) | ~3 ms |
| **NPU** | 32×4096×512 (0.5 ms) | 256×4096×16384 (20 ms) | ~8 ms |

### Combined Backend Performance

| Combination | Expected Speedup | Notes |
|-------------|------------------|-------|
| **CPU+GPU** | 1.5-2x | GPU handles large matrices |
| **CPU+NPU** | 1.3-1.8x | NPU excels at fixed shapes |
| **GPU+NPU** | 2-3x | Best for mixed workloads |
| **CPU+GPU+NPU** | 2.5-4x | Maximum parallelism |

### Thermal Limits

- **CPU**: Throttles at ~90°C
- **GPU**: Throttles at ~110°C
- **Sustained load**: Expect 10-15°C above idle

---

## Troubleshooting

### NPU Device Not Found

```
WARNING: NPU device /dev/accel/accel0 not found
```

**Solution**: Ensure XDNA 2 driver is loaded:
```bash
sudo modprobe amdxdna
ls /dev/accel/
```

### Build Failures

```
Error: Failed to build benchmark_all_backends
```

**Solution**: Check dependencies:
```bash
# Install required packages
sudo dnf install -y gcc make libvulkan-devel

# Clean and rebuild
make clean
make all
```

### Permission Denied

```
Error: Cannot open benchmark_results.csv for writing
```

**Solution**: Check permissions:
```bash
chmod +x scripts/run_all_backends_benchmark.sh
```

### Thermal Throttling Detected

```
Throttled=1
```

**Solution**: This is expected under heavy load. The benchmark will continue but performance may degrade. Monitor temperatures and consider:
- Improving cooling
- Reducing workload
- Adding pauses between tests

---

## Next Steps After Benchmark

### 1. Analyze Results

```bash
# Generate summary statistics
python3 << 'EOF'
import pandas as pd
import matplotlib.pyplot as plt

df = pd.read_csv('benchmark_results.csv')

# Plot performance by backend
plt.figure(figsize=(12, 6))
df.groupby('backend')['elapsed_ms'].mean().plot(kind='bar')
plt.title('Average Performance by Backend')
plt.ylabel('Time (ms)')
plt.savefig('performance_by_backend.png')

# Plot scaling
single = df[df['backend_combo'] == 'CPU']
plt.figure(figsize=(12, 6))
plt.scatter(single['cores_0'], single['elapsed_ms'])
plt.title('CPU Scaling Analysis')
plt.xlabel('Cores')
plt.ylabel('Time (ms)')
plt.savefig('cpu_scaling.png')

# Plot thermal data
thermal = pd.read_csv('thermal_log.csv')
plt.figure(figsize=(12, 6))
plt.plot(thermal['timestamp'], thermal['cpu_temp'], label='CPU')
plt.plot(thermal['timestamp'], thermal['gpu_temp'], label='GPU')
plt.title('Thermal Profile')
plt.xlabel('Time (seconds)')
plt.ylabel('Temperature (°C)')
plt.legend()
plt.savefig('thermal_profile.png')

print('Plots saved to: performance_by_backend.png, cpu_scaling.png, thermal_profile.png')
EOF
```

### 2. Identify Optimal Configurations

```bash
# Find best configuration for each workload
python3 << 'EOF'
import pandas as pd

df = pd.read_csv('benchmark_results.csv')

# Group by workload and find minimum time
best = df.groupby(['model_type', 'tensor_type']).apply(
    lambda x: x.loc[x['elapsed_ms'].idxmin()]
)

print("Optimal configurations:")
print(best[['backend', 'cores_0', 'cores_1', 'elapsed_ms']])
EOF
```

### 3. Create Performance Baselines

Save the results as your performance baseline for future comparisons:

```bash
cp benchmark_results.csv benchmark_results_baseline.csv
cp thermal_log.csv thermal_log_baseline.csv
```

### 4. Document Findings

Create a summary document with:
- Performance characteristics of each backend
- Optimal core counts for each workload
- Thermal limits and throttling behavior
- Recommendations for production workloads

---

## File Locations

### Source Code
- **Benchmark**: `benchmark_all_backends.c`
- **Runner**: `scripts/run_all_backends_benchmark.sh`
- **Backend implementations**:
  - CPU: `cpu/vnni_cpu_backend.c`
  - GPU: `gpu/vulkan_backend.c`
  - NPU: `npu/xdna2_backend.c`

### Documentation
- **This guide**: `docs/multi-backend-benchmark-guide.md`
- **Test matrix**: `scripts/NPU_TEST_MATRIX.md`
- **Results**: `docs/npu-benchmark-results-2026-07-28.md`

### Output Files (after running)
- **Results**: `benchmark_results.csv`
- **Thermal**: `thermal_log.csv`
- **Plots**: `performance_by_backend.png`, `cpu_scaling.png`, `thermal_profile.png`

---

## Contact & Support

**Repository**: https://github.com/pleite/colibri  
**Issue Tracker**: https://github.com/pleite/colibri/issues  
**Hardware**: AMD Strix Halo (192.168.1.129)  
**SSH**: `ssh -i /opt/data/.ssh/id_ed25519 leite@192.168.1.129`

---

*Last updated: July 28, 2026*  
*Status: Ready for execution*