# VNNI int8×int8 Matmul — Strix Halo Multi-Backend Project

## Overview

This is an **independent project** for implementing optimized int8 matrix multiplication on AMD Strix Halo hardware. It is separate from the main colibri repository but uses the same quantization format and hardware targets.

**Hardware**: AMD Ryzen AI Max+ 395 (Strix Halo)
- **CPU**: 32 cores Zen 5 with AVX-512 + VNNI
- **iGPU**: RDNA 4 (gfx1151, Radeon 8060S)
- **NPU**: XDNA 2 (~30 TOPS INT8)
- **Memory**: 65GB unified RAM

## Project Structure

```
vnni-int8-matmul/
├── README.md                    # This file
├── kernel/
│   └── vnni_matmul_test.c      # Working AVX-512 VNNI implementation
├── cpu/
│   └── (future: CPU-specific kernels)
├── gpu/
│   └── (future: Vulkan compute shaders)
├── npu/
│   └── (future: XRT kernels)
└── docs/
    ├── ARCHITECTURE.md          # System architecture overview
    └── DEBUG_SUMMARY.md         # VNNI debugging notes
```

## Current Status

### ✅ Working: AVX-512 VNNI CPU Kernel

**File**: `kernel/vnni_matmul_test.c`

The AVX-512 VNNI implementation **works correctly** on Strix Halo:
- All 6 test cases pass with 0% error
- Performance: ~0.000ms for 64×4 matrix (unoptimized)
- Uses `_mm512_dpbusd_epi32` with proper sign handling

**Test Results**:
```
TEST 1: Signed int8 dot-product kernel — ALL PASS
TEST 2: Signed int8 data — ALL PASS  
TEST 3: Inspect intermediate values — PASS
TEST 4: Alternative approaches — PASS (AVX-512 VNNI matches scalar)
TEST 5: Larger matrix (I=64, O=4) — ALL PASS (0.0e+00 error)
TEST 6: Performance sanity check — PASS
```

### 🚧 Future: GPU and NPU Implementations

- **GPU**: Vulkan compute shaders for RDNA 4
- **NPU**: XRT kernels for XDNA 2 (fixed-shape only)

## Compile and Run

```bash
cd vnni-int8-matmul/kernel/
gcc -O3 -mavx512f -mavx512vnni -mavx512bw -mavx512dq -o vnni_matmul_test vnni_matmul_test.c
./vnni_matmul_test
```

## Quantization Format

This project uses the same int8 quantization as the colibri repo:
- **Format**: Per-row scales (float32), int8 weights
- **Scale calculation**: `scale = max(amax/127, 1e-8)`
- **Dequantization**: `output = scale * int8_value`

## Optimization Plan

### Phase 1: CPU Optimization (Current)
1. ✅ Basic AVX-512 VNNI implementation
2. 🔄 Loop unrolling and tiling
3. 🔄 Prefetching and cache optimization
4. 🔄 Benchmark against scalar reference
5. 🔄 Integrate with colibri engine

### Phase 2: GPU Implementation
1. Design Vulkan compute shaders for RDNA 4
2. Implement int8 matmul in GLSL/SPIR-V
3. Test with unified memory architecture
4. Benchmark against CPU implementation

### Phase 3: NPU Implementation
1. Design XRT kernels for XDNA 2
2. Implement fixed-shape int8 matmul
3. Test with AIE-2 tile ISA
4. Benchmark against CPU/GPU implementations

### Phase 4: Orchestration
1. Design tensor loading/eviction strategy
2. Implement backend selection logic
3. Test with full Ornith 397B model
4. Optimize memory bandwidth utilization

## Performance Targets

| Backend | Target GFLOPS | Notes |
|---------|---------------|-------|
| CPU (AVX-512) | 300-350 | Current focus |
| GPU (Vulkan) | 500-600 | RDNA 4 theoretical |
| NPU (XRT) | 1000+ | XDNA 2 theoretical |

## Related Projects

- **colibri**: Main inference engine (`/home/leite/colibri`)
- **quantization_converter.py**: Python int8 quantization reference
- **backend_runtime.c**: Heterogeneous dispatch layer

## Next Steps for Copilot

1. **Optimize the CPU kernel**:
   - Add loop unrolling (4x-8x)
   - Implement cache tiling (L1/L2)
   - Add prefetching hints
   - Benchmark and profile

2. **Design GPU implementation**:
   - Create Vulkan compute shader skeleton
   - Implement int8 matmul in GLSL
   - Test with small matrices

3. **Design NPU implementation**:
   - Create XRT kernel skeleton
   - Implement fixed-shape matmul
   - Test with AIE-2 tile ISA

4. **Create orchestration plan**:
   - Design tensor loading/eviction strategy
   - Implement backend selection logic
   - Test with real model weights

## Documentation

- `docs/ARCHITECTURE.md` — System architecture and component overview
- `docs/DEBUG_SUMMARY.md` — VNNI debugging notes and findings
- `kernel/vnni_matmul_test.c` — Working implementation with tests

---

**Location**: `/home/leite/colibri/vnni-int8-matmul/`  
**Status**: CPU kernel working, GPU/NPU pending  
**Next**: Optimize CPU kernel and design GPU/NPU implementations
