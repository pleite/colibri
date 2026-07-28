# Quick Start — Placement and Batching Benchmark

## Run the benchmark on the target device

```bash
cd /home/runner/work/colibri/colibri/vnni-int8-matmul

# Build the harness
make benchmark_all_backends

# A compact first run
./scripts/run_all_backends_benchmark.sh --backend all --batch 1 --threads 1 --iters 5 \
    --csv placement_benchmark.csv --thermal thermal_log.csv
```

## Useful follow-up sweeps

```bash
# Compare batch sizes on CPU
./benchmark_all_backends --backend cpu --batch 1 --threads 1 --iters 5 --csv cpu_batch1.csv
./benchmark_all_backends --backend cpu --batch 4 --threads 1 --iters 5 --csv cpu_batch4.csv

# Compare CPU thread counts with a larger batch
./benchmark_all_backends --backend cpu --batch 8 --threads 4 --iters 5 --csv cpu_batch8_threads4.csv
```

## Output files

- `placement_benchmark.csv` — measured results for shape / backend / batch / thread settings
- `thermal_log.csv` — a placeholder file produced by the wrapper script

## Analysis

The CSV contains the measurements needed for placement decisions and memory-traffic analysis:

```bash
head placement_benchmark.csv
```

## Full documentation

See `docs/multi-backend-benchmark-guide.md` for the full workflow and interpretation notes.
