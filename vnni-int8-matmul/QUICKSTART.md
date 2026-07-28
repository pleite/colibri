# Quick Start — Multi-Backend Benchmark

## Run the Benchmark

```bash
cd /home/leite/colibri/vnni-int8-matmul

# Quick test (30 seconds per config, ~2-3 hours total)
./scripts/run_all_backends_benchmark.sh

# Stress test (2 minutes per config, ~8-10 hours total)
./scripts/run_all_backends_benchmark.sh --duration 120

# Thermal soak (5 minutes per config, ~20+ hours total)
./scripts/run_all_backends_benchmark.sh --duration 300
```

## What It Does

Tests **CPU, GPU, and NPU** in all combinations:
- **Single**: CPU, GPU, NPU (1 to max cores)
- **Dual**: CPU+GPU, CPU+NPU, GPU+NPU
- **Triple**: CPU+GPU+NPU
- **15 workload shapes** × all combinations
- **~4550 total tests**
- **Real-time thermal monitoring**
- **CSV output** for analysis

## Output Files

After running, you'll have:
- `benchmark_results.csv` — All test results (4550+ rows)
- `thermal_log.csv` — Temperature readings over time

## Analyze Results

```bash
# Quick summary
head benchmark_results.csv

# Performance by backend
python3 -c "
import pandas as pd
df = pd.read_csv('benchmark_results.csv')
print(df.groupby('backend')['elapsed_ms'].mean())
"

# Find thermal throttling
python3 -c "
import pandas as pd
df = pd.read_csv('benchmark_results.csv')
print(df[df['thermal_throttled'] == 1])
"
```

## Files Created

- `benchmark_all_backends.c` — Main benchmark executable
- `scripts/run_all_backends_benchmark.sh` — Runner script
- `docs/multi-backend-benchmark-guide.md` — Complete guide
- `QUICKSTART.md` — This file

## Full Documentation

See `docs/multi-backend-benchmark-guide.md` for:
- Complete test matrix
- Analysis examples
- Troubleshooting
- Expected results
- Next steps

---

**Ready to run!** Just execute the command above and let it go.