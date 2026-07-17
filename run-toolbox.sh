#!/bin/bash
# Colibri toolbox launcher — GPU-enabled container with host /opt/models mounted.
#
# Usage:
#   ./run-toolbox.sh                          # enter container interactively
#   ./run-toolbox.sh --model /opt/models/ornith-int8 --prompt "hello"
#   ./run-toolbox.sh --help
#
# The binary is at /opt/colibri/c/qwen35_moe inside the container.
# GPU devices (/dev/dri, /dev/kfd) are passed through automatically.
# /opt/models from the host is bind-mounted read-only so your model files are visible.

set -euo pipefail

CONTAINER="colibri-toolbox"
BINARY="/opt/colibri/c/qwen35_moe"
MODELS_DIR="/opt/models"

# If no args, just enter the container interactively
if [ $# -eq 0 ]; then
    podman exec -it "$CONTAINER" /bin/bash
    exit 0
fi

# Run the binary with the provided arguments
podman run --rm \
    --name "${CONTAINER}-run" \
    --privileged \
    --device /dev/dri \
    --device /dev/kfd \
    -v "${MODELS_DIR}:${MODELS_DIR}:ro" \
    -v /tmp/.X11-unix:/tmp/.X11-unix:ro \
    -v /run/user/1000:/run/user/1000:rw \
    -e HSA_OVERRIDE_GFX_VERSION=11.5.1 \
    ghcr.io/pleite/colibri-all:latest \
    "$BINARY" "$@"
