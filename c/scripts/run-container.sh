#!/usr/bin/env bash
set -euo pipefail

RUNTIME="${CONTAINER_RUNTIME:-podman}"
IMAGE="${IMAGE:-}"
NAME="${NAME:-}"
PORT="${PORT:-8000}"
CONTAINER_PORT="${CONTAINER_PORT:-8000}"
BACKEND="${BACKEND:-npu}"
MODEL_DIR="${MODEL_DIR:-}"
MODEL_MOUNT="${MODEL_MOUNT:-/models}"
MEMORY="${MEMORY:-}"
CPUS="${CPUS:-}"

validate_port() {
    local value="$1"
    if [[ ! "$value" =~ ^[0-9]+$ ]]; then
        echo "error: port must be a numeric value" >&2
        exit 2
    fi
    if (( value < 1 || value > 65535 )); then
        echo "error: port must be between 1 and 65535" >&2
        exit 2
    fi
}

validate_model_dir() {
    local dir="$1"
    if [[ ! -d "$dir" ]]; then
        echo "error: model directory not found: $dir" >&2
        exit 1
    fi
    # A usable model directory should contain at least one of the expected runtime artifacts.
    local match
    match=$(find "$dir" \( -name '*.safetensors' -o -name 'config.json' -o -name 'tokenizer.json' \) -print -quit)
    if [[ -z "$match" ]]; then
        echo "warning: model directory missing expected files (config.json, tokenizer.json, *.safetensors): $dir" >&2
    fi
}

usage() {
    cat <<'EOF'
Usage: ./c/scripts/run-container.sh [--image IMAGE] [--name NAME] [--port PORT] [--backend BACKEND] [--model-dir DIR]

Optional flags:
  --image    Container image to run (defaults to a backend-specific image)
  --name     Container name
  --port     Host port to publish (defaults to 8000)
  --backend  Value for COLI_ACCEL (defaults to npu)
  --model-dir Mount a host model directory into /models and set COLI_MODEL=/models
  --memory   Container memory limit (for example 12g)
  --cpus     Container CPU count (for example 4)

Examples:
  c/scripts/run-container.sh
  c/scripts/run-container.sh --backend cpu --image ghcr.io/pleite/colibri-cpu:latest --model-dir /path/to/your-model
  c/scripts/run-container.sh --name colibri-npu --port 8080 --backend npu --model-dir /path/to/your-model
EOF
}

usage_error() {
    usage >&2
}

validate_backend() {
    local value="$1"
    case "$value" in
        cpu|cuda|rocm|vulkan|npu|auto)
            ;;
        *)
            echo "error: unsupported backend '$value'" >&2
            exit 2
            ;;
    esac
}

while [[ $# -gt 0 ]]; do
    case "$1" in
        --image)
            IMAGE="$2"
            shift 2
            ;;
        --name)
            NAME="$2"
            shift 2
            ;;
        --port)
            PORT="$2"
            shift 2
            ;;
        --backend)
            BACKEND="$2"
            shift 2
            ;;
        --model-dir)
            MODEL_DIR="$2"
            shift 2
            ;;
        --memory)
            MEMORY="$2"
            shift 2
            ;;
        --cpus)
            CPUS="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "error: unknown argument: $1" >&2
            usage_error
            exit 2
            ;;
    esac
done

if [ -z "$IMAGE" ]; then
    case "$BACKEND" in
        cpu) IMAGE="ghcr.io/pleite/colibri-cpu:latest" ;;
        vulkan) IMAGE="ghcr.io/pleite/colibri-vulkan:latest" ;;
        rocm) IMAGE="ghcr.io/pleite/colibri-rocm:latest" ;;
        npu) IMAGE="ghcr.io/pleite/colibri-npu:latest" ;;
        all) IMAGE="ghcr.io/pleite/colibri-all:latest" ;;
        auto) IMAGE="ghcr.io/pleite/colibri-cpu:latest" ;;
        *)
            echo "error: unsupported backend '$BACKEND'" >&2
            exit 2
            ;;
    esac
fi

if [ -z "$NAME" ]; then
    NAME="colibri-$BACKEND"
fi

if ! command -v "$RUNTIME" >/dev/null 2>&1; then
    echo "error: container runtime '$RUNTIME' is not installed" >&2
    exit 1
fi

validate_port "$PORT"
validate_backend "$BACKEND"

run_args=(run --rm -d --name "$NAME")
run_args+=(--publish "${PORT}:${CONTAINER_PORT}")
run_args+=(--env "COLI_ACCEL=$BACKEND")

if [ -n "$MEMORY" ]; then
    run_args+=(--memory "$MEMORY")
fi

if [ -n "$CPUS" ]; then
    run_args+=(--cpus "$CPUS")
fi

if [ "$BACKEND" != "cpu" ] && [ "$BACKEND" != "auto" ]; then
    # Keep host group memberships inside the container so the mounted device nodes remain accessible.
    run_args+=(--group-add keep-groups)

    for host_path in /dev/accel /dev/dri /dev/kfd; do
        if [ -e "$host_path" ]; then
            run_args+=(--mount "type=bind,src=$host_path,dst=$host_path")
        fi
    done

    if [ -d /sys/bus/accel ]; then
        run_args+=(--mount "type=bind,src=/sys/bus/accel,dst=/sys/bus/accel,readonly")
    fi
fi

if [ -n "$MODEL_DIR" ]; then
    validate_model_dir "$MODEL_DIR"
    run_args+=(--mount "type=bind,src=$MODEL_DIR,dst=$MODEL_MOUNT,ro")
    run_args+=(--env "COLI_MODEL=$MODEL_MOUNT")
fi

run_args+=("$IMAGE")

printf 'starting container with image %s and runtime %s\n' "$IMAGE" "$RUNTIME"
"$RUNTIME" "${run_args[@]}"
