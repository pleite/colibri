# Plan: CI Framework Migration — Dockerfile.colibri from Fedora to Ubuntu

**Date**: 2026-07-17
**Author**: Murderbot (via Hermes)
**Status**: Draft — awaiting human review and merge

---

## Problem Statement

The Dockerfile.colibri uses **Fedora** (dnf, copr enable) as its base image (registry.fedoraproject.org/fedora:43), but the GitHub Actions workflow (.github/workflows/container-images.yml) runs on **ubuntu-latest** runners. This is a fundamental mismatch:

1. dnf is not available on Ubuntu — the RUN step fails immediately
2. copr enable (used for xanderlent/amd-npu-driver XRT packages) is Fedora-specific and has no Ubuntu equivalent
3. The ROCm repo URL uses rhel10/7.2.4/main — RHEL-style, not usable on Debian/Ubuntu

**Result**: All 4 CI builds (vulkan, rocm, npu, all) have been failing since the Dockerfile was introduced.

---

## Current State

### What works
- c/Makefile — supports VULKAN=1, ROCM=1, NPU=1, and combined VULKAN=1 ROCM=1 NPU=1
- c/backend_*.c/hip — all 4 backend source files present and compile-ready
- .github/workflows/container-images.yml — matrix builds all 4 backends, pushes to GHCR

### What does not work
- Dockerfile.colibri — uses FROM registry.fedoraproject.org/fedora:43 + dnf + copr
- CI on ubuntu-latest — no dnf, no copr, different package names

---

## Solution: Convert Dockerfile to Ubuntu/Debian

### 1. Base image

FROM ubuntu:24.04 AS build

### 2. Install build dependencies (apt)

Replace the dnf install block with apt-get:

    RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential gcc g++ make git python3 python3-pip \
        ca-certificates curl gnupg \
        && rm -rf /var/lib/apt/lists/*

### 3. Install ROCm (Ubuntu)

AMD provides an official Ubuntu repository. Replace the dnf ROCm repo block:

    RUN case "$BACKEND" in \
        rocm|all) \
            curl -fsSL https://repo.radeon.com/rocm/rocm.gpg.key | gpg --dearmor -o /usr/share/keyrings/rocm.gpg && \
            echo "deb [arch=amd64 signed-by=/usr/share/keyrings/rocm.gpg] https://repo.radeon.com/rocm/apt/7.2.4/ ubuntu main" \
                > /etc/apt/sources.list.d/rocm.list && \
            apt-get update && apt-get install -y --no-install-recommends \
                rocm-llvm rocm-device-libs hip-runtime-amd hip-devel rocblas rocblas-devel \
                hipblas hipblas-devel rocm-cmake libomp-dev \
            && rm -rf /var/lib/apt/lists/* ;; \
        esac

### 4. Install Vulkan (Ubuntu)

    RUN case "$BACKEND" in \
        vulkan|all) \
            apt-get update && apt-get install -y --no-install-recommends \
                libvulkan-dev vulkan-tools glslang-tools spirv-tools \
                mesa-vulkan-drivers libvulkan1 \
            && rm -rf /var/lib/apt/lists/* ;; \
        esac

Note: On Ubuntu, the package names differ slightly:
- vulkan-devel -> libvulkan-dev
- vulkan-tools -> vulkan-tools (same)
- glslc -> glslang-tools
- spirv-tools -> spirv-tools (same)
- mesa-vulkan-drivers -> mesa-vulkan-drivers (same)
- vulkan-loader -> libvulkan1

### 5. Install XRT for NPU (Ubuntu)

The xanderlent/amd-npu-driver COPR repo is Fedora-only. For Ubuntu, XRT is available from AMD's official repository:

    RUN case "$BACKEND" in \
        npu|all) \
            apt-get update && apt-get install -y --no-install-recommends \
                xrt libxrt-dev \
            && rm -rf /var/lib/apt/lists/* ;; \
        esac

If xrt is not in the default Ubuntu repos, fall back to AMD XRT from https://github.com/Xilinx/XRT.

### 6. ROCm environment variables

Keep the existing ENV block — ROCm paths are the same on Ubuntu.

### 7. Build step (unchanged)

The make step remains identical — it already handles all backend combinations.

---

## Complete New Dockerfile.colibri

    # syntax=docker/dockerfile:1.7
    FROM ubuntu:24.04 AS build

    ARG BACKEND=vulkan

    ENV ROCM_PATH=/opt/rocm \
        HIP_PATH=/opt/rocm \
        HIP_CLANG_PATH=/opt/rocm/llvm/bin \
        HIP_DEVICE_LIB_PATH=/opt/rocm/amdgcn/bitcode \
        PATH=/opt/rocm/bin:/opt/rocm/llvm/bin:$PATH

    RUN apt-get update && apt-get install -y --no-install-recommends \
        build-essential gcc g++ make git python3 python3-pip \
        ca-certificates curl gnupg \
        && rm -rf /var/lib/apt/lists/*

    RUN case "$BACKEND" in \
        vulkan|all) \
            apt-get update && apt-get install -y --no-install-recommends \
                libvulkan-dev vulkan-tools glslang-tools spirv-tools \
                mesa-vulkan-drivers libvulkan1 \
            && rm -rf /var/lib/apt/lists/* ;; \
        rocm|all) \
            curl -fsSL https://repo.radeon.com/rocm/rocm.gpg.key | gpg --dearmor -o /usr/share/keyrings/rocm.gpg && \
            echo "deb [arch=amd64 signed-by=/usr/share/keyrings/rocm.gpg] https://repo.radeon.com/rocm/apt/7.2.4/ ubuntu main" \
                > /etc/apt/sources.list.d/rocm.list && \
            apt-get update && apt-get install -y --no-install-recommends \
                rocm-llvm rocm-device-libs hip-runtime-amd hip-devel rocblas rocblas-devel \
                hipblas hipblas-devel rocm-cmake libomp-dev \
            && rm -rf /var/lib/apt/lists/* ;; \
        npu|all) \
            apt-get update && apt-get install -y --no-install-recommends \
                xrt libxrt-dev \
            && rm -rf /var/lib/apt/lists/* ;; \
        *) echo "unsupported backend: $BACKEND" >&2; exit 1 ;; \
        esac

    WORKDIR /opt/colibri/c
    COPY c /opt/colibri/c

    RUN make clean && case "$BACKEND" in \
        vulkan) make VULKAN=1 glm qwen35_moe ;; \
        rocm) make ROCM=1 glm qwen35_moe ;; \
        npu) make NPU=1 glm qwen35_moe ;; \
        all) make VULKAN=1 ROCM=1 NPU=1 glm qwen35_moe ;; \
        esac

    EXPOSE 8000
    CMD ["python3", "/opt/colibri/c/openai_server.py", "--host", "0.0.0.0", "--port", "8000"]

---

## Package Name Mapping (Fedora to Ubuntu)

| Fedora Package | Ubuntu Package | Notes |
|---|---|---|
| gcc | gcc (via build-essential) | Same |
| gcc-c++ | g++ (via build-essential) | Same |
| make | make (via build-essential) | Same |
| git | git | Same |
| python3 | python3 | Same |
| python3-pip | python3-pip | Same |
| vulkan-devel | libvulkan-dev | Different name |
| vulkan-tools | vulkan-tools | Same |
| glslc | glslang-tools | Different name |
| spirv-tools | spirv-tools | Same |
| mesa-vulkan-drivers | mesa-vulkan-drivers | Same |
| vulkan-loader | libvulkan1 | Different name |
| vulkan-headers | libvulkan-dev (includes headers) | Merged |
| rocm-llvm | rocm-llvm | Same (from AMD repo) |
| hip-runtime-amd | hip-runtime-amd | Same |
| hip-devel | hip-devel | Same |
| xrt | xrt | Available in Ubuntu 24.04 |
| xrt-devel | libxrt-dev | Different name |
| dnf-plugins-core | N/A | Not needed on Ubuntu |
| libomp-devel | libomp-dev | Different name |
| libomp | (included in libomp-dev) | Merged |

---

## Testing Plan

1. Build locally first on Strix Halo using podman:
   cd /home/leite/colibri
   podman build -f Dockerfile.colibri --build-arg BACKEND=vulkan -t colibri:vulkan .
   podman build -f Dockerfile.colibri --build-arg BACKEND=rocm -t colibri:rocm .
   podman build -f Dockerfile.colibri --build-arg BACKEND=npu -t colibri:npu .
   podman build -f Dockerfile.colibri --build-arg BACKEND=all -t colibri:all .

2. Verify each image contains the expected binary and linked libraries:
   podman run --rm --entrypoint="" colibri:rocm ldd /opt/colibri/c/qwen35_moe | grep -E "libamdhip|libhsa"
   podman run --rm --entrypoint="" colibri:vulkan ldd /opt/colibri/c/qwen35_moe | grep -E "libvulkan"
   podman run --rm --entrypoint="" colibri:npu ldd /opt/colibri/c/qwen35_moe | grep -E "libxrt"

3. Push to GitHub and let CI validate:
   git add Dockerfile.colibri
   git commit -m "fix: convert Dockerfile.colibri from Fedora to Ubuntu for CI compatibility"
   git push origin main

4. Monitor CI — all 4 matrix builds should pass on ubuntu-latest.

---

## Risks and Mitigations

| Risk | Mitigation |
|---|---|
| xrt not available in Ubuntu 24.04 default repos | Fall back to AMD XRT prebuilt packages from GitHub releases |
| ROCm 7.2.4 Ubuntu repo path may differ | Use AMD documented Ubuntu repo path: https://repo.radeon.com/rocm/apt/7.2.4/ |
| glslc path may differ on Ubuntu | The Makefile searches glslc in PATH — both glslang-tools and spirv-tools provide it |
| libomp version mismatch | Ubuntu libomp-dev is LLVM-based, compatible with GCC |

---

## References

- AMD ROCm Ubuntu installation guide: https://rocmdocs.amd.com/en/latest/Installation_Guide/Installation-Guide.html
- AMD XRT GitHub: https://github.com/Xilinx/XRT
- Vulkan SDK for Linux: https://vulkan.lunarg.com/sdk/home#linux
- Toolbox repo (reference for ROCm build): github.com/kyuz0/amd-strix-halo-toolboxes

---

## Summary

The Dockerfile needs to be converted from Fedora (dnf/copr) to Ubuntu (apt) to match the ubuntu-latest GitHub Actions runners. The Makefile and CI workflow are already correct — only the Dockerfile base image and package installation commands need to change. This is a straightforward package-name mapping with proper ROCm and XRT repository configuration for Ubuntu.
