#!/usr/bin/env bash
set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
vnni_dir="$(cd "${script_dir}/.." && pwd)"
container_image="${CONTAINER_IMAGE:-docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv}"
container_name="${CONTAINER_NAME:-colibri-vnni-strix-halo-test}"
log_path="${vnni_dir}/test_output.log"
container_vulkan_icd="${VNNI_VULKAN_ICD:-/usr/share/vulkan/icd.d/radeon_icd.x86_64.json}"

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

for required_device in /dev/dri /dev/kfd; do
    if [[ ! -e "${required_device}" ]]; then
        echo "this harness expects ${required_device} to be available on the Strix Halo host" >&2
        exit 1
    fi
done

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
    --env VK_ICD_FILENAMES="${container_vulkan_icd}"
    --env VNNI_VULKAN_DEBUG=1
    --env HSA_OVERRIDE_GFX_VERSION=11.5.1
    --env GGML_HIP_ENABLE_UNIFIED_MEMORY=1
    --env DISPLAY="${DISPLAY:-}"
    --env WAYLAND_DISPLAY="${WAYLAND_DISPLAY:-}"
    --env XDG_RUNTIME_DIR="${XDG_RUNTIME_DIR:-}"
    --volume "${vnni_dir}:/work/vnni-int8-matmul:rw,z"
    --workdir /work/vnni-int8-matmul
)

if [[ -n "${DISPLAY:-}" ]] && [[ -d /tmp/.X11-unix ]]; then
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
