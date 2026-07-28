#!/usr/bin/env bash
#
# capture_debug_snapshot.sh — collect host/container diagnostics for VNNI runs.
#
# Usage:
#   ./scripts/capture_debug_snapshot.sh [output-file]
#
# When output-file is omitted, the report is printed to stdout.

set -uo pipefail

out_path="${1:-}"
if [[ -n "${out_path}" ]]; then
    mkdir -p "$(dirname "${out_path}")"
    exec >"${out_path}" 2>&1
fi

run() {
    local title="$1"
    shift
    echo
    echo "=== ${title} ==="
    "$@" 2>&1 || true
}

echo "snapshot_time_utc: $(date -u +'%Y-%m-%dT%H:%M:%SZ')"
echo "hostname: $(hostname 2>/dev/null || echo unknown)"
echo "whoami: $(id -un 2>/dev/null || echo unknown)"
echo "pwd: $(pwd)"

run "kernel" uname -a
run "os-release" cat /etc/os-release
run "cpu-model" bash -lc "grep -m1 'model name' /proc/cpuinfo"
run "cpu-flags-first-line" bash -lc "grep -m1 '^flags' /proc/cpuinfo"
run "memory" free -h
run "lsmod amd modules" bash -lc "lsmod | grep -E 'amdxdna|amdgpu|kfd|drm' || true"
run "modinfo amdxdna" modinfo amdxdna
run "device nodes" bash -lc "ls -la /dev/dri /dev/kfd /dev/accel 2>/dev/null || true"
run "accel sysfs" bash -lc "ls -la /sys/bus/accel/devices 2>/dev/null || true"
run "drm render nodes" bash -lc "ls -la /dev/dri/renderD* 2>/dev/null || true"
run "kernel config DRM_AMDXDNA" bash -lc "zcat /proc/config.gz 2>/dev/null | grep -E 'CONFIG_DRM_AMDXDNA|CONFIG_DRM_ACCEL' || true"
run "uapi header presence" bash -lc "ls -la /usr/include/drm/amdxdna_accel.h 2>/dev/null || echo missing"
run "uapi ioctl symbols" bash -lc "grep -nE 'DRM_IOCTL_AMDXDNA_CONFIG_HWCTX|DRM_AMDXDNA_HWCTX_CONFIG_CU|DRM_AMDXDNA_QUERY_RESOURCE_INFO' /usr/include/drm/amdxdna_accel.h 2>/dev/null || true"
run "toolchain" bash -lc "gcc --version | head -n1; ld --version | head -n1; clang --version | head -n1 || true"
run "build tools" bash -lc "make --version | head -n1; cmake --version | head -n1 || true; python3 --version || true"
run "podman/docker" bash -lc "podman --version || true; docker --version || true"
run "vulkan userspace" bash -lc "ls -la /usr/include/vulkan/vulkan.h 2>/dev/null || true; ls -la /usr/share/vulkan/icd.d /etc/vulkan/icd.d 2>/dev/null || true"
run "vulkan libraries" bash -lc "ldconfig -p 2>/dev/null | grep -i vulkan || true"
run "xrt userspace" bash -lc "ldconfig -p 2>/dev/null | grep -Ei 'xrt|amdxdna' || true"
run "packages amdxdna/xrt/vulkan" bash -lc "rpm -qa 2>/dev/null | grep -Ei 'kernel-headers|amdxdna|xrt|vulkan|mesa' || dpkg -l 2>/dev/null | grep -Ei 'linux-headers|amdxdna|xrt|vulkan|mesa' || true"
run "dmesg tail amdxdna" bash -lc "dmesg 2>/dev/null | grep -Ei 'amdxdna|amdgpu|kfd|aie' | tail -n 120 || true"
