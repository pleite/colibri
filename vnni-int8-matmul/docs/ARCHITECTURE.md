# System Architecture — Strix Halo Multi-Backend int8 Matmul

## Hardware Overview

### CPU: AMD Zen 5 (Strix Halo)
- **Cores**: 32 (16P + 16E)
- **Vector Units**: AVX-512 + VNNI (VPMADDUBSD)
- **Cache**: 32MB L3, 1MB L2 per core complex
- **Memory**: 65GB DDR5 unified
- **Target**: 300-350 GFLOPS int8

### iGPU: AMD RDNA 4 (gfx1151)
- **Compute Units**: 40 CUs
- **Clock**: Up to 3.2 GHz
- **Memory**: 65GB unified (shared with CPU)
- **Interface**: Vulkan compute shaders
- **Target**: 500-600 GFLOPS int8

### NPU: AMD XDNA 2
- **AI Engines**: 4 AIE-2 blocks
- **Performance**: ~30 TOPS INT8
- **Constraint**: Fixed-shape kernels only
- **Interface**: XRT (Xilinx Runtime)
- **Target**: 1000+ TOPS INT8

## Software Architecture

### Layer 1: Kernel Implementations
```
kernel/
├── vnni_matmul_test.c      # AVX-512 VNNI (working)
├── avx512_fma.c            # AVX-512 FMA (future)
├── scalar.c                # Portable scalar (baseline)
```

### Layer 2: Backend Wrappers
```
cpu/
├── backend_cpu.c           # CPU backend wrapper
├── backend_cpu.h           # CPU backend API

gpu/
├── backend_vulkan.c        # Vulkan backend wrapper
├── backend_vulkan.h        # Vulkan backend API

npu/
├── backend_xrt.c           # XRT backend wrapper
├── backend_xrt.h           # XRT backend API
```

### Layer 3: Orchestration
```
orchestrator/
├── tensor_loader.c         # Unified memory tensor loading
├── backend_selector.c      # Backend selection logic
├── scheduler.c             # Execution scheduling
```

### Layer 4: Integration
```
integration/
├── colibri_plugin.c        # Colibri engine plugin
├── benchmark.c             # Performance benchmarks
└── test_suite.c            # Validation tests
```

## Data Flow

```
int8 weights (disk)
    ↓
dequantize to int8 (per-row scales)
    ↓
load to unified memory (65GB)
    ↓
dispatch to backend (CPU/GPU/NPU)
    ↓
execute matmul
    ↓
accumulate results (float32)
    ↓
apply scales and bias
    ↓
output (float32)
```

## Backend Selection Logic

### Current (Colibri)
```c
// From backend_runtime.c
if (role == COLI_ROLE_ATTENTION || role == COLI_ROLE_SHARED_EXPERT) {
    // Route to NPU preferred (6x weight)
} else if (role == COLI_ROLE_ROUTER_EXPERT || role == COLI_ROLE_DENSE) {
    // Route to GPU preferred (6x weight)
} else {
    // Small ops → CPU only
}
```

### Proposed (Multi-Backend)
```c
// Backend selection based on:
// 1. Matrix dimensions (I, O)
// 2. Available memory
// 3. Backend utilization
// 4. Latency requirements

if (I * O > THRESHOLD_GPU && gpu_utilization < 0.8) {
    return BACKEND_GPU;
} else if (I * O > THRESHOLD_NPU && npu_utilization < 0.8) {
    return BACKEND_NPU;
} else {
    return BACKEND_CPU;
}
```

## Memory Architecture

### Unified Memory Model
- **Total**: 65GB DDR5
- **CPU accessible**: Yes (direct)
- **GPU accessible**: Yes (via Vulkan)
- **NPU accessible**: Yes (via XRT)

### Tensor Placement Strategy
1. **Hot tensors** (frequently accessed): Keep in unified memory
2. **Cold tensors** (rarely accessed): Evict to disk
3. **Working set**: Fit within L3 cache (32MB) when possible

### Eviction Policy
```c
// LRU (Least Recently Used) with priority
// Priority based on:
// - Access frequency
// - Matrix dimensions
// - Backend utilization
```

## Performance Optimization

### CPU Optimization
1. **Loop unrolling**: 4x-8x unroll for better ILP
2. **Cache tiling**: Match L1/L2 cache sizes
3. **Prefetching**: Hardware prefetch hints
4. **Vectorization**: AVX-512 FMA for floating point

### GPU Optimization
1. **Work-group sizing**: Match CU architecture
2. **Memory coalescing**: Align accesses
3. **Register pressure**: Minimize spills
4. **Compute overlap**: Hide memory latency

### NPU Optimization
1. **Fixed shapes**: Pre-compile kernels
2. **Data flow**: Pipeline between AIE blocks
3. **Memory bandwidth**: Maximize utilization
4. **Synchronization**: Minimize barriers

## Testing Strategy

### Unit Tests
- Scalar reference vs optimized implementation
- Edge cases (zero, negative, overflow)
- Memory alignment requirements

### Integration Tests
- Full model inference (Ornith 397B)
- Backend switching
- Memory eviction/loading

### Performance Tests
- GFLOPS measurement
- Latency profiling
- Memory bandwidth utilization

## Future Enhancements

1. **Dynamic dispatch**: Runtime backend selection
2. **Adaptive tiling**: Auto-tune tile sizes
3. **Mixed precision**: int4/int8/float16
4. **Batching**: Process multiple matrices

---

**Status**: Architecture design phase  
**Next**: Implement CPU optimization and GPU skeleton
