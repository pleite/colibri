# Qwen3.5 MoE Port Plan — colibrì

Tracks implementation of a **Qwen3.5 MoE** inference engine (`c/qwen35_moe.c`)
compatible with models like `deepreinforce-ai/Ornith-1.0-397B`.

This is a **new engine**, not a modification of `glm.c`. The Qwen3.5 architecture
differs from GLM-5.2 in attention mechanism, routing, MTP placement, and weight
layout — all of which require a separate implementation.

## Current status (2026-07-15)

The repository now has a real `c/qwen35_moe.c` engine binary, safetensors
loading logic, a toy-model test harness, and converter/tooling glue. The
implementation is still a CPU-only correctness scaffold rather than a full
Qwen3.5 production engine, so the remaining work is focused on feature
completeness and validation:

- `c/qwen35_moe.c` now has a working skeleton for config parsing, safetensors
  loading, tokenizer scaffolding, and a tiny forward pass.
- The current path is not yet a faithful implementation of the full Qwen3.5
  attention/MoE/MTP architecture for real-world models.
- Quantization and optimization work (int4 weights, int8 activations, expert
  caching, GPU offload) remains pending.
- Reference-level validation against a transformers oracle is still missing.
- The feature audits in `docs/plans/2026-07-15_engine-capability-audit.md` and
  `docs/plans/2026-07-15_copilot-ornith-feature-completeness.md` call out
  additional expected capabilities beyond the initial skeleton: full attention
  math (RoPE/GQA/KV cache), faithful linear-attention behavior, grammar-
  constrained decoding, embeddings/logprobs, and multimodal token handling for
  vision/video/audio.

### Next unimplemented steps

1. Replace the scaffolding with a fuller forward-pass implementation for the
   Qwen3.5 layer types and tensor layout used by real models.
2. Add teacher-forcing and generation tests against a transformers oracle, not
   just the current synthetic fixture.
3. Implement the docs-driven feature gaps: actual attention computation,
   linear-attention fidelity, KV-cache persistence, grammar-constrained
   decoding, embeddings/logprobs, and multimodal prompt handling.
4. Implement quantization and caching optimizations, then harden the
   integration points in the CLI, doctor, planning, and serving flows.

---

## Architecture Differences: GLM-5.2 vs Qwen3.5 MoE

| feature | GLM-5.2 (existing) | Qwen3.5 MoE (target) |
|---|---|---|
| **architecture** | `glm_moe_dsa` | `qwen3_5_moe` |
| **attention** | MLA (q/kv-LoRA, interleaved partial RoPE) | Hybrid linear + full (interval 4), GQA |
| **linear attn** | N/A | Linear conv kernel dim 4, key heads 16, value heads 64 |
| **full attn** | Standard (every layer) | Every 4th layer (interval) |
| **router** | DeepSeek-style DSA sparse attention | Standard top-k routing (10 of 512 experts) |
| **experts** | 21,504 total, per-layer LRU cache | 512 total, standard MoE cache |
| **MTP head** | Layer 78 (int8 required) | Layer 0 (single MTP layer) |
| **hidden size** | varies | 4096 |
| **head dim** | varies | 256 |
| **layers** | 80 | 60 |
| **dtype** | int4 (weights), int8 (MTP) | bfloat16 (weights) |
| **tokenizer** | custom | Qwen3 tokenizer |

---

## Phase Q1 — Core Engine Skeleton

Goal: build a working forward pass that produces the same tokens as the
`transformers` reference (teacher-forcing validation).

### Q1.1 — Model Loading

```
c/qwen35_moe.c:
  - config.json parser (text_config fields)
  - safetensors shard reader (same format as glm.c)
  - weight dequantization: bf16 → f32 (no int4 container needed)
  - tokenizer loading (Qwen3 tokenizer.json)
```

**Key differences from glm.c:**
- No int4 container format — weights are bf16 safetensors
- No expert streaming — all 512 experts fit in RAM (smaller model)
- No DSA sparse attention — standard dense attention

### Q1.2 — Attention Layers

```
c/qwen35_moe.c:
  - linear_attention_layer(): linear conv + key/value projection
  - full_attention_layer(): standard GQA attention (every 4th layer)
  - hybrid_pattern(): interleaves linear and full attention per layer_type
```

**Implementation notes:**
- Linear attention: conv kernel dim 4, key heads 16, value heads 64
- Full attention: standard multi-head with GQA (32 query heads, 2 KV heads)
- Layer types from config: `["linear_attention", ..., "full_attention", ...]`

### Q1.3 — MoE Layers

```
c/qwen35_moe.c:
  - moe_layer(): standard top-k routing (k=10 of 512 experts)
  - expert_forward(): gate + up + down projections
  - router(): softmax over gate weights, top-k selection
```

**Key differences from glm.c:**
- Standard MoE (not DeepSeek-style DSA)
- 512 experts total (not 21,504)
- All experts fit in RAM (no disk streaming needed)
- No int8 quantization requirement for routing

### Q1.4 — MTP Head

```
c/qwen35_moe.c:
  - mtp_head(): single MTP layer at layer 0
  - draft_token_generation(): speculative decoding
```

**Key differences from glm.c:**
- MTP at layer 0 (not layer 78)
- Single MTP layer (not multiple)
- No int8 requirement (bf16 is fine)

### Q1.5 — Validation

```
tests/test_qwen35_moe.c:
  - teacher-forcing validation against transformers oracle
  - token-exact on tiny-random model
  - generation test (20 tokens, greedy)
```

**Validation criteria:**
- 32/32 teacher-forcing matches
- 20/20 greedy generation matches
- Token-exact on tiny-random model

---

## Phase Q2 — Optimization

Goal: match or exceed reference performance with quantization and parallelism.

### Q2.1 — Quantization

```
c/qwen35_moe.c:
  - int4 weight quantization (same as glm.c)
  - int8 activation quantization (IDOT kernels)
  - bf16 → int4 container converter (tools/convert_qwen35_to_int4.py)
```

**Quantization strategy:**
- Weights: int4 (same as glm.c, ~8× compression)
- Activations: int8 (IDOT kernels, 2-3× speedup)
- MTP head: int8 (required for speculation acceptance)
- Norms/router: f32 (high precision)

### Q2.2 — Expert Streaming & Caching

```
c/qwen35_moe.c:
  - define an expert storage abstraction (resident, cached, pinned, streaming)
  - implement per-layer expert cache slots with LRU/heat-based eviction
  - load expert weights on demand from safetensors / quantized expert containers
  - support quantized expert payloads (int4 weights + row scales, int8 fallback)
  - batch unique experts per layer and step, then run them through the cache/stream path
  - add hot-store / pinning for frequently reused experts
  - add hit/miss accounting, residency stats, and cache debug telemetry
  - add RAM/VRAM budget planning for expert residency (resource_plan.py)
```

**Implementation tasks:**
- Model the expert lifecycle explicitly: resident-in-RAM, cached-in-RAM, pinned-hot, streamed-from-storage, and uploaded-to-backend.
- Split expert access into three phases: route selection, cache lookup, and on-miss load/activate.
- Keep the dense path resident and make the expert path the only part that uses streaming/cache policy.
- Add prefetch and overlap logic so expert loads can happen while the current token/step continues computing.
- Add a correctness guard so cache hits and stream misses preserve bit-exact output compared with the dense reference path.

**Key differences from glm.c:**
- 512 experts total (not 21,504)
- The first implementation can stay RAM-backed and simpler than GLM, but it should still expose the same cache/streaming seams for later disk-backed scaling.
- Expert streaming should be implemented as a policy layer, not as a rewrite of the full model stack.

### Q2.3 — Parallelism & Multi-Backend Expert Dispatch

```
c/qwen35_moe.c:
  - OpenMP parallel matmul (same as glm.c)
  - dispatch expert matmuls through the existing backend abstraction (ROCm/HIP, Vulkan, NPU, CUDA)
  - use CPU for routing/orchestration while offloading eligible matmuls to accelerators
  - allow multiple backends/devices to be used in parallel when available
  - keep a CPU fallback path for every expert matmul
```

**Parallelism strategy:**
- OpenMP for matmul (32 cores on Strix Halo)
- Use the existing thin backend layer to route resident and cached experts to CPU, ROCm, Vulkan, NPU, or CUDA as available.
- For expert-heavy decode/prefill, fan out independent experts across available backends/devices rather than forcing a single backend.
- Use all backends at once as needed: CPU handles orchestration and small work, accelerators handle large expert matmuls, and the scheduler can overlap cache loads with backend execution.
- Preserve a graceful fallback path so any backend can be disabled without breaking correctness.

---

## Phase Q3 — Integration

Goal: integrate with existing colibri tooling (coli, doctor, plan, serve).

### Q3.1 — CLI Integration

```
c/coli:
  - add "qwen35" model type
  - --model flag for Qwen3.5 models
  - auto-detect architecture from config.json
```

### Q3.2 — Doctor Integration

```
c/doctor.py:
  - validate Qwen3.5 config
  - check expert counts and layer types
  - validate safetensors headers
  - plan RAM/disk/VRAM usage
```

### Q3.3 — Resource Planning

```
c/resource_plan.py:
  - Qwen3.5-specific layer analysis
  - expert cache sizing (512 experts)
  - RAM budget for full expert set
```

### Q3.4 — OpenAI Server

```
c/openai_server.py:
  - add "qwen35" model endpoint
  - same API as glm.c server
  - support for streaming and non-streaming
```

---

## Phase Q4 — Testing & Benchmarking

Goal: validate correctness and measure performance.

### Q4.1 — Correctness Tests

```
tests/test_qwen35_moe.c:
  - teacher-forcing: 32/32 matches
  - greedy generation: 20/20 matches
  - token-exact on tiny-random model
  - quantization accuracy (int4 vs bf16)
```

### Q4.2 — Performance Benchmarks

```
benchmarks/qwen35_bench.py:
  - tokens/second (decode)
  - tokens/second (prefill)
  - RAM usage (resident + cache)
  - disk I/O (if streaming)
  - GPU utilization (if enabled)
```

### Q4.3 — Community Testing

```
- Share benchmarks with community
- Collect hardware reports
- Document known issues
- Iterate on optimizations
```

---

## Implementation Order

1. **Q1.1** — Model loading (config, safetensors, tokenizer)
2. **Q1.2** — Attention layers (linear + full)
3. **Q1.3** — MoE layers (routing + expert forward)
4. **Q1.4** — MTP head
5. **Q1.5** — Validation (teacher-forcing, generation)
6. **Q2.1** — Quantization (int4 weights, int8 activations)
7. **Q2.2** — Expert caching (LRU, RAM budget)
8. **Q2.3** — Parallelism (OpenMP, GPU backend)
9. **Q3.1** — CLI integration (coli, doctor, plan)
10. **Q3.2** — OpenAI server
11. **Q4.1** — Correctness tests
12. **Q4.2** — Performance benchmarks

---

## Estimated Timeline

- **Q1 (Core Engine):** 2-3 weeks
- **Q2 (Optimization):** 1-2 weeks
- **Q3 (Integration):** 1 week
- **Q4 (Testing):** 1 week
- **Total:** 5-7 weeks for a single developer

---

## Key Files to Create/Modify

### New Files
- `c/qwen35_moe.c` — Main engine (~2,000 lines)
- `c/qwen35_moe.h` — Header file (~100 lines)
- `tests/test_qwen35_moe.c` — Validation tests (~300 lines)
- `tools/convert_qwen35_to_int4.py` — Weight converter (~200 lines)
- `benchmarks/qwen35_bench.py` — Performance benchmarks (~150 lines)

### Modified Files
- `c/Makefile` — Add qwen35 target
- `c/coli` — Add qwen35 model type
- `c/doctor.py` — Add qwen35 validation
- `c/resource_plan.py` — Add qwen35 analysis
- `c/openai_server.py` — Add qwen35 endpoint
- `README.md` — Document qwen35 support

---

## Dependencies

- **ROCm/HIP** (optional, for GPU acceleration)
- **Vulkan** (optional, for Vulkan backend)
- **OpenMP** (required, for CPU parallelism)
- **Python 3** (required, for tooling)
- **GCC 15+** (required, for C compilation)

---

## Success Criteria

1. **Correctness:** 32/32 teacher-forcing matches, 20/20 greedy generation matches
2. **Performance:** ≥50% of reference performance (bf16), ≥2× speedup with int4
3. **Integration:** Works with coli, doctor, plan, serve
4. **Documentation:** README, benchmarks, community testing

---

## Risks & Mitigations

### Risk: Architecture Complexity
- **Mitigation:** Start with bf16 (no quantization), add int4 later
- **Mitigation:** Use existing glm.c patterns where possible

### Risk: Attention Mechanism
- **Mitigation:** Implement linear attention first, then full attention
- **Mitigation:** Validate each attention type independently

### Risk: Expert Routing
- **Mitigation:** Use standard top-k routing (simpler than DSA)
- **Mitigation:** All 512 experts fit in RAM (no streaming needed)

### Risk: MTP Head
- **Mitigation:** Single MTP layer (simpler than GLM-5.2)
- **Mitigation:** No int8 requirement (bf16 is fine)

---

## Community Resources

- **Ornith-1.0-397B:** https://huggingface.co/deepreinforce-ai/Ornith-1.0-397B
- **Qwen3.5 Architecture:** https://github.com/QwenLM/Qwen3.5
- **Colibri Discord:** Community testing and feedback
- **GitHub Issues:** Track progress and report bugs

---

## Next Steps

1. **Implement Q1.1** — Model loading (config, safetensors, tokenizer)
2. **Implement Q1.2** — Attention layers (linear + full)
3. **Validate** — Teacher-forcing against transformers oracle
4. **Iterate** — Fix bugs, optimize, add features

This plan provides a clear roadmap for implementing Qwen3.5 MoE support in
colibri. The key is to start with a working bf16 implementation, validate
correctness, then add quantization and optimization.

---

## Notes for GitHub Copilot

When implementing this plan, use these patterns:

1. **Follow glm.c structure** — same file organization, same helper functions
2. **Use existing patterns** — OpenMP, quantization, expert caching
3. **Validate early** — teacher-forcing tests before optimization
4. **Document clearly** — comments in C, README updates, benchmark results

The goal is a working, correct implementation first, then optimized.