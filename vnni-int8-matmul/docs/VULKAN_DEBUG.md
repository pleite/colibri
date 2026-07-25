# Vulkan Backend Debug Guide

## Scope

The Vulkan backend in `vnni-int8-matmul/gpu/vulkan_backend.c` is tested only on Strix Halo. The harness in `vnni-int8-matmul/scripts/strix-halo-podman-test.sh` uses the same headless container pattern as the Strix Halo toolbox and llama-server workflows and does not attempt portable fallbacks.

## Prerequisites

The Strix Halo test harness requires:

- access to `/dev/dri` and `/dev/kfd` from Podman
- the `docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv` container image
- the AMD RADV ICD inside the container at `/usr/share/vulkan/icd.d/radeon_icd.x86_64.json`

The harness does not require a host-side X server. If a display is available, it will pass that through; otherwise it runs in the same headless container context used by the toolbox and llama-server workflows.

## What the harness does

The Podman harness:

1. verifies that Podman is available and usable
2. requires a graphical session before it starts
3. mounts the checked-out `vnni-int8-matmul` tree into the container
4. runs the build and test suite inside the Strix Halo toolbox container
5. writes a `test_output.log` file next to the source tree for CI artifacts

## Run it

```bash
cd /home/runner/work/colibri/colibri/vnni-int8-matmul
make podman-test
```

Or directly:

```bash
./scripts/strix-halo-podman-test.sh
```

## Debugging steps

If the harness fails, inspect the generated `test_output.log` first. The script also leaves the container output in the log so you can see whether the failure happened during the build, the debug harness, or the runtime tests.

Useful checks from the host:

```bash
export VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json
vulkaninfo --summary | grep -A5 "GPU0"
```

Useful checks inside the container:

```bash
podman run --rm --device /dev/dri --device /dev/kfd \
  --security-opt label=disable --cap-add=SYS_PTRACE \
  --env VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json \
  docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv \
  bash -lc 'ls -la /dev/dri && vulkaninfo --summary | head'
```

## Notes for the backend

The backend still uses explicit Vulkan loader checks and the debug harness remains available for targeted failures. The important change here is that testing is now anchored to a Strix Halo Podman workflow instead of optional host-side fallback behavior.
