#!/usr/bin/env bash
#
# build-npu-kernels.sh — build every enumerated XDNA 2 kernel artifact in a
# Podman container.
#
# The shape set comes from sched/npu_shapes.c (through tools/npu_shapes_list),
# so this script never carries its own copy of the enumeration. For each shape
# it compiles the AIE design with the pinned IRON/Peano toolchain and packs the
# resulting instruction stream into npu/kernels/<name>.npukernel, next to the
# xclbin it was compiled with.
#
# The toolchain is a build-only image (npu/aie/Containerfile.aie-toolchain): no
# NPU device is passed in and none is needed. Kernels are artifacts; running
# them is what scripts/strix-halo-podman-test.sh does.
#
# Usage:
#   ./scripts/build-npu-kernels.sh [--allow-partial] [--image-only] [shape ...]
#
# A shape argument is "rows x inner x out" (e.g. 256x4096x1024) and restricts
# the build to that shape.
#
# Environment overrides:
#   CONTAINER_IMAGE   run this toolchain image instead of building one
#   TOOLCHAIN_IMAGE   tag to build/use (default localhost/colibri-aie-toolchain:latest)
#   KERNEL_DIR        output directory (default npu/kernels)
#   PODMAN            container CLI (default podman)
#
# Every shape is recorded in npu/kernels/manifest.json with its status. A shape
# that failed to compile always fails the run; a shape no tiling of the upstream
# designs can express is reported as "unsupported" and fails the run too, unless
# --allow-partial is given. No artifact is ever emitted for a shape that was not
# compiled for it: the runtime refuses a missing shape with -ENOENT, which is
# the documented behaviour, and a wrong artifact is not.

set -euo pipefail

script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
vnni_dir="$(cd "${script_dir}/.." && pwd)"
repo_dir="$(cd "${vnni_dir}/.." && pwd)"
aie_dir="${vnni_dir}/npu/aie"
lock_file="${aie_dir}/toolchain.lock"

podman_bin="${PODMAN:-podman}"
toolchain_image="${TOOLCHAIN_IMAGE:-localhost/colibri-aie-toolchain:latest}"
kernel_dir="${KERNEL_DIR:-npu/kernels}"
allow_partial=0
image_only=0
declare -a wanted_shapes=()

while [[ $# -gt 0 ]]; do
    case "$1" in
        --allow-partial) allow_partial=1 ;;
        --image-only) image_only=1 ;;
        -h|--help) sed -n '2,40p' "${BASH_SOURCE[0]}"; exit 0 ;;
        -*) echo "unknown option: $1" >&2; exit 2 ;;
        *) wanted_shapes+=("$1") ;;
    esac
    shift
done

if ! command -v "${podman_bin}" >/dev/null 2>&1; then
    echo "${podman_bin} is required to build the NPU kernels" >&2
    exit 1
fi
if [[ ! -f "${lock_file}" ]]; then
    echo "missing toolchain pin file ${lock_file}" >&2
    exit 1
fi

# ── Toolchain image ──────────────────────────────────────────────────────────

if [[ -n "${CONTAINER_IMAGE:-}" ]]; then
    toolchain_image="${CONTAINER_IMAGE}"
    if ! "${podman_bin}" image exists "${toolchain_image}" 2>/dev/null; then
        echo "Pulling ${toolchain_image}"
        "${podman_bin}" pull "${toolchain_image}"
    fi
else
    declare -a build_args=()
    while IFS='=' read -r key value; do
        [[ -z "${key}" || "${key}" == \#* ]] && continue
        build_args+=(--build-arg "${key}=${value}")
    done < "${lock_file}"

    echo "Building AIE toolchain image ${toolchain_image}"
    "${podman_bin}" build \
        "${build_args[@]}" \
        -f "${aie_dir}/Containerfile.aie-toolchain" \
        -t "${toolchain_image}" \
        "${aie_dir}"
fi

# ── Kernel build ─────────────────────────────────────────────────────────────

if [[ "${image_only}" == "1" ]]; then
    echo "Toolchain image ${toolchain_image} is ready; skipping the kernel build"
    exit 0
fi

echo "Building kernel artifacts into ${kernel_dir}"
"${podman_bin}" run --rm \
    --user root \
    --security-opt label=disable \
    --volume "${repo_dir}:/work:rw,z" \
    --workdir /work/vnni-int8-matmul \
    --env "KERNEL_DIR=${kernel_dir}" \
    --env "ALLOW_PARTIAL=${allow_partial}" \
    --env "WANTED_SHAPES=${wanted_shapes[*]:-}" \
    "${toolchain_image}" \
    bash -lc '
set -euo pipefail

echo "--- toolchain ---"
python --version
"${PEANO_INSTALL_DIR}/bin/clang" --version | head -1
xclbinutil --version 2>/dev/null | head -1 || true
ert-opcode-probe

echo "--- shape set ---"
make tools/npu_shapes_list
./tools/npu_shapes_list

python3 npu/aie/pack_npukernel.py --self-test
exec python3 npu/aie/build_all.py \
    --kernel-dir "${KERNEL_DIR}" \
    --shapes "${WANTED_SHAPES}" \
    $( [ "${ALLOW_PARTIAL}" = "1" ] && echo --allow-partial )
'
