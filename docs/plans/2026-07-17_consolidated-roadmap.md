# Colibrì consolidated roadmap

Date: 2026-07-17

This document supersedes the older exploratory notes under `docs/plans/`. It is intentionally focused on the work that still matters now that the repository already contains the core runtime, Qwen3.5 quantized-weight scaffolding, planner/doctor support, and Jinja-based server prompt rendering.

## Current baseline

The repository already has meaningful implementation in place:

- `c/qwen35_moe.c` now loads quantized weights into a `QTensor` container and routes matmuls through the shared runtime path.
- `c/backend_runtime.c` and `c/backend_runtime.h` provide a role-aware multi-backend runtime with CPU fallback semantics.
- `c/resource_plan.py` and `c/doctor.py` generate and validate parallel-backend plans, including role affinity.
- `c/openai_server.py` supports Jinja chat templates, reasoning-content splitting, and tool-call rendering/parsing for the OpenAI-compatible server path.
- The primary validation command remains `cd c && make check`.

The remaining work is therefore less about re-creating the architecture and more about hardening, finishing the missing fidelity gaps, and making the runtime ready for production use.

## Priority workstreams

### 1. Runtime / backend parity

Goal: make the backend runtime the stable integration layer for CPU, Vulkan, ROCm and NPU execution without regressing the CPU reference path.

Key tasks:
- Keep the quantized `QTensor` path as the canonical execution path for Qwen3.5/Ornith weights.
- Extend coverage for runtime dispatch, role-based partitioning, backend masking, and parallel scheduler behavior.
- Add real-device smoke checks for Vulkan/ROCm/NPU shims where available, while preserving CPU fallback semantics.
- Add or refine profiling hooks so backend transfers and kernel time can be compared against the CPU reference path.

Acceptance criteria:
- `cd c && make check` stays green.
- Parallel runtime tests cover the same code paths that the planner and doctor rely on.
- Backend selection remains deterministic and documented via environment variables and planner output.

### 2. Model fidelity for the text path

Goal: close the remaining gaps that still prevent the text-only engine from being considered complete for Qwen3.5/Ornith-style workloads.

Key tasks:
- Finish the linear-attention / SSM path so it is no longer a stub in the Qwen3.5/Ornith stack.
- Integrate the remaining decoding features that are still missing or only partially exposed: grammar-constrained decoding, logprobs, embeddings, and tokenizer edge cases.
- Preserve exactness and sampling behavior for the reference model family rather than introducing a separate compatibility mode.

Acceptance criteria:
- Golden tests exist for the missing text-path features.
- The server and CLI surfaces expose the same behavior for the same prompt and sampling settings.
- The implementation remains grounded in the existing quantized-storage and backend-runtime design.

### 3. Server / agent surface

Goal: make the OpenAI-compatible server the default front-end for agent-style workloads and keep its prompt/rendering behavior consistent with the model family.

Key tasks:
- Keep the Jinja-template flow as an opt-in but well-tested path when the model ships a `chat_template.jinja`.
- Harden tool-call parsing/streaming, reasoning-content handling, and structured-output behavior.
- Keep the server’s defaults aligned with the model’s expectations and document when the built-in renderer should be preferred over the Jinja path.
- Treat the colibri engine itself as the primary runtime for Ornith MoE; do not treat `llama-server` as the main execution path for MoE models.

Acceptance criteria:
- The existing OpenAI-server tests stay green.
- Tool-calling and reasoning-content behavior has coverage for both non-streaming and streaming requests.
- Prompt rendering remains stable for both GLM-style and Qwen-style chat templates.

### 4. Multimodal support

Goal: add the first useful multimodal slice without trying to solve all modalities at once.

Recommended scope:
- Start with image support (vision tokens + preprocessing + server request plumbing).
- Revisit video and audio only after the image path is proven and the corresponding model assets are available.

Key tasks:
- Add an image preprocessing pipeline and a concrete vision-token insertion path.
- Wire the server to accept image inputs and forward them to the engine.
- Keep the implementation isolated from the core text path so it can be developed as a separate workstream if needed.

Acceptance criteria:
- One end-to-end image prompt can be accepted by the server and routed to the model.
- The implementation remains optional and does not break text-only usage.

### 5. Packaging, CI and release readiness

Goal: ensure the repository is easy to build, test and run across the supported environments.

Key tasks:
- Keep the container images and backend-specific builds reproducible on Ubuntu/Fedora-based environments.
- Extend CI so the runtime, planner/doctor and server tests are run automatically for the main build matrix.
- Maintain one canonical quickstart and one canonical implementation note; archive old exploratory notes instead of leaving several competing plans in circulation.

Acceptance criteria:
- The main CI workflow covers the core runtime and server test suites.
- The docs remain aligned with the current implementation and do not describe outdated or contradictory paths.

## Recommended execution order

1. Stabilize the runtime and keep the existing test suite green.
2. Close the remaining text-path fidelity gaps in the engine.
3. Harden the server/agent surface around tool calling, reasoning and structured output.
4. Add the first multimodal slice (image support).
5. Finish packaging and release-hardening work once the core runtime is stable.

## What is explicitly out of scope for the next milestone

- Full audio support for the Ornith-style multimodal stack.
- Native Vulkan/ROCm/XRT kernels as the first milestone; CPU fallback remains acceptable until hardware kernels are available.
- Treating `llama-server` as the main execution path for MoE models.

## Deliverables for the next milestone

- One active roadmap document (`docs/plans/2026-07-17_consolidated-roadmap.md`).
- One clearly archived set of superseded planning notes under `docs/plans/archive/`.
- A documented, testable path for runtime parity, server fidelity and the first multimodal slice.
