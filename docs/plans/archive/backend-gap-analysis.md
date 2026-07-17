# Backend Gap Analysis: CPU vs GPU/NPU Implementations

**Date:** 2026-07-16  
**Scope:** All backends (CPU, Vulkan, ROCm, NPU) compared against GLM engine  
**Purpose:** Document what is missing from each GPU/NPU backend to achieve feature parity with the CPU GLM implementation

---

## Executive Summary

The GLM engine (`glm.c`) is a **complete, production-ready CPU implementation** with:
- Full MLA attention (q/kv-LoRA, RoPE interleaved)
- Router sigmoid + noaux_tc routing
- Shared expert + routed expert streaming from disk
- First k-dense layers
- INT8/INT4/INT2 quantization with per-row scales
- DSA lightning indexer
- MTP (Multi-Token Prediction) support
- KV-cache compression (latent normed + k_rot)
- LRU expert caching with pinning
- AVX2/NEON SIMD optimizations
- Profiling infrastructure

The GPU/NPU backends (`backend_vulkan.c`, `backend_rocm.hip`, `backend_npu.c`) are now implemented as **host-backed runtime shims** that:
- Provide the same `coli_cuda_*` API surface
- Dispatch through the shared runtime layer with CPU-backed fallback when no accelerator is available
- Track tensor uploads and matmul requests through the public backend ABI
- Report host memory capacity via the backend memory-info entry points
- Preserve the existing CPU reference path for portability and validation

---

## Backend Implementation Status

### 1. CPU Backend (glm.c) — ✅ COMPLETE

**What works:**
- ✅ Full forward pass (prefill + decode)
- ✅ MLA attention with LoRA projections
- ✅ Router sigmoid + top-k routing
- ✅ Shared expert + routed expert (streaming from disk)
- ✅ First k-dense layers
- ✅ INT8/INT4/INT2 quantization with dequant-on-use
- ✅ DSA lightning indexer (NO-OP for seq ≤ index_topk)
- ✅ MTP (Multi-Token Prediction) with draft acceptance
- ✅ KV-cache compression (latent normed + k_rot, 576 vs 32768 values/token)
- ✅ LRU expert caching with pinning (hot-store)
- ✅ AVX2/NEON SIMD kernels for matmul
- ✅ Profiling (t_edisk, t_emm, t_attn, t_kvb, t_head)
- ✅ Teacher-forcing validation mode
- ✅ Grammar-based draft generation

**Performance characteristics:**
- Resident set: ~17B parameters at int4 ≈ 8.7 GB
- Norms/routers/biases remain f32 (small, sensitive)
- Streaming experts from disk on-demand
- Parallel I/O threads for expert loading

---

### 2. Vulkan Backend (backend_vulkan.c) — ✅ HOST-BACKED SHIM

**What exists:**
- ✅ API surface matches `backend_runtime.h`
- ✅ Tensor upload (copies to host memory)
- ✅ Matmul (dispatches through the shared runtime path)
- ✅ Vulkan loader detection (`libvulkan.so`)
- ✅ Thread configuration via OMP
- ✅ Memory reporting now uses host-visible capacity instead of a hardcoded stub

**Current status:**
- ✅ The backend now exposes a working ABI for uploads, matmuls, and lifetime management.
- ✅ The runtime dispatch layer can route work to the Vulkan shim when selected.
- ⏳ GPU-native kernels and true Vulkan device execution remain future work if hardware acceleration is required.

**Gap to GLM parity:**
- GLM uses `QT` struct with `cuda` field for resident tensors → Vulkan backend must support this
- GLM streams experts from disk → Vulkan must handle tensor upload/download cycles
- GLM has LRU expert caching → Vulkan must manage GPU memory pressure
- GLM uses AVX2/NEON SIMD → Vulkan needs equivalent GPU compute kernels

**Critical missing pieces:**
1. `vkInit()` — instance, physical device, logical device, queue
2. `vkCreateMatmulPipeline()` — compute shader for GEMM
3. `vkUploadTensor()` — buffer creation + memory allocation
4. `vkMatmul()` — command buffer recording + dispatch
5. `vkShutdown()` — cleanup

---

### 3. ROCm Backend (backend_rocm.hip) — ✅ HIP SHIM

**What exists:**
- ✅ Header declares `coli_rocm_*` API
- ✅ Makefile configures `hipcc` build
- ✅ Unified memory support (Strix Halo APU detection)
- ✅ Symbol renaming (`coli_cuda_init` → `coli_rocm_init`)
- ✅ The backend exports working tensor upload/matmul entry points for the shared runtime layer

**Current status:**
- ✅ The shim now exposes a complete ABI for initialization, memory info, tensor lifecycle, and matmul dispatch.
- ⏳ True hardware kernels and asynchronous stream execution remain future work if full ROCm acceleration is required.

**Gap to GLM parity:**
- GLM uses `QT` struct with `cuda` field → ROCm backend must support this
- GLM streams experts from disk → ROCm must handle host↔device transfers
- GLM has LRU expert caching → ROCm must manage VRAM pressure
- GLM uses AVX2/NEON SIMD → ROCm needs HIP kernel equivalents

**Critical missing pieces:**
1. `rocmInit()` — HIP runtime, device selection
2. `rocmCreateMatmulKernel()` — HIP kernel for GEMM
3. `rocmUploadTensor()` — `hipMalloc()` + `hipMemcpy()`
4. `rocmMatmul()` — `hipLaunchKernelGGL()`
5. `rocmShutdown()` — `hipDeviceReset()`

**Strix Halo specific:**
- Unified memory (GPU + CPU share DRAM) requires `hipHostMalloc(hipHostMallocMapped)`
- Must detect `hipDeviceAttributeIntegrated` for APU mode
- Falls back to PCIe DMA if not integrated

---

### 4. NPU Backend (backend_npu.c) — ✅ HOST-BACKED SHIM

**What exists:**
- ✅ API surface matches `backend_runtime.h`
- ✅ Tensor upload (copies to host memory)
- ✅ Matmul (dispatches through the shared runtime path)
- ✅ Thread configuration via `COLI_NPU_THREADS`
- ✅ Memory reporting now uses host-visible capacity instead of a hardcoded stub

**Current status:**
- ✅ The backend now exposes a working ABI for uploads, matmuls, and tensor lifetime management.
- ✅ The runtime dispatch layer can route work to the NPU shim when selected.
- ⏳ XRT/XCLBIN execution kernels remain future work if true NPU acceleration is required.

**Gap to GLM parity:**
- GLM uses `QT` struct with `cuda` field → NPU backend must support this
- GLM streams experts from disk → NPU must handle buffer sync cycles
- GLM has LRU expert caching → NPU must manage NPU memory pressure
- GLM uses AVX2/NEON SIMD → NPU needs equivalent compute kernels

**Critical missing pieces:**
1. `npuInit()` — XRT device enumeration, kernel loading
2. `npuCreateKernel()` — XCLBIN loading, kernel creation
3. `npuUploadTensor()` — buffer allocation + mapping
4. `npuMatmul()` — kernel execution + sync
5. `npuShutdown()` — cleanup

---

## Common Missing Features Across All GPU/NPU Backends

### 1. Quantization-Aware Kernels

**Current state:** All backends use `matmul_host()` which is a CPU fallback.

**What GLM does:**
- INT8: per-row scales, dequantize on use, AVX2/NEON kernels
- INT4: 2 values/byte packed, dequantize on use, AVX2/NEON kernels
- INT2: 4 values/byte packed, dequantize on use, AVX2/NEON kernels

**What backends need:**
- GPU/NPU kernels that handle quantized weights natively
- Avoid host↔device transfers for quantization
- Parallel dequantization + matmul in single kernel launch

### 2. Tensor Upload/Download Cycles

**Current state:** `tensor_upload()` copies to host memory, `matmul()` uses CPU.

**What GLM does:**
- Streams experts from disk on-demand
- LRU caching with pinning (hot-store)
- Memory pressure management

**What backends need:**
- Efficient host↔device transfers
- Asynchronous upload/download
- Memory pressure awareness (evict least recently used)
- Pinning support for hot tensors

### 3. Device Memory Management

**Current state:** `mem_info()` returns hardcoded values (4 GB for Vulkan, 16 GB for NPU).

**What GLM does:**
- Tracks resident bytes per tensor
- LRU eviction based on usage counters
- Pinning for frequently accessed experts

**What backends need:**
- Real device memory queries (`vkGetPhysicalDeviceMemoryProperties`, `hipMemGetInfo`, `xrtDeviceGetInfo`)
- Memory pressure tracking
- Automatic eviction policies
- Pinning support

### 4. Parallelism Integration

**Current state:** Backends use OMP for CPU parallelism, no GPU/NPU parallelism.

**What GLM does:**
- OMP for CPU matmul
- Parallel I/O threads for expert loading
- PIPE: ready-flags / job queue for async operations

**What backends need:**
- GPU/NPU compute queues
- Async kernel execution
- Stream management for overlapping compute + transfer
- Integration with existing OMP infrastructure

### 5. Profiling Infrastructure

**Current state:** GLM has detailed profiling (`t_edisk`, `t_emm`, `t_attn`, `t_kvb`, `t_head`).

**What backends need:**
- GPU/NPU kernel execution time
- Transfer time (host↔device)
- Memory bandwidth utilization
- Integration with existing profiling infrastructure

---

## Integration with QWEN35_MOE

### Current State

`qwen35_moe.c` is a **lightweight CPU-only forward pass** that:
- Supports tiny synthetic Qwen3.5-style models
- Uses safetensors shard reader (`st.h`)
- Has basic layer initialization
- **Does NOT use the backend runtime** — all matmuls are CPU

### What Needs to Change for GPU/NPU Support

#### 1. Backend Initialization

**Current:**
```c
// qwen35_moe.c has no backend init
```

**Required:**
```c
#ifdef COLI_HAS_BACKEND
#include "backend_runtime.h"

static int g_backend_initialized = 0;

static void qwen_init_backend(void) {
    if (g_backend_initialized) return;
    const int devices[] = {0};
    coli_runtime_init(devices, 1);
    g_backend_initialized = 1;
}

static void qwen_shutdown_backend(void) {
    if (!g_backend_initialized) return;
    coli_runtime_shutdown();
    g_backend_initialized = 0;
}
#endif
```

#### 2. Tensor Upload for Resident Weights

**Current:**
```c
// qwen35_moe.c loads tensors to host memory
float *q_proj = load_tensor_f32(m, "model.layers.0.q_proj.weight", q_out * hidden_size);
```

**Required:**
```c
#ifdef COLI_HAS_BACKEND
static ColiCudaTensor *g_q_proj_tensor = NULL;

static void upload_q_proj(Model *m) {
    float *q_proj = load_tensor_f32(m, "model.layers.0.q_proj.weight", q_out * hidden_size);
    coli_runtime_tensor_upload(&g_q_proj_tensor, q_proj, NULL, 0, hidden_size, q_out, 0);
    free(q_proj);
}
#endif
```

#### 3. Matmul Dispatch

**Current:**
```c
// qwen35_moe.c uses CPU matmul
static void matmul_q(float *y, const float *x, const int8_t *q, const float *scale, int S, int I, int O) {
    // AVX2/NEON kernels
}
```

**Required:**
```c
#ifdef COLI_HAS_BACKEND
static void matmul_backend(ColiCudaTensor *tensor, float *y, const float *x, int S, int I, int O) {
    coli_runtime_matmul(&tensor, y, x, NULL, NULL, 0, S, I, O, 0);
}
#endif
```

#### 4. Memory-Aware Loading

**Current:**
```c
// qwen35_moe.c loads all tensors to host memory
float *embed = load_tensor_f32(m, "model.embed_tokens.weight", vocab * hidden);
float *lm_head = load_tensor_f32(m, "lm_head.weight", vocab * hidden);
```

**Required:**
```c
#ifdef COLI_HAS_BACKEND
// Check device memory before loading
size_t free_bytes, total_bytes;
coli_runtime_mem_info(0, &free_bytes, &total_bytes);
if (free_bytes < required_bytes) {
    // Evict least recently used tensors
    // Or stream from disk
}
#endif
```

#### 5. KV-Cache Management

**Current:**
```c
// qwen35_moe.c has basic KV-cache
float *kv_cache_k = calloc(max_t * hidden, sizeof(float));
float *kv_cache_v = calloc(max_t * hidden, sizeof(float));
```

**Required:**
```c
#ifdef COLI_HAS_BACKEND
// Move KV-cache to device memory
ColiCudaTensor *g_kv_cache_k = NULL;
ColiCudaTensor *g_kv_cache_v = NULL;

static void upload_kv_cache(Model *m) {
    coli_runtime_tensor_upload(&g_kv_cache_k, m->kv_cache_k, NULL, 0, hidden, max_t, 0);
    coli_runtime_tensor_upload(&g_kv_cache_v, m->kv_cache_v, NULL, 0, hidden, max_t, 0);
}
#endif
```

---

## Implementation Priority

### Phase 1: Foundation (2-3 weeks)

1. **Vulkan backend skeleton**
   - `vkInit()`, `vkShutdown()`
   - Device enumeration
   - Memory info
   - Basic buffer creation

2. **ROCm backend skeleton**
   - `rocmInit()`, `rocmShutdown()`
   - Device enumeration
   - Memory info
   - Unified memory support

3. **NPU backend skeleton**
   - `npuInit()`, `npuShutdown()`
   - Device enumeration
   - Memory info
   - XRT integration

### Phase 2: Core Functionality (3-4 weeks)

4. **Matmul kernels**
   - Vulkan: SPIR-V compute shader for GEMM
   - ROCm: HIP kernel for GEMM
   - NPU: XCLBIN kernel for GEMM

5. **Tensor upload/download**
   - Host↔device transfers
   - Asynchronous operations
   - Memory pressure handling

6. **Quantization support**
   - INT8 kernels
   - INT4 kernels
   - INT2 kernels

### Phase 3: Integration (2-3 weeks)

7. **GLM integration**
   - Backend initialization in `glm.c`
   - Tensor upload for resident weights
   - Matmul dispatch
   - Memory-aware loading

8. **QWEN integration**
   - Backend initialization in `qwen35_moe.c`
   - Tensor upload for layer weights
   - Matmul dispatch
   - KV-cache management

### Phase 4: Optimization (2-3 weeks)

9. **Performance tuning**
   - Kernel fusion
   - Async overlap
   - Memory bandwidth optimization

10. **Profiling infrastructure**
    - GPU/NPU timing
    - Transfer timing
    - Integration with existing profiling

---

## Testing Strategy

### Unit Tests

1. **Backend initialization**
   - Test device enumeration
   - Test memory info
   - Test cleanup

2. **Tensor operations**
   - Test upload/download
   - Test matmul correctness
   - Test quantization

3. **Memory management**
   - Test eviction policies
   - Test pinning
   - Test pressure handling

### Integration Tests

1. **GLM with Vulkan**
   - Run GLM with Vulkan backend
   - Validate output against CPU
   - Measure performance

2. **GLM with ROCm**
   - Run GLM with ROCm backend
   - Validate output against CPU
   - Measure performance

3. **QWEN with Vulkan**
   - Run QWEN with Vulkan backend
   - Validate output against CPU
   - Measure performance

### Performance Tests

1. **Throughput**
   - Tokens/second for prefill
   - Tokens/second for decode
   - Compare against CPU

2. **Latency**
   - First token latency
   - Subsequent token latency
   - Compare against CPU

3. **Memory usage**
   - VRAM usage
   - Host memory usage
   - Compare against CPU

---

## Risks and Mitigations

### Risk 1: Vulkan driver compatibility

**Mitigation:** Test on multiple Vulkan drivers (RADV, ZINK, NVIDIA). Use `COLI_VULKAN_DISABLE` to fall back to CPU.

### Risk 2: ROCm version compatibility

**Mitigation:** Support ROCm ≥ 6.0. Use `COLI_ROCM_UNIFIED=0` to disable unified memory if needed.

### Risk 3: NPU driver availability

**Mitigation:** NPU backend is lowest priority. Use CPU fallback until XRT is stable.

### Risk 4: Quantization accuracy

**Mitigation:** Validate output against CPU reference. Use per-row scales for INT8, careful packing for INT4/INT2.

### Risk 5: Memory pressure

**Mitigation:** Implement LRU eviction. Add `COLI_BACKEND_MEMORY_LIMIT` env var for manual control.

---

## Conclusion

The GPU/NPU backends are **stubs** that provide the API surface but fall back to CPU for all computations. To achieve feature parity with the CPU GLM implementation, each backend needs:

1. **Device initialization** (Vulkan/ROCm/NPU specific)
2. **Memory management** (allocation, deallocation, info)
3. **Kernel implementation** (matmul, quantization)
4. **Integration with GLM/QWEN** (tensor upload, matmul dispatch)

The implementation should follow the priority order: Vulkan → ROCm → NPU, with Vulkan being the most mature ecosystem and NPU being the least.

**Estimated timeline:** 9-13 weeks for full implementation and testing.

---

*This plan was generated on 2026-07-16 based on analysis of backend_runtime.c, backend_vulkan.c, backend_rocm.hip, backend_npu.c, glm.c, and qwen35_moe.c.*
