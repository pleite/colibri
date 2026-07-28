# Multi-Backend Placement / Batching Benchmark

**Status**: Ready for on-device use

This harness is the measurement step that complements the placement policy. It gives
an explicit CSV output for the knobs that matter when tuning this machine:

- backend choice (`cpu`, `gpu`, `npu`, or `all`)
- matrix shape (`rows`, `inner_dim`, `out_cols`)
- batch size (how many matmuls are issued in one measurement window)
- CPU thread count (for the multi-batch CPU sweep)
- an estimate of the memory traffic implied by the measured batch

The output format is:

```csv
backend,rows,inner_dim,out_cols,batch_size,threads,iters,elapsed_ms,success,bytes_processed,estimated_gib_s
```

## Build

```bash
cd /home/runner/work/colibri/colibri/vnni-int8-matmul
make benchmark_all_backends
```

On a host without Vulkan headers or the amdxdna UAPI headers the binary still
builds, but the GPU and NPU backends are reported as unavailable so the CPU path
can still be measured.

## Manual run on the target device

Run the harness directly for a compact sweep:

```bash
./benchmark_all_backends --backend all --batch 1 --threads 1 --iters 5 --csv placement_benchmark.csv
```

For a more useful placement experiment, sweep the batch and thread knobs:

```bash
./benchmark_all_backends --backend cpu --batch 1 --threads 1 --iters 5 --csv cpu_batch1.csv
./benchmark_all_backends --backend cpu --batch 4 --threads 1 --iters 5 --csv cpu_batch4.csv
./benchmark_all_backends --backend cpu --batch 8 --threads 4 --iters 5 --csv cpu_batch8_threads4.csv
```

The wrapper script provides the same flow in one command:

```bash
./scripts/run_all_backends_benchmark.sh --backend all --batch 1 --threads 1 --iters 5 \
    --csv placement_benchmark.csv --thermal thermal_log.csv
```

## What to look for

- `elapsed_ms` shows how a batch behaves for each shape.
- `estimated_gib_s` is a simple traffic estimate from the bytes that would be
  touched by the batch. It is useful for spotting when a shape is limited by
  memory bandwidth rather than arithmetic throughput.
- A larger batch can reveal whether the backend benefits from amortized fixed
  costs; a larger CPU thread count can show whether the host-side batch loop is
  scaling effectively.
- When the GPU or NPU backend is available on the target device, the same CSV can
  be used to compare placement decisions against the CPU-only baseline.

## Quick analysis

```bash
python3 - <<'PY'
import csv
from statistics import mean
with open('placement_benchmark.csv') as fh:
    rows = list(csv.DictReader(fh))
for backend in sorted({r['backend'] for r in rows}):
    subset = [r for r in rows if r['backend'] == backend]
    print(backend, 'rows=', len(subset))
    print('  median elapsed_ms=', sorted(float(r['elapsed_ms']) for r in subset)[len(subset)//2])
PY
```

## Notes

- The benchmark uses the same shape set the placement policy already reasons
  about; it is intended to be run on the hardware being tuned, not on a separate
  build machine.
- If the target device has the full Vulkan and amdxdna toolchain installed, the
  GPU and NPU rows will be measured directly. If not, they are reported as skipped
  so the CPU measurement path remains usable.
