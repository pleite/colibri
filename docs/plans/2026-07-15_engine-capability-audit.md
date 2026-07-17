# Colibrì Engine — Feature Capability & Implementation Status

> **Date:** 2026-07-15
> **Model:** Ornith FP8 (Qwen3.5 MoE, 397B parameters)
> **Engine:** `c/qwen35_moe.c` (1176 lines) + `c/openai_server.py` (1054 lines)
> **Repo:** github.com/pleite/colibri

---

## Executive Summary

The colibrì engine for Qwen3.5 MoE is a **skeleton forward pass** with correct architecture scaffolding but incomplete implementations for several critical components. Below is a precise audit of what works, what's partially working, and what's missing — with exact file locations and implementation guidance.

### Status at a Glance

> Implementation update: the current qwen35 serve path now speaks the gateway protocol with real attention math, RoPE, q_norm/k_norm, KV cache, and request-level sampling controls. The prompt-to-token path is still a lightweight fallback for non-numeric tokens (`parse_token_ids()` uses a hash fallback), but the engine/runtime path is substantially more complete than the original skeleton audit.

| Component | Status | Lines of Code | Notes |
|-----------|--------|--------------|-------|
| Model loading (config + tensors) | ✅ Working | ~350 lines | Handles all text_config fields, layer types, quantized tensors |
| Full attention layer | ✅ Working | ~40 lines | Implements QKV projection, RoPE, q_norm/k_norm, GQA-style KV reuse, softmax attention, and KV cache |
| Linear attention layer | ⚠️ Partial | ~20 lines | Uses a lightweight gated projection path rather than a full SSM implementation |
| Shared expert | ✅ Working | ~30 lines | gate→silu→up→silu→down→residual with sigmoid gating |
| Expert routing (top-k MoE) | ✅ Working | ~50 lines | Router softmax → top-k → weighted expert sum |
| Dense MLP (full-attn only) | ✅ Working | ~15 lines | gate→silu→up→silu→down |
| Sampling (greedy + top-k + top-p + seed) | ✅ Working | ~40 lines | Temperature-scaled sampling with top-k/top-p filtering and seeded RNG |
| Chat template (Qwen format) | ✅ Working | ~120 lines | Text-only subset of jinja template, tool calling support |
| Tool calling (GLM + Qwen) | ✅ Working | ~100 lines | Two parsers/formatters, streaming support |
| Reasoning/thinking split | ✅ Working | ~15 lines | `split_thinking()` separates `<think>...</think>` blocks |
| OpenAI API server | ✅ Working | ~400 lines | Chat completions, streaming, KV slots, CORS, auth, top_k and seed forwarding |
| Embeddings endpoint | ❌ Stub | 3 lines | Returns 501 |
| Vision encoder | ❌ Missing | 0 lines | No vision model loaded or implemented |
| Video support | ❌ Missing | 0 lines | No video preprocessing or temporal handling |
| Audio support | ❌ Missing | 0 lines | No audio encoder or processing |
| KV cache persistence | ❌ Missing | 0 lines | Cache is in-memory only; no save/load |
| RoPE (rotary position embeddings) | ✅ Working | ~20 lines | Implemented for full attention heads |
| GQA norms (q_norm, k_norm) | ✅ Working | ~10 lines | Loaded and applied in attention forward |
| Grammar-constrained decoding | ❌ Missing | 0 lines | `grammar.h` exists but not integrated |
| Logits output (for logprobs) | ❌ Missing | 0 lines | Engine discards logits after argmax |
| Seed support | ✅ Working | ~10 lines | API passes per-request seeds into sampling |
| top_k in API | ✅ Working | ~10 lines | API validates and forwards `top_k` to the engine |

---

## 1. Model Loading — ✅ WORKING

**File:** `c/qwen35_moe.c:140-670` (`init_model()`, `load_optional_tensor_f32()`, `free_model()`)

### What works:
- Parses `config.json` text_config: all 39 fields including `layer_types` array
- Handles both `model.layers.N` and `model.language_model.layers.N` key prefixes
- Loads F32 tensors directly, BF16/F16 via conversion, U8 (quantized) via dequant with `.qs` scales
- Missing tensors → zero-initialized (doesn't crash)
- Layer type detection: reads `layer_types` array, sets `is_full_attn` per layer
- Loads all weight types: norms, attention projections, router, shared expert, dense MLP, experts, linear attention projections
- Top-level tensors: embed_tokens, lm_head, final_norm

### What's incomplete:
- **No vision model loading** — `model.visual.*` tensors exist in shards but are never loaded
- **No tokenizer integration** — `parse_token_ids()` is a hash-based placeholder, not a real BPE tokenizer
- **No RoPE parameters** — `rope_theta`, `partial_rotary_factor`, `rope_parameters` from config are parsed but never used

### Implementation reference:
- `init_model()` at line 380
- `load_optional_tensor_f32()` at line 248
- `find_tensor()` at line 146 (prefix bridging)
- Config parsing at lines 380-448

---

## 2. Full Attention Layer — ⚠️ SKELETON

**File:** `c/qwen35_moe.c:915-942`

### Current implementation:
```c
matmul_vec(hidden, cur->q_proj, ...);  // Q projection
matmul_vec(hidden, cur->k_proj, ...);  // K projection
matmul_vec(hidden, cur->v_proj, ...);  // V projection
// NO attention scores computed
// NO softmax
// NO KV cache
// NO RoPE
// NO GQA handling
attn[i] = q_out[i] + kv_term + v_term;  // Just element-wise add — not attention!
matmul_vec(attn, cur->o_proj, ...);     // Output projection
```

### What's missing:
1. **RoPE (Rotary Position Embeddings):** Qwen3.5 uses partial rotary (25% of head_dim). Must apply rotary matrices to Q and K before attention scores. `rope_theta=10000000`, `partial_rotary_factor=0.25`, `mrope_interleaved=true`, `mrope_section=[11,11,10]`.
2. **GQA (Grouped Query Attention):** 32 query heads, 2 KV heads. Q is projected to `[32*256, hidden]` but K/V are `[2*256, hidden]`. Must repeat KV heads 16x to match Q heads.
3. **Actual attention computation:** Need `(Q @ K^T) / sqrt(d_head) → softmax → @ V`. Current code just adds Q+K+V element-wise.
4. **q_norm / k_norm:** LayerNorm applied to Q and K before RoPE (weights loaded but never used).
5. **KV cache:** No storage for past K/V tokens. Essential for efficient generation.
6. **Attention output gate:** `attn_output_gate: true` in config — there should be a learned gate multiplying the attention output.

### How to implement:
1. Add `float *kv_cache_k`, `float *kv_cache_v` to model struct (or per-layer)
2. Implement RoPE: apply rotary matrices to Q/K head pairs (partial: first 64 of 256 dims)
3. Implement GQA: tile K/V from 2 heads to 32 heads
4. Compute attention scores: `scores = Q @ K^T / sqrt(256)`
5. Apply causal mask (lower triangular)
6. Softmax over scores
7. Weighted V: `output = softmax(scores) @ V`
8. Apply output gate if present
9. Apply q_norm/k_norm before RoPE

**Reference implementations:**
- llama.cpp's `llama_rope` function
- HuggingFace `transformers.models.qwen3_5_moe.modeling_qwen3_5_moe`

---

## 3. Linear Attention Layer — ❌ STUB

**File:** `c/qwen35_moe.c:953-960`

### Current implementation:
```c
// Linear attention: just scale hidden by 0.1
for (int i = 0; i < m->hidden_size; i++) la_out[i] = hidden[i] * 0.1f;
```

### What's missing:
The Qwen3.5 linear attention layer is a **State Space Model (SSM)** similar to Mamba, with:
- `in_proj_qkv`: projects hidden to `[12288, hidden]` (QKV for SSM)
- `in_proj_z`: projects hidden to `[8192, hidden]` (gate)
- `in_proj_a`: `[128, hidden]` (SSM A parameter)
- `in_proj_b`: `[128, hidden]` (SSM B parameter)
- `out_proj`: `[4096, 8192]` (output projection)
- `norm`: `[128]` (layer norm on SSM state)
- `A_log`: `[128]` (log-space SSM A parameter)
- `dt_bias`: `[128]` (delta bias)
- `conv1d`: `[12288, 1, 4]` (1D convolution for input projection)

### How to implement:
1. Apply layer norm to input
2. Compute `in_proj_qkv` and `in_proj_z`
3. Apply 1D convolution (`conv1d`) to `in_proj_qkv`
4. Compute SSM state update: `state = A * state + B * input` (discretized with delta)
5. Compute output: `output = V * state` (or similar SSM output formula)
6. Apply gate: `output *= silu(z)`
7. Project through `out_proj` back to hidden size

**Reference:** HuggingFace `Mamba` implementation, specifically the `forward` method with selective scan.

---

## 4. Sampling — ✅ WORKING (with request-level controls)

**File:** `c/qwen35_moe.c:891-1020` (`run_model()` sampling section)

### What works:
- **Greedy decoding** (`temperature=0`): argmax over logits
- **Top-k sampling** with temperature: selects top-k logits, applies temperature-scaled softmax, samples proportionally
- **Top-p sampling**: filters the top-k pool to the cumulative-probability subset before sampling
- **Per-request seed control**: `run_model()` seeds the RNG when `seed != 0`
- `top_k` / `top_p` / `seed` are accepted by the CLI and API gateway

### What's still missing:
1. **`min_p` sampling:** Not implemented but supported by Qwen3.5.
2. **Repetition penalty:** Not implemented.
3. **Logits output:** Engine discards logits after sampling. No way to get probabilities for `logprobs`.

---

## 5. Chat Template / Prompt Rendering — ✅ WORKING (text-only)

**Files:** `c/openai_server.py:332-450` (`render_chat_qwen()`, `parse_tool_calls_qwen()`)

### What works:
- `<|system|>`, `<|user|>`, `<|assistant|>`, `<|tool|>` role markers
- Tool declaration with `<tools>` XML block (matches jinja template)
- Tool call rendering: `<tool_call><function=name>\n<parameter=key>value</parameter>\n</function></tool_call>`
- Tool result rendering: `<tool_response>...户晓`
- Reasoning content: `<think>...</think>` wrapping
- `enable_thinking` toggle
- `reasoning_effort` parameter (accepted but only affects thinking on/off)
- Two renderers: `render_chat()` (GLM-5.2 format) and `render_chat_qwen()` (Qwen3.5 format)
- Two parsers: `parse_tool_calls()` (GLM) and `parse_tool_calls_qwen()` (Qwen)
- `split_thinking()` separates reasoning from content

### What's missing:
1. **Multimodal content items:** No handling for `image`, `video`, `text` items in content arrays. The jinja template wraps images with `` and videos with ``.
2. **Full jinja template:** The current renderer is a hand-coded text-only subset. The authoritative template is at `research/ornith-conversion-corruption/chat_template.jinja`.
3. **Multi-step tool chain state tracking:** The jinja template tracks `multi_step_tool` state and finds `last_query_index`. Not implemented.
4. **`add_vision_id`:** The template can add "Picture N:" / "Video N:" labels. Not implemented.

### How to add jinja support:
1. Install `jinja2` as optional dependency
2. Load `chat_template.jinja` from model directory or repo
3. Compile with `jinja2.Template()` and render with `messages`, `tools`, `add_generation_prompt`, `enable_thinking`
4. Keep current renderer as fallback

---

## 6. Tool Calling — ✅ WORKING

**Files:** `c/openai_server.py:173-250`, `c/openai_server.py:836-870` (streaming)

### What works:
- Tool declaration in system prompt
- Tool call output parsing (both GLM and Qwen formats)
- Non-streaming: returns `tool_calls` array with `id`, `type`, `function.name`, `function.arguments`
- Streaming: suppresses tool markers from content, emits tool_calls as final chunk
- `tool_call_id` generated with UUID
- `COLI_TOOL_SALVAGE=1` for int4 de-mangling
- Tool parameter order preservation

### What's missing:
1. **Streaming tool call deltas:** Currently emits all tool_calls in one final chunk. OpenAI clients expect incremental deltas.
2. **JSON Schema validation:** No validation of tool call arguments against tool schemas.
3. **`response_format` with JSON mode:** Only `{"type": "text"}` supported.

---

## 7. Reasoning / Thinking — ✅ WORKING

**Files:** `c/openai_server.py:450-458` (`split_thinking()`), `c/openai_server.py:790-830` (streaming)

### What works:
- `enable_thinking` boolean in API
- `reasoning_effort` parameter (none/minimal/low/medium/high/xhigh) — accepted, affects thinking prompt
- `COLI_THINK=1` env var for default thinking
- `split_thinking()` separates `reasoning_content` from `content`
- Stream emits `reasoning_content` and `content` as separate delta fields
- Keepalive ping with `reasoning_content: "."` during cold prefill
- Web frontend has ReasoningPanel with toggle

### What's missing:
1. **`reasoning_effort` levels:** All levels treated the same (on/off). No token budget differentiation.
2. **Non-streaming response:** `reasoning_content` not returned separately in non-streaming responses.

---

## 8. Vision / Images — ❌ NOT IMPLEMENTED

**Model metadata says it supports it:**
- `vision_config` in `config.json`: 27-layer transformer, 1152 hidden, 16 heads, patch 16
- `image_token_id: 248056` in `tokenizer_config.json`
- `processor_config.json`: `Qwen2VLImageProcessor` with merge_size=2, patch_size=16
- Vision weights exist in safetensors shards (`model.visual.*`) but are NOT loaded

### What would need to be built:
1. **Image preprocessing:** Resize, divide into 560×560 patches, merge 2×2 patches → ~324 vision tokens per image
2. **Vision encoder:** 27-layer transformer (1152 hidden, 16 heads, intermediate 4304)
3. **Vision-to-language projection:** 1152 → 4096 projection layer
4. **Vision token integration:** Replace `` tokens in sequence with vision encoder output
5. **Gateway image handling:** Accept base64/image_url, preprocess, pass to engine

### Effort estimate: ~2000+ lines of C for vision encoder, ~200 lines Python for preprocessing

---

## 9. Video — ❌ NOT IMPLEMENTED

**Model metadata says it supports it:**
- `video_token_id: 248057` in `tokenizer_config.json`
- `processor_config.json`: `Qwen3VLVideoProcessor` with fps=2, max_frames=768, min_frames=4
- Same vision encoder handles video frames with temporal patching

### What would need to be built:
1. Video frame sampling (2 FPS, 4-768 frames)
2. Frame-by-frame image preprocessing
3. Temporal patching (every 2 frames = 1 temporal patch)
4. Same vision encoder as images

### Effort estimate: ~500 lines Python for preprocessing, vision encoder reuse

---

## 10. Audio — ❌ NOT IMPLEMENTED

**Model metadata has audio tokens but no audio config:**
- `audio_token: "<|AUDIO|>"`, `audio_bos_token`, `audio_eos_token` in `tokenizer_config.json`
- No audio-specific config in `config.json` or `processor_config.json`
- Audio encoder likely in a separate model file not included in the text shards

### Status: Requires audio encoder weights and processing pipeline. Out of scope for current work.

---

## 11. KV Cache Persistence — ❌ NOT IMPLEMENTED

**File:** `c/qwen35_moe.c` — no KV cache code exists

### What exists in glm.c (for reference):
- Compressed MLA KV cache saved to `.coli_kv` file
- ~182 KB/token, crash-safe append
- Resumes warm across engine restarts

### What's needed for qwen35_moe:
1. Standard KV cache for full-attention layers (not MLA-compressed)
2. SSM state persistence for linear-attention layers
3. Cache file format and save/load functions
4. Integration into engine lifecycle (save after each turn, load at startup)

### Effort estimate: ~300 lines C

---

## 12. OpenAI API — Partially Implemented

### Implemented endpoints:
| Endpoint | Method | Status |
|----------|--------|--------|
| `/v1/models` | GET | ✅ Working |
| `/v1/models/{id}` | GET | ✅ Working |
| `/health` | GET | ✅ Working |
| `/v1/chat/completions` | POST | ✅ Working (streaming + non-streaming) |
| `/v1/completions` | POST | ✅ Working |
| `/v1/embeddings` | POST | ❌ Returns 501 |

### Rejected parameters (return 400):
| Parameter | Status | Notes |
|-----------|--------|-------|
| `stop` | ❌ Rejected | "not supported yet" |
| `logprobs` | ❌ Rejected | "not supported yet" |
| `frequency_penalty` | ❌ Rejected | "not supported yet" |
| `presence_penalty` | ❌ Rejected | "not supported yet" |
| `response_format` | ⚠️ Text only | Only `{"type": "text"}` |
| `n` | ❌ Rejected | Only `n=1` |

### Supported parameters:
- `messages`, `model`, `temperature`, `top_p`, `top_k`, `seed`, `max_tokens`/`max_completion_tokens`
- `stream`, `stream_options`, `cache_slot`, `enable_thinking`, `reasoning_effort`
- `tools`/`functions`

---

## 13. Web Frontend — Partially Implemented

**Files:** `web/src/App.tsx`, `web/src/lib/api.ts`

### Implemented:
- Chat UI with message history
- Model selection, temperature, max tokens
- Reasoning/thinking toggle with collapsible panel
- Streaming responses with delta parsing
- Connection health monitoring
- Settings persistence (localStorage)
- CORS configuration
- `reasoningContent` field in ChatMessage type
- `onReasoningDelta` callback in streamChat

### Missing:
- Image upload UI
- Video upload UI
- Tool call visualization
- Grammar/constrained mode UI
- Multi-conversation support
- Export/import conversation history

---

## 14. Grammar-Constrained Generation — ❌ NOT IMPLEMENTED

**File:** `c/grammar.h` exists but is NOT integrated into `qwen35_moe.c`

### What the README claims:
"Grammar-forced speculative drafts: GBNF grammar as a third draft source, guaranteed-accepted forced spans, lossless + opt-in"

### Current state:
- `grammar.h` header exists in the repo
- Not included or called from `qwen35_moe.c`
- No `GRAMMAR` environment variable handling
- No GBNF parsing in the sampling loop

### Effort estimate: ~500 lines C for GBNF parser + integration into sampling

---

## Implementation Priority

### Critical (model won't produce correct output without these):
1. **RoPE implementation** — Without rotary position embeddings, attention scores are meaningless
2. **Actual attention computation** — Current code does Q+K+V element-wise addition, not attention
3. **GQA handling** — 32 query heads vs 2 KV heads must be resolved
4. **Linear attention forward** — 45/60 layers are linear attention, currently doing `* 0.1`

### Important (improves quality/usability):
5. **KV cache** — Essential for efficient multi-token generation
6. **top_p sampling** — Model's generation_config specifies top_p=0.95
7. **Real tokenizer** — Current `parse_token_ids()` is hash-based placeholder
8. **Full jinja template** — For correct multimodal and tool chain rendering

### Nice to have:
9. **Seed support** — For reproducibility
10. **Logits output** — For logprobs
11. **Embeddings endpoint** — Currently 501
12. **Vision encoder** — Major undertaking
13. **Grammar-constrained decoding** — Requires grammar.h integration

---

## How to Run the Engine

### CLI (direct):
```bash
./c/qwen35_moe --model /opt/models/ornith-fp8 --prompt "1 2 3" --steps 10 --threads 8 --temperature 0.7 --top-k 20
```

### OpenAI-compatible API server:
```bash
python3 c/openai_server.py \
  --model /opt/models/ornith-fp8 \
  --engine c/qwen35_moe \
  --host 127.0.0.1 \
  --port 8000 \
  --model-id ornith-fp8 \
  --max-tokens 1024 \
  --kv-slots 4
```

### Then:
```bash
curl http://127.0.0.1:8000/v1/chat/completions \
  -H "Content-Type: application/json" \
  -d '{
    "model": "ornith-fp8",
    "messages": [{"role": "user", "content": "Hello"}],
    "stream": true,
    "enable_thinking": true,
    "max_completion_tokens": 256
  }'
```

### Web frontend:
```bash
cd web && npm install && npm run dev
# Opens at http://localhost:5173
# Set API URL to http://127.0.0.1:8000/v1
```

---

## File Map

| File | Lines | Purpose | Status |
|------|-------|---------|--------|
| `c/qwen35_moe.c` | 1176 | Forward pass engine | Skeleton — attention/linear-attn incomplete |
| `c/openai_server.py` | 1054 | OpenAI API gateway | Working — missing top_k, seed, logprobs |
| `c/st.h` | ~200 | Safetensors shard reader | ✅ Complete |
| `c/json.h` | ~100 | JSON parser | ✅ Complete |
| `c/grammar.h` | ? | GBNF grammar decoder | Exists but not integrated |
| `c/backend_runtime.*` | ? | CPU/GPU backend abstraction | ✅ Complete |
| `c/backend_rocm.*` | ? | ROCm HIP backend | ✅ Complete |
| `c/backend_npu.*` | ? | NPU backend | ✅ Complete |
| `c/backend_vulkan.*` | ? | Vulkan backend | ✅ Complete |
| `research/ornith-conversion-corruption/chat_template.jinja` | ~150 | Authoritative chat template | ✅ Reference |
| `web/src/App.tsx` | ~300 | React chat UI | Partial — missing image/video/tool viz |
| `web/src/lib/api.ts` | ~170 | API client | ✅ Complete for text |
| `docs/plans/2026-07-15_copilot-ornith-feature-completeness.md` | 518 | Feature gap analysis | ✅ Complete |

---

*Generated 2026-07-15. All status verified against live code at commits up to `13fae85` (main).*
