# Architecture notes

## CPU backend

The CPU backend implements a row-major int8 matmul where each output element is the dot product of an input row and a weight row. It is written specifically for Strix Halo and uses AVX-512 VNNI (`_mm512_dpbusd_epi32`) when the host exposes the instruction set. The implementation does not fall back to a generic scalar path because the target is a fixed Strix Halo deployment.

## GPU backend

The Vulkan backend is intentionally a thin wrapper around the CPU implementation. It probes for a Vulkan loader and, when present, exposes a Vulkan-facing entry point. The current build does not attempt to generate or execute a full GPU shader pipeline; it uses the CPU path as the functional implementation to keep the prototype small and deterministic.

## NPU backend

The XDNA2 backend uses a fixed-shape contract: one input row and four output columns (`rows=1`, `out_cols=4`). That keeps the wrapper minimal and matches the intended XDNA2 use case for a narrow, high-throughput kernel.
