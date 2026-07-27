# NPU Test Matrix — Copilot Action Plan

## Issue
**GitHub Issue**: https://github.com/pleite/colibri/issues/82  
**Status**: 🔴 Blocked — Missing `DRM_IOCTL_AMDXDNA_CONFIG_HWCTX` kernel ioctl

## Objective
Enable NPU execution by resolving the kernel UAPI limitation and validating all matmul shapes across different core configurations and parallelism levels.

---

## Phase 1: Kernel Investigation & Resolution

### 1.1 Investigate Kernel Requirements

**Tasks for Copilot:**

1. **Check current kernel version**
   ```bash
   uname -r
   # Expected: 7.0.12-101.fc43.x86_64 or similar
   ```

2. **Check kernel headers**
   ```bash
   ls -la /usr/include/drm/amdxdna_accel.h
   # Check if header exists and what version
   
   grep -r "DRM_IOCTL_AMDXDNA_CONFIG_HWCTX" /usr/include/
   # Search for the ioctl definition
   ```

3. **Check kernel configuration**
   ```bash
   zcat /proc/config.gz | grep DRM_AMDXDNA
   # Check if XDNA driver is enabled
   ```

4. **Search kernel source**
   - Look for `DRM_IOCTL_AMDXDNA_CONFIG_HWCTX` in kernel source
   - Check if it's in mainline or AMD-specific patches
   - Identify what kernel version introduced it

5. **Check for out-of-tree patches**
   - Search AMD GitHub for XDNA 2 patches
   - Check if there's a backport available
   - Look for kernel update scripts or repositories

### 1.2 Resolve Kernel Issue

**Options (in priority order):**

1. **Kernel Update**
   - Find kernel version that includes the ioctl
   - Update kernel or install newer kernel package
   - Reboot if necessary

2. **Kernel Patch**
   - If ioctl exists in newer kernel but not current
   - Backport the patch to current kernel
   - Rebuild kernel module

3. **Alternative Approach**
   - Check if there's a different ioctl that works
   - Look for userspace workarounds
   - Consider patching the runtime to handle missing ioctl gracefully

### 1.3 Validate Kernel Fix

**Test:**
```bash
# After kernel update/patch
cd /home/leite/colibri/vnni-int8-matmul
bash scripts/run_npu_benchmark_in_container.sh
```

**Expected:**
- No more "kernel UAPI lacks DRM_IOCTL_AMDXDNA_CONFIG_HWCTX" errors
- Tests should execute and report actual timings
- Accuracy should be PASS for most shapes

---

## Phase 2: Comprehensive Test Execution

### 2.1 Test Matrix Overview

**Shapes:** 10 total
- 5 whole-array (256-row tiles, 32 cores)
- 5 single-core (32-row tiles, 1 core each)

**Execution Modes:** 4 total
1. Sequential (1 shape at a time)
2. Parallel (10 shapes back-to-back)
3. Mixed (2 shapes at a time)
4. All-types (all 10 shapes simultaneously)

**Total Tests:** 40

### 2.2 Detailed Test Matrix

#### Shape Definitions

**Whole-Array Shapes (256-row tiles):**
```
1. 256×4096×1024  — expert gate_proj/up_proj
2. 256×1024×4096  — expert down_proj
3. 256×4096×16384 — self_attn q_proj
4. 256×4096×512   — self_attn k_proj/v_proj
5. 256×8192×4096  — self_attn o_proj
```

**Single-Core Shapes (32-row tiles):**
```
6.  32×4096×1024  — expert gate_proj/up_proj
7.  32×1024×4096  — expert down_proj
8.  32×4096×16384 — self_attn q_proj
9.  32×4096×512   — self_attn k_proj/v_proj
10. 32×8192×4096  — self_attn o_proj
```

#### Execution Mode Details

**Mode 1: Sequential**
- Run each shape individually
- Measure baseline performance
- Verify accuracy for each shape
- **Tests:** 10

**Mode 2: Parallel (All Shapes)**
- Run all 10 shapes back-to-back
- Measure total throughput
- Calculate average per-shape time
- Test sustained NPU utilization
- **Tests:** 10 (measured as one batch)

**Mode 3: Mixed (Pairs)**
- Run shapes in pairs (2 at a time)
- Test 5 pairs: (1,2), (3,4), (5,6), (7,8), (9,10)
- Measure pair execution time
- Test mixed core utilization (whole + single)
- **Tests:** 10 (5 pairs × 2 shapes)

**Mode 4: All-Types (Maximum Parallelism)**
- Run all 10 shapes simultaneously
- Test maximum NPU utilization
- Measure contention effects
- Test whole-array + single-core mixing
- **Tests:** 10 (all shapes in one batch)

### 2.3 Test Execution Script

**Script:** `scripts/run_npu_benchmark_in_container.sh`

**Usage:**
```bash
cd /home/leite/colibri/vnni-int8-matmul
bash scripts/run_npu_benchmark_in_container.sh
```

**Output:**
- `npu_benchmark_results.txt` — Full benchmark output
- `npu_benchmark_results.csv.txt` — CSV data (if extraction works)

### 2.4 Expected Metrics

For each test, capture:
- **Shape**: rows×inner_dim×out_cols
- **Core Type**: whole or single
- **Execution Mode**: sequential, parallel, mixed, all_types
- **Parallel Count**: number of shapes running simultaneously
- **Elapsed Time**: milliseconds
- **Accuracy**: PASS or FAIL
- **Error Code**: if failed

---

## Phase 3: Performance Analysis

### 3.1 Performance Metrics

**Calculate:**
1. **Per-shape timing** (sequential mode)
   - Average time per shape
   - Min/max/standard deviation

2. **Throughput** (parallel mode)
   - Total time for all 10 shapes
   - Average time per shape
   - Speedup vs sequential

3. **Scaling** (mixed mode)
   - Time for pairs vs single
   - Scaling efficiency

4. **Contention** (all-types mode)
   - Time for all 10 shapes
   - Compare to parallel mode
   - Identify contention effects

### 3.2 Core Utilization Analysis

**Questions to Answer:**
1. How many cores are actually used per shape?
2. Do whole-array shapes use all 32 cores?
3. Do single-core shapes use only 1 core?
4. What happens when mixing whole-array + single-core?
5. Is there contention when running all shapes simultaneously?

**Verification:**
```bash
# Check NPU utilization during benchmark
watch -n 1 cat /sys/devices/system/accel/accel0/stats
```

### 3.3 Accuracy Validation

**For each shape:**
1. Run NPU execution
2. Compare against CPU reference
3. Calculate error margin
4. Report PASS/FAIL

**Expected:**
- All shapes should pass accuracy check
- Error margin should be < 1.0 float
- Larger shapes may have slightly higher error

---

## Phase 4: Documentation & Reporting

### 4.1 Test Results Documentation

**Create:**
1. **Test Results CSV**
   - One row per test
   - All metrics from Phase 2.4

2. **Performance Summary**
   - Tables showing timings by shape
   - Tables showing timings by execution mode
   - Charts if possible

3. **Core Utilization Report**
   - Which cores are used for which shapes
   - Contention analysis
   - Recommendations

### 4.2 Recommendations

**Based on results, recommend:**
1. Optimal execution mode for different workloads
2. Shape-specific optimizations
3. Core allocation strategies
4. Parallelism limits

---

## Phase 5: Validation & Regression

### 5.1 Regression Testing

**After kernel fix:**
1. Re-run full test matrix
2. Compare against baseline (Phase 2)
3. Verify all tests pass
4. Measure performance improvement

### 5.2 Continuous Integration

**Recommend:**
1. Add benchmark to CI pipeline
2. Run on every kernel update
3. Track performance over time
4. Alert on regressions

---

## Deliverables

### For Copilot

1. **Kernel Investigation Report**
   - Current kernel version and capabilities
   - What's missing and why
   - Recommended fix

2. **Test Execution Results**
   - Complete test matrix results
   - Performance metrics
   - Accuracy validation

3. **Performance Analysis**
   - Core utilization analysis
   - Scaling efficiency
   - Contention effects

4. **Recommendations**
   - Optimal execution strategies
   - Shape-specific optimizations
   - Future improvements

### For Repository

1. **Updated benchmark script** (if improvements needed)
2. **Test results** (committed to repo)
3. **Documentation** (README updates)
4. **CI integration** (if applicable)

---

## Timeline

**Phase 1: Kernel Investigation** — 1-2 hours
- Check kernel version and headers
- Identify missing ioctl
- Determine fix approach

**Phase 2: Test Execution** — 30 minutes
- Run full test matrix
- Collect all metrics
- Validate accuracy

**Phase 3: Performance Analysis** — 1 hour
- Analyze results
- Calculate metrics
- Identify patterns

**Phase 4: Documentation** — 30 minutes
- Create reports
- Write recommendations
- Update documentation

**Phase 5: Validation** — 30 minutes
- Re-run tests after fix
- Verify improvements
- Commit results

**Total Estimated Time:** 4-5 hours

---

## Success Criteria

✅ Kernel ioctl issue resolved  
✅ All 40 tests execute successfully  
✅ Accuracy: 100% PASS  
✅ Performance metrics captured  
✅ Core utilization documented  
✅ Recommendations provided  
✅ Results committed to repository  

---

## Notes

- **NPU Device**: `/dev/accel/accel0`
- **AIE Array**: 8 columns × 6 rows = 48 tiles
- **Whole-Array Tiles**: 16 tiles (32 cores)
- **Single-Core Tiles**: 1 tile (1 core)
- **Kernel Module**: `amdxdna`
- **Container**: `localhost/colibri-strix-halo-test:latest`

---

## Contact

**Issue**: https://github.com/pleite/colibri/issues/82  
**Benchmark Script**: `scripts/run_npu_benchmark_in_container.sh`  
**Results**: `npu_benchmark_results.txt`
