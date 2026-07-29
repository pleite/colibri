# GPU Batched Mode - Quick Testing Guide

## Prerequisites
- Strix Halo hardware (AMD Ryzen AI Max+ 395)
- Vulkan SDK installed
- RADV drivers working (GFX1151)
- Code pulled from main branch

## Quick Start

### 1. Build the Code
```bash
cd /home/leite/colibri/vnni-int8-matmul
make clean && make
```

### 2. Test GPU Serial Mode (Baseline)
```bash
./benchmark_all_backends --backend gpu --batching-mode serial
```

### 3. Test GPU Batched Mode
```bash
./benchmark_all_backends --backend gpu --batching-mode batched --batch 16
```

### 4. Test Concurrent Mode (All Backends)
```bash
./benchmark_all_backends --concurrent
```

## Expected Results

### Serial vs Batched Performance
- **Serial**: Baseline (1x)
- **Batched (batch=16)**: Should be 4-18x faster
  - Small shapes: ~4-8x efficiency
  - Large shapes: ~9-18x efficiency

### CSV Output Format
```
backend,rows,inner_dim,out_cols,batch_size,threads,iters,elapsed_ms,success,bytes_processed,estimated_gib_s,mode,active_backends
gpu,256,4096,1024,16,1,5,12.345,1,5368709120,1.234,batched,single
```

## Troubleshooting

### Build Errors
- **Missing Vulkan headers**: Install Vulkan SDK
- **Missing libvulkan**: Install vulkan-loader package
- **Compilation errors**: Check forward declarations are in place

### Runtime Errors
- **VK_ERROR_OUT_OF_POOL_MEMORY**: Command pool capacity bug (should be fixed)
- **No Strix Halo device**: Check GPU is detected by Vulkan
- **Shader not found**: Ensure gpu/comp.spv exists

### Performance Issues
- **No speedup in batched mode**: Check batch_size is > 1
- **Crashes with large batch**: Try smaller batch_size
- **Memory errors**: Check system has enough RAM

## Test Commands Reference

### Single Shape Test
```bash
# Test specific shape with batch=16
./benchmark_all_backends --backend gpu --batching-mode batched --batch 16 --rows 256 --inner 4096 --out 1024
```

### All Shapes Test
```bash
# Test all shapes with batched mode
./benchmark_all_backends --backend gpu --batching-mode batched --batch 16
```

### Concurrent Test
```bash
# Test all backends concurrently
./benchmark_all_backends --concurrent --batch 16
```

### CSV Output
```bash
# Save results to CSV
./benchmark_all_backends --backend gpu --batching-mode batched --batch 16 --csv results.csv
```

## Validation Checklist

### Build
- [ ] Code compiles without errors
- [ ] All backend objects built
- [ ] Benchmark executable created

### Functional
- [ ] GPU serial mode works
- [ ] GPU batched mode works (batch=1, 2, 4, 8, 16)
- [ ] Output is correct (no crashes, no garbage data)
- [ ] CSV output generated correctly

### Performance
- [ ] Batched mode is faster than serial
- [ ] Efficiency is 4-18x (depending on shape)
- [ ] Larger shapes show better efficiency
- [ ] Results are consistent across runs

### Edge Cases
- [ ] batch_size=1 works (like serial)
- [ ] batch_size=32 works (maximum)
- [ ] Small shapes work
- [ ] Large shapes work
- [ ] Concurrent mode works

## Performance Analysis

### Expected Speedup Formula
```
Efficiency = (Serial_Time / Batched_Time) / batch_size
```

### Example
- Serial time: 100ms
- Batched time (batch=16): 12ms
- Efficiency: (100 / 12) / 16 = 0.52 (52% efficient)

### Good Efficiency Ranges
- **Excellent**: > 0.7 (70%)
- **Good**: 0.5 - 0.7 (50-70%)
- **Acceptable**: 0.3 - 0.5 (30-50%)
- **Poor**: < 0.3 (30%)

## Common Shapes to Test

From strix_halo_profile.csv:
```
256, 4096, 1024   # FFN up-proj
256, 1024, 4096   # FFN down-proj
256, 7168, 7168   # Attention q/k/v/o proj
256, 7168, 512    # Attention k/v proj
256, 512, 7168    # Attention q/o proj
```

## Reporting Issues

If you find issues, please report:
1. **Error messages**: Full error output
2. **Commands run**: Exact command line
3. **Expected vs actual**: What you expected vs what happened
4. **System info**: Strix Halo specs, driver version
5. **Reproducibility**: Does it happen every time?

## Success Criteria

✅ **Fixed Successfully** if:
- Code compiles and runs without errors
- Batched mode shows 4-18x efficiency
- No crashes or memory leaks
- Output is correct
- Concurrent mode works

❌ **Needs More Work** if:
- Compilation errors
- Runtime crashes
- No performance improvement
- Incorrect output
- Memory issues

---

**Quick Test Command**:
```bash
./benchmark_all_backends --backend gpu --batching-mode batched --batch 16
```

**Expected**: Should show ~4-18x efficiency compared to serial mode.

**If it works**: You're done! The fix is successful.
**If it fails**: Check troubleshooting section above.

---

**Testing Guide Version**: 1.0  
**Last Updated**: 2026-07-28  
**For**: Strix Halo GPU Batched Mode Testing
