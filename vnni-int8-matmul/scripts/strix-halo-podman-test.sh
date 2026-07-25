#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
vnni_dir="$(cd "${script_dir}/.." && pwd)"
repo_root="$(cd "${vnni_dir}/.." && pwd)"
container_image="${CONTAINER_IMAGE:-docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv}"
container_name="${CONTAINER_NAME:-colibri-vnni-strix-halo-test}"
log_path="${vnni_dir}/test_output.log"

if ! command -v podman >/dev/null 2>&1; then
    echo "podman is required for the Strix Halo Vulkan test harness" >&2
    exit 1
fi

if ! podman info >/dev/null 2>&1; then
    echo "podman is installed but not usable on this host" >&2
    exit 1
fi

if [[ ! -f "${vnni_dir}/Makefile" ]]; then
    echo "expected Makefile at ${vnni_dir}/Makefile" >&2
    exit 1
fi

if [[ -z "${DISPLAY:-}" ]] && [[ -z "${WAYLAND_DISPLAY:-}" ]]; then
    echo "This harness is intentionally Strix Halo-only and requires an active graphical session; no DISPLAY or WAYLAND_DISPLAY is set." >&2
    exit 1
fi

if [[ -n "${DISPLAY:-}" ]] && [[ ! -d /tmp/.X11-unix ]]; then
    echo "Expected X11 socket directory /tmp/.X11-unix for DISPLAY=${DISPLAY}" >&2
    exit 1
fi

if [[ -z "${VK_ICD_FILENAMES:-}" ]]; then
    VK_ICD_FILENAMES="/usr/share/vulkan/icd.d/radeon_icd.x86_64.json"
fi

if [[ ! -f "${VK_ICD_FILENAMES}" ]]; then
    echo "Expected AMD Vulkan ICD at ${VK_ICD_FILENAMES}" >&2
    exit 1
fi

if ! podman image exists "${container_image}" 2>/dev/null; then
    echo "Pulling Strix Halo Vulkan container image: ${container_image}"
    podman pull "${container_image}"
fi

podman_args=(
    run --rm
    --name "${container_name}"
    --device /dev/dri
    --device /dev/kfd
    --security-opt label=disable
    --cap-add=SYS_PTRACE
    --env VK_ICD_FILENAMES="${VK_ICD_FILENAMES}"
    --env VNNI_VULKAN_DEBUG=1
    --env DISPLAY="${DISPLAY:-}"
    --env WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-}"
    --env XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-}"
    --volume "${vnni_dir}:/work/vnni-int8-matmul:rw,z"
    --workdir /work/vnni-int8-matmul
)

if [[ -n "${DISPLAY:-}" ]]; then
    podman_args+=(--volume /tmp/.X11-unix:/tmp/.X11-unix:ro)
fi

if [[ -n "${XAUTHORITY:-}" ]] && [[ -f "${XAUTHORITY}" ]]; then
    podman_args+=(--env XAUTHORITY="${XAUTHORITY}" --volume "${XAUTHORITY}:${XAUTHORITY}:ro")
fi

if [[ -n "${XDG_RUNTIME_DIR:-}" ]] && [[ -d "${XDG_RUNTIME_DIR}" ]]; then
    podman_args+=(--volume "${XDG_RUNTIME_DIR}:${XDG_RUNTIME_DIR}:rw")
fi

podman_args+=("${container_image}" bash -lc 'set -euo pipefail; ln -sf /usr/bin/ld.bfd /etc/alternatives/ld && ln -sf /usr/bin/ld.bfd /usr/bin/ld && make clean && make && make test')

: > "${log_path}"
printf 'Running Strix Halo Vulkan Podman harness with image %s\n' "${container_image}" | tee "${log_path}"
{
    podman "${podman_args[@]}"
} 2>&1 | tee -a "${log_path}"
