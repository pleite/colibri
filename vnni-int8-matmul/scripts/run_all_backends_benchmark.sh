#!/bin/bash
#
# run_all_backends_benchmark.sh — Run comprehensive multi-backend benchmark
#
# Tests CPU, GPU, and NPU in all combinations with thermal monitoring
#
# Usage:
#   ./scripts/run_all_backends_benchmark.sh [--duration SECONDS] [--csv FILE] [--thermal FILE]
#
# Examples:
#   ./scripts/run_all_backends_benchmark.sh
#   ./scripts/run_all_backends_benchmark.sh --duration 60
#   ./scripts/run_all_backends_benchmark.sh --duration 300 --csv results.csv

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VNNI_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REPO_DIR="$(cd "${VNNI_DIR}/../.." && pwd)"

cd "${VNNI_DIR}"

# Default parameters
DURATION="${1:-30}"
CSV_FILE="${2:-benchmark_results.csv}"
THERMAL_FILE="${3:-thermal_log.csv}"

# Parse arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        --duration)
            DURATION="$2"
            shift 2
            ;;
        --csv)
            CSV_FILE="$2"
            shift 2
            ;;
        --thermal)
            THERMAL_FILE="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [--duration SECONDS] [--csv FILE] [--thermal FILE]"
            echo ""
            echo "Options:"
            echo "  --duration SECONDS  Test duration per config (default: 30)"
            echo "  --csv FILE          CSV output file (default: benchmark_results.csv)"
            echo "  --thermal FILE      Thermal log file (default: thermal_log.csv)"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "=== Multi-Backend Benchmark Suite ==="
echo "Repository: ${REPO_DIR}"
echo "Working directory: ${VNNI_DIR}"
echo "Duration per test: ${DURATION} seconds"
echo "CSV output: ${VNNI_DIR}/${CSV_FILE}"
echo "Thermal log: ${VNNI_DIR}/${THERMAL_FILE}"
echo ""

# Check NPU device
if [[ ! -e /dev/accel/accel0 ]]; then
    echo "WARNING: NPU device /dev/accel/accel0 not found"
    echo "NPU tests will be skipped"
fi

# Build the benchmark executable
echo "Building benchmark_all_backends..."
make clean > /dev/null 2>&1
make all > /dev/null 2>&1

if [[ ! -f benchmark_all_backends ]]; then
    echo "Error: Failed to build benchmark_all_backends"
    exit 1
fi

echo "Build successful"
echo ""

# Run the benchmark
echo "Starting benchmark suite..."
echo "Press Ctrl+C to stop gracefully"
echo ""

./benchmark_all_backends \
    --duration "${DURATION}" \
    --csv "${CSV_FILE}" \
    --thermal "${THERMAL_FILE}"

EXIT_CODE=$?

echo ""
echo "=== Benchmark Complete ==="
echo "Exit code: ${EXIT_CODE}"

if [[ ${EXIT_CODE} -eq 0 ]]; then
    echo ""
    echo "Results saved to:"
    echo "  - ${VNNI_DIR}/${CSV_FILE}"
    echo "  - ${VNNI_DIR}/${THERMAL_FILE}"
    echo ""
    echo "To analyze results:"
    echo "  cat ${CSV_FILE} | head -20"
    echo "  python3 -c \"import pandas as pd; df = pd.read_csv('${CSV_FILE}'); print(df.describe())\""
    echo ""
    echo "Thermal analysis:"
    echo "  cat ${THERMAL_FILE} | head -20"
    echo "  python3 -c \"import pandas as pd; df = pd.read_csv('${THERMAL_FILE}'); print(df.describe())\""
else
    echo ""
    echo "Benchmark failed with exit code ${EXIT_CODE}"
fi

exit ${EXIT_CODE}