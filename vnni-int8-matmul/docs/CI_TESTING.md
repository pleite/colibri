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
      
      - name: Setup Vulkan environment
        run: |
          echo "VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json" >> $GITHUB_ENV
          sudo ln -sf /usr/bin/ld.bfd /etc/alternatives/ld
          sudo ln -sf /usr/bin/ld.bfd /usr/bin/ld
      
      - name: Run VNNI tests
        working-directory: vnni-int8-matmul
        run: |
          make clean
          make
          make test
      
      - name: Upload test results
        if: always()
        uses: actions/upload-artifact@v4
        with:
          name: vnni-test-results
          path: |
            vnni-int8-matmul/tests/test_backends
            vnni-int8-matmul/tests/vulkan_runtime_test
            vnni-int8-matmul/*.log
```

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

```
Run make test
  ./tests/test_backends
  CPU backend OK (avx512-vnni)
  Vulkan backend OK (vulkan-compute)
  XDNA2 backend OK (xdna2-fixed-4x1)
  Edge-case tests OK
  All backend tests passed.
  ./tests/vulkan_runtime_test
  Vulkan runtime test passed via vulkan-compute
```

## Step 4: Containerized Testing (Optional)

For isolated testing, use Podman containers:

```yaml
- name: Test in Podman container
  run: |
    podman run --rm \
      --device /dev/dri \
      --device /dev/kfd \
      --security-opt label=disable \
      --cap-add=SYS_PTRACE \
      -v ${{ github.workspace }}/vnni-int8-matmul:/opt/vnni:rw \
      -e VK_ICD_FILENAMES=/usr/share/vulkan/icd.d/radeon_icd.x86_64.json \
      docker.io/kyuz0/amd-strix-halo-toolboxes:vulkan-radv \
      bash -c 'cd /opt/vnni && ln -sf /usr/bin/ld.bfd /etc/alternatives/ld && ln -sf /usr/bin/ld.bfd /usr/bin/ld && make clean && make && make test'
```

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
