# Backend and engine parity gaps — colibrì

Date: 2026-07-16

This document consolidates the missing work needed to bring the current engine and accelerator stack up to the level of the CPU/GLM baseline, and to make Qwen3.5 MoE and the other backends reach the same level of support.

## Summary

The current repository already has:

- a CPU fallback path,
- a GLM-style engine path,
- a Qwen3.5 MoE scaffold,
- backend entry points for CUDA, ROCm/HIP, Vulkan, and NPU.

What is still missing is the work that turns these into a fully validated, feature-complete, backend-agnostic stack.

## 1. Shared parity blockers for all backends

These are the gaps that must be closed before every backend is truly "up to par" with the CPU/GLM baseline.

1. Finish the backend contract end to end
   - Ensure each backend implements the same runtime contract (`init`, `malloc`, `free`, `copy`, `matmul`, `cleanup`) and provides fallback semantics when a kernel or shape is unsupported.
   - Keep the engine code backend-agnostic; the same model path should run on CPU, CUDA, ROCm, Vulkan, and NPU without special-case logic.

2. Implement the resident low-bit tensor path
   - Add the FP8/FP6/FP4 resident-format pipeline end to end, including converter metadata, layout rules, runtime capability checks, and backend dispatch points.
   - This is the main missing piece for making the GPU/NPU backends comparable to the existing int4/int8 stack used by GLM.

3. Add backend-specific correctness and regression tests
   - Every backend needs correctness tests for the same matmul/weight paths that CPU and GLM already exercise.
   - Tests should cover CPU fallback, backend success, and graceful fallback on unsupported shapes or missing drivers.

4. Wire planner/doctor/server reporting into each backend
   - The planner, doctor flow, and runtime diagnostics should report backend availability, capability limits, and fallback reasons in a consistent way.
   - This is required for operational parity and for turning backend selection into a supported feature rather than a manual experiment.

## 2. Missing steps for Qwen3.5 MoE

The Qwen3.5 MoE path is currently a correctness scaffold rather than a production-quality implementation. The missing work is:

1. Replace the skeleton forward pass with a faithful Qwen3.5 implementation
   - Implement the actual attention path, including RoPE, GQA, and KV-cache behavior.
   - Make the linear-attention path a real SSM-style block rather than a lightweight placeholder.

2. Finish the core model features that the engine is expected to support
   - Full attention math and causal masking
   - Correct linear-attention behavior
   - KV-cache persistence and reuse
   - Grammar-constrained decoding
   - Logits/logprobs output
   - Embeddings and token-level generation semantics

3. Add validation against a reference implementation
   - Teacher-forcing tests and generation tests should be added against a transformers oracle rather than only synthetic fixtures.

4. Add the multimodal and advanced generation surfaces
   - Vision, video, and audio handling remain missing.
   - The chat template should be upgraded from the current text-only subset to the full template behavior expected by the model.

## 3. Backend-by-backend gap list

### CPU backend

- Status: baseline fallback path.
- Missing steps:
  - Keep the CPU fallback path as the correctness reference for all other backends.
  - Add the same low-bit/resident-tensor and planner/doctor reporting hooks that the GPU/NPU backends need.
  - Ensure CPU remains the default fallback when a backend is unavailable or a shape is unsupported.

### GLM engine path

- Status: the existing GLM path is the reference implementation for the current engine stack.
- Missing steps:
  - Preserve backend-agnostic behavior so the same GLM execution path works across CPU, CUDA, ROCm, Vulkan, and NPU.
  - Port the GLM-specific quantized/streaming logic into the shared backend abstraction rather than keeping it tied to one backend.
  - Add backend parity tests so GLM results are validated on every backend, not only the CPU path.

### CUDA backend

- Status: reference backend for GPU execution.
- Missing steps:
  - Finish the shared low-bit resident-tensor path for CUDA as part of the common backend contract.
  - Add the same runtime capability checks, fallback reporting, and regression tests used by ROCm/Vulkan/NPU.
  - Keep CUDA as the reference path for performance and correctness while the other backends catch up.

### ROCm/HIP backend

- Status: backend abstraction and build support exist, but the full execution stack is still incomplete.
- Missing steps:
  - Complete the low-bit resident-tensor path end to end.
  - Add kernel dispatch points, converter metadata, and runtime gating for FP8/FP6/FP4 formats.
  - Validate the backend on AMD hardware and ensure correct fallback behavior on unsupported shapes or missing ROCm support.

### Vulkan backend

- Status: a shim exists, but the compute path is not finished.
- Missing steps:
  - Implement the full Vulkan compute pipeline: SPIR-V/shader path, descriptor setup, command submission, and synchronization.
  - Add correctness tests for 8/6/4-bit matmul coverage on Mesa/RADV-style devices.
  - Finish runtime selection and planner/doctor reporting so Vulkan can be used as a supported fallback.

### NPU backend

- Status: compatibility shim plus capability gating; actual offload is not complete.
- Missing steps:
  - Add the real XDNA 2 offload path for fixed-shape subgraphs.
  - Add device discovery and capability reporting for AMD XDNA 2 and clear CPU/GPU fallback paths.
  - Add observability and tests for unavailable/degraded NPU modes.

## 4. Practical order of implementation

The highest-value sequence is:

1. Finish the shared backend contract and runtime fallback semantics.
2. Implement the low-bit resident-tensor path once and wire it into all relevant backends.
3. Finish Qwen3.5 correctness and core generation features.
4. Add backend-parity tests and planner/doctor reporting.
5. Then tackle performance tuning and backend-specific optimization.
