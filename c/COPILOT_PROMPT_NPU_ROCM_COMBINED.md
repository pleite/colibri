COPILOT_PROMPT_START
# Plan: Enable Combined ROCm + NPU Backend for Strix Halo

## Problem
The colibri engine supports ROCm (HIP), NPU (CPU reference shim), Vulkan, and CUDA backends — but the Makefile treats them as mutually exclusive. Strix Halo has BOTH an iGPU (Radeon 8060S, needs ROCm) and an NPU (XDNA 2 / amdxdna). We need to build with BOTH backends so the engine can dispatch matmuls to either device.

## Current State

### backend_rocm.hip (388 lines)
- Real HIP kernels for matmul on AMD GPUs
- Unified memory support for APUs (Strix Halo)
- Exports: coli_cuda_init, coli_cuda_matmul, etc. (via ABI compat)
- Uses: hipMalloc, hipMemcpy, hipLaunchKernelGGL

### backend_npu.c (314 lines)
- CPU reference shim — runs matmuls on host cores
- No real NPU driver interaction (amdxdna not implemented)
- Exports the same coli_cuda_* ABI
- Falls back to scalar C matmul with OpenMP

### backend_cuda.cu (230 lines)
- NVIDIA CUDA backend (not relevant for Strix Halo)

### qwen35_moe.c
- Uses COLI_HAS_BACKEND macro to detect any backend
- Calls coli_cuda_init() / coli_cuda_matmul() for acceleration
- Falls back to CPU matmul if backend unavailable
- Only initializes ONE device (device 0)

### Makefile
- ROCM=1 sets BACKEND_OBJ=backend_rocm.o, adds -DCOLI_CUDA -DCOLI_ROCM
- NPU=1 sets BACKEND_OBJ=backend_npu.o, adds -DCOLI_NPU
- They are mutually exclusive (error if both set)
- qwen35_moe$(EXE) links with $(BACKEND_OBJ)

## Solution

### 1. Modify Makefile: Allow combined ROCm + NPU build

Change the backend selection logic so ROCm and NPU can be built together:

```makefile
# Backend selection: ROCm + NPU can coexist on Strix Halo
# ROCm provides GPU acceleration, NPU provides CPU reference path
BACKEND_OBJ =
BACKEND_CFLAGS =

ifeq ($(ROCM),1)
BACKEND_CFLAGS += -DCOLI_CUDA -DCOLI_ROCM
BACKEND_OBJ += backend_rocm.o
endif

ifeq ($(NPU),1)
BACKEND_CFLAGS += -DCOLI_NPU
BACKEND_OBJ += backend_npu.o
endif
```

Update the qwen35_moe build rule:
```makefile
qwen35_moe$(EXE): qwen35_moe.c st.h json.h compat.h $(BACKEND_OBJ)
	$(CC) $(CFLAGS) $(BACKEND_CFLAGS) qwen35_moe.c $(BACKEND_OBJ) -o qwen35_moe$(EXE) $(LDFLAGS)
```

Remove the mutual exclusion errors.

### 2. Modify qwen35_moe.c: Dual-backend initialization

The engine currently calls `coli_cuda_init(devices, 1)` once. Change it to:

1. Try ROCm first (device 0 = iGPU)
2. If ROCm fails or isn't built, fall back to NPU (device 0 = CPU)
3. Store which backend is active so matmul knows which to call

Add a runtime flag: `static int g_backend_type = 0;` (0=none, 1=roc m, 2=npu)

In the matmul hot path, check `g_backend_type` and call the appropriate init/matmul function.

### 3. Update Dockerfile.colibri

Already done in branch dockerfile/npu-rocm-build. The Dockerfile builds with ROCM=1. No changes needed — the Makefile change above will handle the NPU side if NPU=1 is set.

### 4. Add environment variable support

Add COLI_BACKEND env var to let users choose at runtime:
- COLI_BACKEND=roc m → use ROCm only
- COLI_BACKEND=npu → use NPU only  
- COLI_BACKEND=auto (default) → try ROCm, fall back to NPU

## Files to Modify

1. **c/Makefile** — Remove mutual exclusion, allow combined BACKEND_OBJ
2. **c/qwen35_moe.c** — Dual-backend init logic, g_backend_type flag
3. **c/backend_npu.c** — No changes needed (already has correct ABI)
4. **c/backend_rocm.hip** — No changes needed

## Testing

After changes:
1. Build with ROCM=1 only → should work (existing behavior)
2. Build with NPU=1 only → should work (existing behavior)
3. Build with ROCM=1 NPU=1 → should compile and link both backends
4. Run with ROCm available → should use GPU acceleration
5. Run without ROCm (no /dev/dri) → should fall back to NPU/CPU

## Constraints
- Single file changes only where needed
- Preserve existing CPU fallback behavior
- Don't break existing ROCm-only or NPU-only builds
- Keep the ABI compatibility layer (coli_cuda_* functions)
- OpenMP for parallel matmul in NPU path

## Priority
1. Makefile: allow combined build
2. qwen35_moe.c: dual-backend init
3. Verify compilation with all 3 modes (ROCm only, NPU only, both)
4. Update Dockerfile if needed (already done)
COPILOT_PROMPT_END
