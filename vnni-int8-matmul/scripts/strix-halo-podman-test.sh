#!/usr/bin/env bash
#
# strix-halo-podman-test.sh — build and run the Strix Halo building blocks
# inside a headless Podman container.
#
# The container is built from Dockerfile.strix-halo-test, which guarantees the
# build toolchain (ld, amdxdna kernel UAPI headers, Vulkan headers) on top of a
# base image. The default base is this repository's own colibri-vulkan image;
# the community Strix Halo toolbox image also works as a base:
#
#   BASE_IMAGE=docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv make podman-test
#
# The toolbox image alone cannot compile this tree — it has no linker and no
# kernel headers. Do not work around that with an ad-hoc
# `ln -sf /usr/bin/ld.bfd /usr/bin/ld` in the test step; fix the image.
#
# Headless by design: the Vulkan backend requests zero instance extensions, so
# no X11 socket, no Wayland socket, no DISPLAY and no Xvfb are needed. Passing a
# display server into the container is not a fix for anything and is therefore
# deliberately not done here.
#
# Devices:
#   /dev/dri          RADV GFX1151 iGPU (Vulkan compute)
#   /dev/kfd          ROCm compute node (present on Strix Halo, used by HIP)
#   /dev/accel/accel0 XDNA 2 NPU (amdxdna accel node)
#
# Environment overrides:
#   BASE_IMAGE        upstream toolbox image the harness image is built from
#   CONTAINER_IMAGE   run this image as-is instead of building one
#   CONTAINER_NAME    container name
#   VNNI_VULKAN_ICD   path to the RADV ICD manifest inside the container
#   SKIP_NPU=1        do not require/pass the NPU device
#   REQUIRE_NPU=1     fail instead of warning when the NPU node is absent

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
vnni_dir="$(cd "${script_dir}/.." && pwd)"
repo_dir="$(cd "${vnni_dir}/.." && pwd)"
base_image="${BASE_IMAGE:-ghcr.io/pleite/colibri-vulkan:latest}"
harness_image="${CONTAINER_IMAGE:-localhost/colibri-strix-halo-test:latest}"
container_name="${CONTAINER_NAME:-colibri-vnni-strix-halo-test}"
log_path="${vnni_dir}/test_output.log"
container_vulkan_icd="${VNNI_VULKAN_ICD:-/usr/share/vulkan/icd.d/radeon_icd.x86_64.json}"
npu_device="/dev/accel/accel0"

if ! command -v podman >/dev/null 2>&1; then
    echo "podman is required for the Strix Halo test harness" >&2
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
        echo "this harness expects ${required_device} on the Strix Halo host" >&2
        exit 1
    fi
done

npu_args=()
if [[ "${SKIP_NPU:-0}" != "1" ]]; then
    if [[ -e "${npu_device}" ]]; then
        npu_args+=(--device "${npu_device}")
    elif [[ "${REQUIRE_NPU:-0}" == "1" ]]; then
        echo "REQUIRE_NPU=1 but ${npu_device} does not exist" >&2
        echo "load the amdxdna module (Linux >= 6.14) and retry" >&2
        exit 1
    else
        echo "warning: ${npu_device} not found; NPU tests will report SKIP." >&2
        echo "         load the amdxdna module, or set SKIP_NPU=1 to silence this." >&2
    fi
fi

if [[ -n "${CONTAINER_IMAGE:-}" ]]; then
    if ! podman image exists "${harness_image}" 2>/dev/null; then
        echo "Pulling ${harness_image}"
        podman pull "${harness_image}"
    fi
else
    echo "Building harness image ${harness_image} from ${base_image}"
    podman build \
        --build-arg "BASE_IMAGE=${base_image}" \
        -f "${vnni_dir}/Dockerfile.strix-halo-test" \
        -t "${harness_image}" \
        "${vnni_dir}"
fi

podman_args=(
    run --rm
    --name "${container_name}"
    --device /dev/dri
    --device /dev/kfd
    "${npu_args[@]+"${npu_args[@]}"}"
    --security-opt label=disable
    --cap-add=SYS_PTRACE
    --env VK_ICD_FILENAMES="${container_vulkan_icd}"
    --env VNNI_VULKAN_DEBUG=1
    --env XDNA2_VERBOSE=1
    --env HSA_OVERRIDE_GFX_VERSION=11.5.1
    --env GGML_HIP_ENABLE_UNIFIED_MEMORY=1
    # The repository root is mounted because the NPU backend compiles the
    # XDNA 2 driver from c/npu_kernels/.
    --volume "${repo_dir}:/work:rw,z"
    --workdir /work/vnni-int8-matmul
)

podman_args+=("${harness_image}" bash -lc '
set -euo pipefail
echo "--- toolchain ---"
gcc --version | head -1
command -v ld >/dev/null || {
    echo "no linker in the image; rebuild it from Dockerfile.strix-halo-test" >&2
    exit 1
}
ld --version | head -1
echo "--- kernel uapi ---"
test -f /usr/include/drm/amdxdna_accel.h || {
    echo "<drm/amdxdna_accel.h> missing; install kernel-headers >= 6.14 in the image" >&2
    exit 1
}
echo "amdxdna UAPI present"
echo "--- vulkan loader ---"
ls -l /usr/lib64/libvulkan.so.1 /usr/lib/x86_64-linux-gnu/libvulkan.so.1 2>/dev/null || true
test -f "${VK_ICD_FILENAMES}" || { echo "ICD manifest ${VK_ICD_FILENAMES} not found" >&2; exit 1; }
echo "--- devices ---"
ls -l /dev/dri /dev/kfd 2>/dev/null || true
ls -l /dev/accel 2>/dev/null || echo "no /dev/accel (NPU tests will skip)"
echo "--- build ---"
make clean
make
echo "--- test ---"
make test
')

: > "${log_path}"
printf 'Running headless Strix Halo Podman harness with image %s\n' "${harness_image}" | tee "${log_path}"
{
    podman "${podman_args[@]}"
} 2>&1 | tee -a "${log_path}"

# The repository root is bind-mounted into the container, so anything the
# container removes under it removes the host's copy too. `tee` keeps writing to
# an unlinked inode without complaining, which used to leave CI with a passing
# test run and no log at all.
if [[ ! -f "${log_path}" ]]; then
    echo "the harness log ${log_path} was deleted while the run was in progress" >&2
    echo "nothing running inside the container may remove it (see the 'clean' target)" >&2
    exit 1
fi
