# CI Testing Guide — GitHub Actions with Local Strix Halo Runner

## Overview

This guide documents how to set up a self-hosted GitHub Actions runner on Strix Halo to test Vulkan GPU backends in CI.

## Prerequisites

- Strix Halo machine with AMD Radeon 8060S iGPU
- GitHub repository with Actions enabled
- Self-hosted runner registered with the repository
- Podman or Docker for containerized testing

## Step 1: Set Up Self-Hosted Runner

### 1.1 Create Runner on GitHub

1. Go to your repository → Settings → Actions → Runners
2. Click "New self-hosted runner"
3. Select "Linux" → "x64"
4. Follow the instructions to create a runner token

### 1.2 Install Runner on Strix Halo

```bash
# Create runner directory
mkdir -p /home/leite/actions-runner && cd /home/leite/actions-runner

# Download runner package
curl -o actions-runner-linux-x64-2.316.1.tar.gz https://github.com/actions/runner/releases/download/v2.316.1/actions-runner-linux-x64-2.316.1.tar.gz
tar xzf actions-runner-linux-x64-2.316.1.tar.gz

# Configure runner
./config.sh \
  --url https://github.com/pleite/colibri \
  --token <YOUR_RUNNER_TOKEN> \
  --labels strix-halo,vulkan,gpu \
  --unattended

# Start runner as service
./svc.sh install
sudo systemctl start actions-runner
```

### 1.3 Verify Runner

```bash
# Check runner status
sudo systemctl status actions-runner

# Test runner connectivity
cd /home/leite/actions-runner
./run.sh --once
```

## Step 2: Create GitHub Actions Workflow

Create `.github/workflows/vnni-test.yml`:

```yaml
name: VNNI GPU Tests

on:
  push:
    branches: [main]
  pull_request:
    branches: [main]

jobs:
  test-vulkan:
    runs-on: [self-hosted, strix-halo, vulkan]

    steps:
      - name: Checkout code
        uses: actions/checkout@v4

      - name: Run VNNI tests in the Strix Halo podman harness
        working-directory: vnni-int8-matmul
        shell: bash
        run: |
          set -euo pipefail
          make podman-test

      - name: Upload test results
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: vnni-test-results
          path: |
            vnni-int8-matmul/tests/test_backends
            vnni-int8-matmul/tests/vulkan_runtime_test
            vnni-int8-matmul/test_output.log
```

## Forcing a run, and what to check when nothing runs

The workflow is dispatchable:

* Actions → **VNNI GPU Tests** → **Run workflow**, or
* `gh workflow run vnni-test.yml --ref <branch>`

Inputs:

| Input | Default | Effect |
|---|---|---|
| `base_image` | `ghcr.io/pleite/colibri-vulkan:latest` | base the harness image is layered on |
| `require_npu` | `true` | fail when `/dev/accel/accel0` is absent |
| `skip_npu` | `false` | do not pass the NPU device through at all |

Use `require_npu=false` to exercise the CPU and Vulkan paths while the NPU is
unavailable; leave it `true` for normal runs so a missing accel node is a red
run rather than a silent SKIP.

### Runs stuck in "queued"

A queued run that never starts is **always** a runner problem, never a workflow
problem: GitHub has no runner online matching `[self-hosted, strix-halo,
vulkan]`. Note that `container-images.yml` targets `[self-hosted, linux, x64]`,
so if both workflows queue, the runner is offline rather than mislabelled.

On the Strix Halo host:

```bash
sudo systemctl status actions-runner      # or: ./svc.sh status
sudo systemctl start actions-runner
sudo journalctl -u actions-runner -n 100 --no-pager
```

Then confirm on GitHub under Settings → Actions → Runners that the runner is
**Idle** and carries all three labels. Cancel the backlog before re-running:

```bash
gh run list --workflow vnni-test.yml --status queued \
  --json databaseId --jq '.[].databaseId' | xargs -rn1 gh run cancel
```

The workflow sets `concurrency: cancel-in-progress`, so future downtime
supersedes stale runs instead of accumulating them.

## Step 3: Test Workflow

### 3.1 Push Test Commit

```bash
cd /home/leite/colibri
echo "# Test" >> README.md
git add README.md
git commit -m "test: verify CI workflow"
git push origin main
```

### 3.2 Monitor Actions

1. Go to repository → Actions tab
2. Watch the workflow run
3. Check logs for test output

### 3.3 Expected Output

On a Strix Halo runner with the iGPU available and no `.npukernel` artifact
loaded:

```
Run make podman-test
  --- toolchain ---
  gcc (GCC) 15.x
  GNU ld version 2.4x
  --- kernel uapi ---
  amdxdna UAPI present
  --- test ---
  CPU backend OK (avx512-vnni)
  Vulkan backend OK (vulkan-compute-strix-halo)
  XDNA2 backend SKIP (no NPU kernel loaded for this shape)
  Edge-case tests OK
```

`SKIP` is the correct result for the NPU until an AIE-compiled `.npukernel`
artifact is available; it must never become `OK`, because the only way a
shapeless NPU path could produce numbers is a CPU fallback, which this tree
forbids. Treat any NPU `OK` without a loaded artifact as a regression.

## Step 4: The container the harness uses

`make podman-test` builds `vnni-int8-matmul/Dockerfile.strix-halo-test` and runs
the suite inside it. The image is layered on the community Strix Halo toolbox
image (`docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv`), which provides a
working RADV/Mesa stack for GFX1151 but is an inference runtime, not a build
environment. On its own it is missing:

| Missing | Symptom | Package |
|---|---|---|
| `ld` | `collect2: fatal error: cannot find 'ld'` | `binutils` |
| `<drm/amdxdna_accel.h>` | NPU sources do not compile | `kernel-headers` (≥ 6.14) |
| `<drm/drm.h>` | pulled in by the amdxdna header | `libdrm-devel` |
| `<vulkan/vulkan.h>` | Vulkan backend does not compile | `vulkan-headers`, `vulkan-loader-devel` |

Do **not** patch this up inside the test step with
`ln -sf /usr/bin/ld.bfd /usr/bin/ld`. That was the shortcut an earlier run took;
it makes the environment undocumented and unreproducible, and it does nothing
about the missing headers. Change the Dockerfile instead — it asserts that `ld`
and both headers exist, so a broken base image fails at image-build time rather
than halfway through a test.

Override the base image with `BASE_IMAGE` if the toolbox tag moves:

```yaml
- name: Run the harness
  working-directory: vnni-int8-matmul
  env:
    BASE_IMAGE: docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv
    REQUIRE_NPU: "1"
  run: make podman-test
```

### NPU access from the container

The NPU is the `amdxdna` accel node, not `/dev/dri`. The harness passes
`--device /dev/accel/accel0` when it exists. Requirements on the runner:

```bash
lsmod | grep amdxdna              # module loaded (in-tree since Linux 6.14)
ls -l /dev/accel/accel0           # node present
ls /lib/firmware/amdnpu/17f0_11/  # Strix Halo NPU firmware installed
```

Set `REQUIRE_NPU=1` in CI so a missing node fails the job instead of silently
turning the NPU tests into SKIPs. XRT is deliberately not installed in the
image: `c/npu_kernels` issues DRM ioctls directly, and having XRT present would
make it ambiguous which path a run exercised.

## Step 5: Add Test Reporting

### 5.1 Create Test Summary

Add to workflow after `make test`:

```yaml
- name: Create test summary
  if: always()
  run: |
    echo "## VNNI Test Results" >> $GITHUB_STEP_SUMMARY
    echo "" >> $GITHUB_STEP_SUMMARY
    echo "### CPU Backend" >> $GITHUB_STEP_SUMMARY
    if grep -q "CPU backend OK" vnni-int8-matmul/test_output.log; then
      echo "✅ PASSED" >> $GITHUB_STEP_SUMMARY
    else
      echo "❌ FAILED" >> $GITHUB_STEP_SUMMARY
    fi
    echo "" >> $GITHUB_STEP_SUMMARY
    echo "### Vulkan Backend" >> $GITHUB_STEP_SUMMARY
    if grep -q "Vulkan backend OK" vnni-int8-matmul/test_output.log; then
      echo "✅ PASSED" >> $GITHUB_STEP_SUMMARY
    else
      echo "❌ FAILED" >> $GITHUB_STEP_SUMMARY
    fi
```

### 5.2 Capture Test Output

Modify the test step to capture output:

```yaml
- name: Run VNNI tests
  working-directory: vnni-int8-matmul
  run: |
    make clean
    make
    make test 2>&1 | tee test_output.log
```

## Troubleshooting

### `cannot find 'ld'` / missing headers

The job is running the bare toolbox image instead of the harness image. Run
`make podman-test`, which builds `Dockerfile.strix-halo-test`, rather than
invoking `podman run` against the base image directly.

### NPU tests skip on a machine that has an NPU

```bash
lsmod | grep amdxdna
ls -l /dev/accel/accel0
sudo dmesg | grep -i amdxdna
```

`CREATE_HWCTX` failing with `ENOENT` is a userspace bug (no device heap
allocated before the context), not a missing kernel feature; an unimplemented
ioctl reports `ENOTTY`. See `vnni-int8-matmul/docs/TESTING.md` for the full
errno table.

### Runner Not Appearing

```bash
# Check runner logs
sudo journalctl -u actions-runner -f

# Verify configuration
cat /home/leite/actions-runner/.runner
```

### Vulkan Tests Failing

1. Check `VK_ICD_FILENAMES` is set
2. Verify GPU is accessible: `ls -la /dev/dri/`
3. Test Vulkan: `vulkaninfo --summary`

### Container Issues

```bash
# Check container logs
podman logs <container-id>

# Verify device access
podman run --rm --device /dev/dri alpine ls -la /dev/dri/
```

## Cost Considerations

- Self-hosted runners use your own hardware (no GitHub minutes cost)
- Strix Halo has 128 GB RAM — ensure sufficient resources
- GPU compute is fast but may impact other workloads during CI

## Maintenance

- Keep runner updated: `./config.sh --unattended` after new releases
- Monitor disk space: `du -sh /home/leite/actions-runner`
- Check runner health: `sudo systemctl status actions-runner`

## Example Workflow Run

See: https://github.com/pleite/colibri/actions/runs/<RUN_ID>
