#!/bin/bash
#
# run_all_backends_benchmark.sh — build and run the placement/batching benchmark.
#
# Usage:
#   ./scripts/run_all_backends_benchmark.sh [--backend cpu|gpu|npu|all]
#                                           [--batch N] [--threads N]
#                                           [--iters N] [--csv FILE] [--thermal FILE]
#
# The benchmark now focuses on the knobs that affect placement and memory
# traffic: batch size, CPU thread count and the actual backend choice. It writes a
# CSV that can be consumed by analysis scripts on the target device.

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VNNI_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

cd "${VNNI_DIR}"

BACKEND="all"
BATCH=1
THREADS=1
ITERS=5
CSV_FILE="benchmark_results.csv"
THERMAL_FILE="thermal_log.csv"

resolve_path() {
    local target="$1"
    if [[ "$target" = /* ]]; then
        echo "$target"
    else
        echo "${VNNI_DIR}/${target}"
    fi
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --backend)
            BACKEND="$2"
            shift 2
            ;;
        --batch)
            BATCH="$2"
            shift 2
            ;;
        --threads)
            THREADS="$2"
            shift 2
            ;;
        --iters)
            ITERS="$2"
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
        --duration)
            ITERS="$2"
            shift 2
            ;;
        --help)
            echo "Usage: $0 [--backend cpu|gpu|npu|all] [--batch N] [--threads N] [--iters N] [--csv FILE] [--thermal FILE]"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

CSV_PATH="$(resolve_path "${CSV_FILE}")"
THERMAL_PATH="$(resolve_path "${THERMAL_FILE}")"
mkdir -p "$(dirname "${CSV_PATH}")"

printf '=== Placement / batching benchmark ===\n'
printf 'Working directory: %s\n' "${VNNI_DIR}"
printf 'Backend: %s\n' "${BACKEND}"
printf 'Batch size: %s\n' "${BATCH}"
printf 'CPU threads: %s\n' "${THREADS}"
printf 'Iterations: %s\n' "${ITERS}"
printf 'CSV output: %s\n' "${CSV_PATH}"
printf 'Thermal log: %s\n' "${THERMAL_PATH}"
printf '\n'

printf 'timestamp,backend,cpu_temp,gpu_temp\n' > "${THERMAL_PATH}"

make benchmark_all_backends >/dev/null

./benchmark_all_backends \
    --backend "${BACKEND}" \
    --batch "${BATCH}" \
    --threads "${THREADS}" \
    --iters "${ITERS}" \
    --csv "${CSV_PATH}"

printf '\nResults saved to %s\n' "${CSV_PATH}"
printf 'Thermal placeholder saved to %s\n' "${THERMAL_PATH}"
