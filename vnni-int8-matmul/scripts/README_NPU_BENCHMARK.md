# NPU Full Benchmark Suite

Comprehensive benchmark for AMD XDNA 2 NPU on Strix Halo.

## What it tests

- **All supported shapes**: 10 different matrix dimensions
- **Core types**: Single-core (32-row) and whole-array (256-row) execution
- **Execution modes**: Sequential, parallel, and mixed parallelism
- **Accuracy validation**: Compares NPU results against CPU reference
- **Performance metrics**: Timing for each execution mode

## Supported shapes

### Whole-array (256-row tiles) - 32 cores
- 256×4096×1024 (expert gate_proj/up_proj)
- 256×1024×4096 (expert down_proj)
- 256×4096×16384 (self_attn q_proj)
- 256×4096×512 (self_attn k_proj/v_proj)
- 256×8192×4096 (self_attn o_proj)

### Single-core (32-row tiles) - 1 core each
- 32×4096×1024 (expert gate_proj/up_proj)
- 32×1024×4096 (expert down_proj)
- 32×4096×16384 (self_attn q_proj)
- 32×4096×512 (self_attn k_proj/v_proj)
- 32×8192×4096 (self_attn o_proj)

## Benchmark phases

1. **Sequential**: Run each shape one at a time
2. **Parallel**: Run all 10 shapes back-to-back (measures throughput)
3. **Mixed**: Run shapes in pairs (2 at a time)
4. **All types**: Run all shapes simultaneously (maximum parallelism)

## Usage

```bash
# From the vnni-int8-matmul directory
./scripts/npu_full_benchmark.sh
```

## Output

- **npu_benchmark_results.csv**: Machine-readable CSV with all test results
- **npu_benchmark_results.csv.txt**: Human-readable full output

### CSV format
```csv
rows,inner_dim,out_cols,core_type,test_type,parallel_count,elapsed_ms,accuracy_ok
256,4096,1024,whole,sequential,1,12.34,1
32,4096,1024,single,sequential,1,4.56,1
...
```

## Requirements

- Strix Halo hardware with XDNA 2 NPU
- amdxdna kernel module loaded
- NPU kernels built (run `make npu-kernels` if needed)
- Root or render group access to /dev/accel/accel0

## Notes

- Each test uses random but deterministic input data (seed=42)
- Accuracy threshold: 1.0 float difference tolerated
- Timing includes kernel loading overhead (first run only)
- Results are cached in memory for parallel tests

## Example output

```
=== NPU Matmul Benchmark Suite ===
Testing 10 shapes across different execution modes

Phase 1: Sequential execution
  Testing 256x4096x1024 (whole core)... 12.34 ms, accuracy: PASS
  Testing 256x1024x4096 (whole core)... 8.76 ms, accuracy: PASS
  ...

Phase 2: Parallel execution (all shapes)
  Running 10 shapes in parallel...
  Total parallel time: 95.23 ms (avg 9.52 ms per shape)
  ...

=== Benchmark Summary ===
Total tests: 40
Tests passed: 40
```

## Troubleshooting

- **Kernel not found**: Run `make npu-kernels` to build kernels
- **Missing `DRM_IOCTL_AMDXDNA_CONFIG_HWCTX`**: host kernel/UAPI is too old for CU/PDI registration; use amdxdna headers and kernel >= 6.14 with `CONFIG_DRM_AMDXDNA`
- **Permission denied**: Add user to `render` group or run as root
- **Module not loaded**: `sudo modprobe amdxdna`
