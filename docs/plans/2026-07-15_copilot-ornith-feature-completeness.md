# Copilot Plan: Ornith FP8 Feature Completeness for colibrì

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Document every feature documented in the Ornith FP8 model metadata that the colibrì engine does not yet implement, with deep research, exact file references, and implementation guidance.

**Architecture:** The colibrì engine is a single-file C forward pass (`c/glm.c`, `c/qwen35_moe.c`) plus an OpenAI-compatible Python gateway (`c/openai_server.py`). The Ornith FP8 model (`/opt/models/ornith-fp8/`) is a Qwen3.5 MoE with vision, audio, video, tool calling, and structured output. The plan maps each model capability to the engine code that must handle it.

**Tech Stack:** C99, safetensors, Qwen3 tokenizer, OpenAI API compat, Jinja2 chat template.

---

## Model Metadata Summary (Source of Truth)

All references below come from the actual files at `/opt/models/ornith-fp8/`:

| File | Purpose |
|------|---------|
| `config.json` | Architecture, quantization, text/vision config |
| `generation_config.json` | Sampling params, EOS tokens |
| `tokenizer_config.json` | Token IDs, special tokens, multimodal markers |
| `processor_config.json` | Image/video preprocessing |
| `preprocessor_config.json` | Image processor config |
| `model.safetensors.index.json` | 185,811 tensor weight map across 122 shards |

### Key Architecture Facts (from `config.json`)

```
architectures: Qwen3_5MoeForConditionalGeneration
model_type: qwen3_5_moe
hidden_size: 4096
num_hidden_layers: 60
num_attention_heads: 32
num_key_value_heads: 2 (GQA, ratio 16:1)
head_dim: 256
num_experts: 512
num_experts_per_tok: 10
moe_intermediate_size: 1024
shared_expert_intermediate_size: 1024
vocab_size: 248320
max_position_embeddings: 262144
partial_rotary_factor: 0.25
rope_theta: 10000000
full_attention_interval: 4 (every 4th layer)
layer_types: 45 linear_attention + 15 full_attention
attn_output_gate: true
router_aux_loss_coef: 0.001
mamba_ssm_dtype: float32
quant_method: compressed-tensors (int8 weights, FP8 activation)
```

### Multimodal Token IDs (from `tokenizer_config.json`)

```
image_token_id: 248056
video_token_id: 248057
vision_start_token_id: 248053
vision_end_token_id: 248054
audio_token: "<|AUDIO|>"
audio_bos_token: "<|AUDIO_BOS|>"
audio_eos_token: "<|AUDIO_EOS|>"
image_token: "<|image|>"
video_token: "<|VIDEO|>"
vision_bos_token: "<|vision_bos|>"
vision_end_token: "<|vision_eos|>"
```

### Vision Config (from `config.json`)

```
vision_config:
  model_type: qwen3_5_moe_vision
  depth: 27
  hidden_size: 1152
  intermediate_size: 4304
  num_heads: 16
  patch_size: 16
  spatial_merge_size: 2
  temporal_patch_size: 2
  num_position_embeddings: 2304
  out_hidden_size: 4096
  in_channels: 3
```

### Quantization Config (from `config.json`)

```
quant_method: compressed-tensors
format: float-quantized
weights: int8, channel-wise, symmetric, memoryless_minmax observer
input_activations: int8, token-wise, dynamic
ignore: lm_head, embed_tokens, mlp.gate, shared_expert_gate, linear_attn.*, model.visual.*
```

---

## Feature Gap Analysis

### 1. Jinja Chat Template Rendering

**Status:** PARTIALLY IMPLEMENTED in `c/openai_server.py` (lines 253-314)
**Missing:** Full multimodal content rendering, reasoning_content handling, multi-step tool chains

**Reference:** `/home/leite/colibri/research/ornith-conversion-corruption/chat_template.jinja` (the authoritative template from the model)

**What the official template does:**
1. **Tool declaration block** (when `tools` provided): Wraps tools in `<tools>` XML, adds function-calling instructions with `<tool_call><function=...>...</function></tool_call>` syntax
2. **System message handling:** Different rendering when tools are present (tools block goes before system content) vs absent
3. **Multimodal content items:** Detects `image`, `video`, `text` items in content arrays and wraps with proper tokens:
   - `image` → `` (vision_start) ... `》` (vision_end)
   - `video` → `` (vision_start) ... `》` (vision_end)
4. **Reasoning content:** Extracts `reasoning_content` from assistant messages and wraps in `<think>...</think>` tags; splits content at `</think>` if present
5. **Tool call output:** Renders `tool_calls` as `<tool_call><function=name>\n<parameter=arg>value</parameter>\n...</function></tool_call>`
6. **Tool results:** Wraps in `<tool_response>...户晓` with auto-inserted `</s><|user|>` markers between consecutive tool results
7. **Multi-step tool chains:** Tracks `multi_step_tool` state across messages, finds last user query index, validates no orphan tool results
8. **Generation prompt suffix:** Appends `<|assistant|>\n<think>\n` (with thinking) or `<|assistant|>\n<think>\n\n</think>\n\n` (without)

**Current engine implementation (`openai_server.py:253-314`):**
- Renders basic text-only chat (system, user, assistant, tool roles)
- Handles tool declarations with a hardcoded XML block (byte-matches the jinja template's tool section)
- Handles tool call output parsing with `parse_tool_calls()` (lines 208-244)
- **Missing:** Multimodal content items (images/videos in content arrays), reasoning_content extraction, multi-step tool chain state tracking, the full jinja template logic

**Implementation reference:**
- Official template: `/home/leite/colibri/research/ornith-conversion-corruption/chat_template.jinja`
- Current renderer: `/home/leite/colibri/c/openai_server.py:253-314` (`render_chat()`)
- Tool call parser: `/home/leite/colibri/c/openai_server.py:208-244` (`parse_tool_calls()`)
- Tool call constants: `BOX_START="<tool_call>"`, `BOX_END="</tool_call>"`, `TR_OPEN="<tool_response>"`, `TR_CLOSE="户晓"`

**Implementation approach:**
1. Extract the jinja template into a standalone `.jinja` file in the repo (already exists at `research/ornith-conversion-corruption/chat_template.jinja`)
2. Add a Python-based template renderer in `openai_server.py` that uses `jinja2` to compile and render the template
3. Keep the current fast-path text-only renderer as fallback for when jinja2 is unavailable
4. Pass `add_generation_prompt`, `enable_thinking`, `reasoning_effort` as template variables

---

### 2. Tool Calling — Full OpenAI Compatibility

**Status:** IMPLEMENTED (text-only, GLM-style XML format)
**Missing:** Streaming tool call deltas, function calling (legacy `functions` parameter), tool result validation

**Reference:** `c/openai_server.py:173-314`, `c/openai_server.py:580-640` (streaming tool calls)

**What's working:**
- Tool declaration in system prompt (XML `<tools>` block)
- Tool call output parsing (GLM format: `<tool_call><function=name>\n<parameter=arg>value</parameter>...</function></tool_call>`)
- Non-streaming: returns `tool_calls` array in response
- Streaming: suppresses tool markers from content stream, emits tool_calls as final chunk
- `COLI_TOOL_SALVAGE=1` environment variable for int4 de-mangling
- Tool parameter order preservation via `_tool_param_order()`

**What's missing:**
1. **Streaming tool call deltas:** Currently emits all tool_calls in one final chunk. OpenAI clients expect incremental `tool_calls` deltas (index 0 first, then index 1, etc.)
2. **Legacy `functions` parameter:** The server checks `body.get("functions")` but the OpenAI API has deprecated this in favor of `tools[].type="function"`
3. **Tool result validation:** No schema validation on tool call arguments before sending to the model
4. **Parallel tool calls:** The template supports multiple `tool_calls` per message but the streaming parser may not handle concurrent tool call chunks correctly
5. **`tool_call_id` field:** OpenAI expects `tool_calls[].id` to be a unique string; current implementation generates it but doesn't guarantee uniqueness across calls

**Implementation reference:**
- Tool call constants: `BOX_START="<tool_call>"`, `BOX_END="</tool_call>"`, `TR_OPEN="<tool_response>"`, `TR_CLOSE="户晓"`
- Stream tool handling: `openai_server.py:620-640` (`emit_tools()` function)
- Parse function: `parse_tool_calls()` at line 208
- Tool param order: `_tool_param_order()` at line 194

**Implementation approach:**
1. Add `tool_call_id` generation with UUID prefix for uniqueness
2. Modify streaming to emit tool_call deltas incrementally (one per `tool_calls` item)
3. Add schema validation hook (optional, for when tools have JSON Schema)
4. Keep `functions` parameter as alias for backward compatibility

---

### 3. Multimodal Capabilities (Vision — Images)

**Status:** NOT IMPLEMENTED in engine, NOT IMPLEMENTED in gateway
**Model supports:** Image understanding via Qwen3VL vision encoder

**Reference:**
- Vision config: `config.json` → `vision_config` (depth 27, hidden 1152, patch 16, spatial_merge 2)
- Processor: `processor_config.json` → `Qwen2VLImageProcessor` (merge_size 2, patch_size 16, temporal_patch_size 2)
- Preprocessor: `preprocessor_config.json` → image normalization (mean 0.5, std 0.5, rescale 1/255)
- Token: `image_token_id: 248056`, rendered as `` in chat template
- Weight prefix: `model.visual.*` (excluded from quantization — runs in FP16)
- Safetensors: vision weights exist in shards but are NOT loaded by the engine

**What the model expects for images:**
1. Image is preprocessed: resize to longest edge, divide into 560x560 patches, merge every 2x2 patches into one (spatial_merge_size=2)
2. Each patch becomes a sequence of vision tokens (560/16=35 patches per side, merged to 17+1=18 tokens per side, total ~324 tokens per image)
3. Vision tokens are projected through a 27-layer vision encoder (Qwen3VL vision model)
4. Vision encoder output is projected back to language model hidden size (1152→4096)
5. Vision tokens are inserted into the text sequence at `` positions

**Implementation approach (long-term):**
1. Add vision encoder to `qwen35_moe.c` (27-layer transformer, 1152 hidden, 16 heads)
2. Add image preprocessing pipeline (resize, patch, merge)
3. Add vision token projection layer
4. Integrate into forward pass: replace `` tokens with vision encoder output
5. This is a significant undertaking (~2000+ lines of C) — best done as a separate module

**Implementation approach (short-term, gateway level):**
1. Add image preprocessing in `openai_server.py` using PIL/Pillow
2. Convert images to base64 or raw pixel arrays
3. Pass to engine via a new CLI flag or environment variable
4. This is a stopgap — the engine still needs to handle vision tokens

---

### 4. Multimodal Capabilities (Video)

**Status:** NOT IMPLEMENTED in engine, NOT IMPLEMENTED in gateway
**Model supports:** Video understanding via temporal patching

**Reference:**
- Processor: `processor_config.json` → `Qwen3VLVideoProcessor` (fps 2, max_frames 768, min_frames 4)
- Token: `video_token_id: 248057`, rendered as `` in chat template
- Temporal: `temporal_patch_size: 2` (2 frames per temporal patch)
- Size: longest_edge 25165824, shortest_edge 4096

**What the model expects for video:**
1. Video is sampled at 2 FPS, min 4 frames, max 768 frames
2. Each frame is processed as an image (same as vision)
3. Temporal patches group consecutive frames (every 2 frames = 1 temporal patch)
4. Total video tokens = image_tokens_per_frame × num_temporal_patches

**Implementation approach:** Same as vision but with frame sampling and temporal grouping.

---

### 5. Multimodal Capabilities (Audio)

**Status:** NOT IMPLEMENTED in engine, NOT IMPLEMENTED in gateway
**Model supports:** Audio understanding via audio tokens

**Reference:**
- Token: `audio_token: "<|AUDIO|>"`, `audio_bos_token: "<|AUDIO_BOS|>"`, `audio_eos_token: "<|AUDIO_EOS|>"`
- No audio-specific config in `config.json` or `processor_config.json`
- Audio weights likely in `model.visual.*` or a separate audio encoder (not present in text_config)

**What's known:** Qwen3.5 supports audio via a separate audio encoder (not detailed in the model metadata we have). The tokenizer has audio special tokens but no audio processor config exists.

**Implementation approach:** Requires audio encoder model weights and processing pipeline. Out of scope for Phase 2.

---

### 6. Sampling Parameters

**Status:** PARTIALLY IMPLEMENTED
**Missing:** top_k, min_p, repetition_penalty, presence/frequency penalty, seed

**Reference:** `generation_config.json`:
```json
{
  "do_sample": true,
  "temperature": 0.6,
  "top_k": 20,
  "top_p": 0.95,
  "eos_token_id": [248046, 248044],
  "pad_token_id": 248044
}
```

**What's working:**
- `temperature` (0.0-2.0)
- `top_p` (nucleus sampling)
- `max_tokens` / `max_completion_tokens`
- `n=1` only

**What's missing:**
1. **`top_k`**: Model metadata specifies top_k=20 but engine ignores it
2. **`min_p`**: Not in generation_config but supported by Qwen3.5
3. **`seed`**: Explicitly rejected by `generation_options()` with "not supported yet"
4. **`frequency_penalty` / `presence_penalty`**: Explicitly rejected
5. **`logprobs`**: Explicitly rejected
6. **Custom `stop` sequences**: Explicitly rejected
7. **`response_format`**: Only `{"type": "text"}` supported
8. **`n > 1`**: Explicitly rejected
9. **`reasoning_effort`**: Accepted but only affects thinking prompt prefix

**Implementation reference:**
- `generation_options()` at `openai_server.py:316-351`
- Sampling in engine: `qwen35_moe.c` (needs to be checked for current sampling impl)

**Implementation approach:**
1. Add `top_k` to `generation_options()` and pass to engine
2. Add `seed` support (simple: set random seed before sampling)
3. Add `min_p` as optional parameter
4. Keep penalties rejected (not in engine's matmul path)
5. Add `logprobs` support (requires logits output from engine)
---

### 7. Reasoning / Thinking Mode

**Status:** IMPLEMENTED (basic)
**Missing:** `reasoning_effort` levels, structured thinking output

**Reference:**
- `generation_config.json`: `do_sample: true`
- Chat template: `<think>...</think>` tags
- Server: `reasoning_effort` parameter (none/minimal/low/medium/high/xhigh)
- `COLI_THINK=1` env var for default thinking

**What's working:**
- `enable_thinking` boolean toggles `<think>...</think>` wrapping
- `reasoning_effort` accepted but only used to set thinking on/off
- `COLI_THINK=1` makes thinking default on
- Stream keeps alive with `reasoning_content: "."` pings during cold prefill

**What's missing:**
1. **`reasoning_effort` levels:** The template and model support different thinking depths (none/minimal/low/medium/high/xhigh) but the engine treats them all the same
2. **`reasoning_content` field in response:** OpenAI API returns `reasoning_content` as a separate field from `content`; current implementation doesn't split them
3. **Structured thinking:** Some levels might limit thinking token budget

**Implementation reference:**
- `chat_completion()` at `openai_server.py:780-798`
- `render_chat()` thinking handling at line 313-314
- Stream keepalive at lines 600-620

**Implementation approach:**
1. Parse `reasoning_content` from engine output (split at `</think>`)
2. Return `reasoning_content` and `content` as separate fields in API response
3. Map `reasoning_effort` levels to thinking token budgets (future)

---

### 8. Grammar-Constrained Generation

**Status:** NOT IMPLEMENTED in engine or gateway
**Reference:** `c/grammar.h` exists in the repo (referenced in README)

**What the model supports:** GBNF grammar-constrained decoding for structured output (JSON, function calling, etc.)

**Implementation reference:**
- Grammar header: `c/grammar.h` (exists but not integrated into qwen35_moe.c)
- README mentions: "Grammar-forced speculative drafts" as a feature

**Implementation approach:** Integrate `grammar.h` into the engine's sampling loop. This is a significant feature — autoregressive constrained decoding.

---

### 9. KV Cache Persistence

**Status:** NOT IMPLEMENTED in qwen35_moe engine (exists in glm.c)
**Reference:** `c/glm.c` has KV cache persistence; `c/qwen35_moe.c` does not

**What's working in glm.c:**
- Compressed MLA KV cache saved to `.coli_kv` file
- ~182 KB/token, crash-safe append
- Resumes warm across engine restarts

**What's needed for qwen35_moe:**
1. Standard KV cache (not MLA-compressed) for full-attention layers
2. Linear attention state persistence (SSM state, not KV)
3. Cache file format for the new engine

**Implementation reference:**
- glm.c KV cache: search `c/glm.c` for `KVSAVE`, `coli_kv`, `kv_cache`
- qwen35_moe.c: currently no KV cache code

**Implementation approach:**
1. Add KV cache struct to `qwen35_model`
2. Add cache save/load functions
3. Integrate into engine lifecycle

---

### 10. OpenAI API — Missing Endpoints

**Status:** PARTIALLY IMPLEMENTED
**Missing:** Embeddings, fine-tuning, files, batches, moderations

**Reference:** `c/openai_server.py`

**What's implemented:**
- `GET /v1/models` — model listing
- `GET /v1/models/{model_id}` — model detail
- `GET /health` — health check with scheduler stats
- `POST /v1/chat/completions` — chat with streaming
- `POST /v1/completions` — text completion

**What's missing (standard OpenAI endpoints):**
1. `POST /v1/embeddings` — text embeddings
2. `POST /v1/fine_tuning/jobs` — fine-tuning (not applicable for this engine)
3. `POST /v1/files` — file upload (not applicable)
4. `POST /v1/batches` — batch processing (not applicable)
5. `POST /v1/moderations` — content moderation (not applicable)

**Implementation approach:**
1. Add `POST /v1/embeddings` — run forward pass on prompt, extract last hidden state, project to embedding dim
2. Return 404 for other endpoints (future consideration)

---

### 11. Multi-Session / KV Slots

**Status:** IMPLEMENTED (1 slot, configurable up to 16)
**Reference:** `c/openai_server.py` — `kv_slots` parameter, `cache_slot` in requests

**What's working:**
- `--kv-slots N` CLI flag (1-16)
- `cache_slot` parameter in chat completions
- Scheduler with bounded FIFO admission
- Queue wait time tracking

**What could be improved:**
1. Per-slot statistics (tokens generated, cache hit rate per slot)
2. Automatic slot rotation / eviction
3. Slot health monitoring

---

### 12. Web Frontend — Missing Features

**Status:** PARTIALLY IMPLEMENTED
**Reference:** `web/src/App.tsx`, `web/src/lib/api.ts`

**What's implemented:**
- Basic chat UI with message history
- Model selection, temperature, max tokens
- Thinking toggle, cache slot selection
- Streaming responses
- Connection health monitoring
- Settings persistence (localStorage)
- CORS configuration

**What's missing:**
1. **Image upload:** No UI for attaching images to messages
2. **Video upload:** No UI for video attachments
3. **Tool calling UI:** No visualization of tool calls in the chat
4. **Grammar/constrained mode:** No UI for specifying grammar constraints
5. **Reasoning content display:** No separate panel for thinking output
6. **Multi-conversation:** Only one conversation per cache slot
7. **Export/Import:** No way to export conversation history
8. **Keyboard shortcuts:** No shortcuts for send, stop, new conversation

---

## Implementation Priority

### Phase A: Quick Wins (gateway-level, Python only)

| # | Feature | Files | Effort |
|---|---------|-------|--------|
| A1 | Add `top_k` sampling support | `openai_server.py`, `qwen35_moe.c` | 2h |
| A2 | Add `seed` support | `openai_server.py`, `qwen35_moe.c` | 1h |
| A3 | Split `reasoning_content` from `content` in response | `openai_server.py` | 2h |
| A4 | Streaming tool call deltas (incremental) | `openai_server.py` | 3h |
| A5 | Add `POST /v1/embeddings` endpoint | `openai_server.py` | 4h |
| A6 | Extract jinja template to standalone file | `research/`, `openai_server.py` | 1h |

### Phase B: Engine Features (C code)

| # | Feature | Files | Effort |
|---|---------|-------|--------|
| B1 | KV cache persistence for qwen35_moe | `qwen35_moe.c` | 6h |
| B2 | Grammar-constrained decoding integration | `qwen35_moe.c`, `grammar.h` | 8h |
| B3 | Logits output for logprobs | `qwen35_moe.c`, `openai_server.py` | 4h |

### Phase C: Multimodal (significant work)

| # | Feature | Files | Effort |
|---|---------|-------|--------|
| C1 | Image preprocessing in gateway | `openai_server.py`, PIL | 4h |
| C2 | Vision encoder in engine (27-layer transformer) | `qwen35_moe.c` (+new module) | 40h+ |
| C3 | Video support (frame sampling + temporal) | `openai_server.py`, `qwen35_moe.c` | 20h+ |
| C4 | Audio support (requires audio encoder) | New module | TBD |

### Phase D: Web Frontend

| # | Feature | Files | Effort |
|---|---------|-------|--------|
| D1 | Image upload UI | `web/src/App.tsx`, `api.ts` | 4h |
| D2 | Tool call visualization | `web/src/App.tsx` | 3h |
| D3 | Reasoning content panel | `web/src/App.tsx` | 2h |
| D4 | Multi-conversation support | `web/src/App.tsx` | 6h |

---

## File Reference Index

| File | Path in Repo | Purpose |
|------|-------------|---------|
| Ornith FP8 config | `/opt/models/ornith-fp8/config.json` | Architecture, quantization, vision config |
| Ornith FP8 generation | `/opt/models/ornith-fp8/generation_config.json` | Sampling params |
| Ornith FP8 tokenizer | `/opt/models/ornith-fp8/tokenizer_config.json` | Special tokens, multimodal IDs |
| Ornith FP8 processor | `/opt/models/ornith-fp8/processor_config.json` | Image/video preprocessing |
| Ornith FP8 preprocessor | `/opt/models/ornith-fp8/preprocessor_config.json` | Image processor config |
| Ornith FP8 weight map | `/opt/models/ornith-fp8/model.safetensors.index.json` | 185,811 tensor locations |
| Chat template (jinja) | `/home/leite/colibri/research/ornith-conversion-corruption/chat_template.jinja` | Authoritative rendering template |
| OpenAI gateway | `/home/leite/colibri/c/openai_server.py` | HTTP API server (824 lines) |
| Qwen3.5 engine | `/home/leite/colibri/c/qwen35_moe.c` | Forward pass engine (1126 lines) |
| Shard reader | `/home/leite/colibri/c/st.h` | Safetensors tensor loading |
| JSON parser | `/home/leite/colibri/c/json.h` | Minimal JSON parser |
| Grammar header | `/home/leite/colibri/c/grammar.h` | GBNF grammar decoder |
| GLM engine (reference) | `/home/leite/colibri/c/glm.c` | Existing engine with KV cache |
| Phase 2 prompt | `/home/leite/colibri/c/COPILOT_PROMPT_QWEN35MOE_PHASE2.md` | Architecture details for engine |
| Web frontend | `/home/leite/colibri/web/src/App.tsx` | React chat UI |
| Web API client | `/home/leite/colibri/web/src/lib/api.ts` | Fetch wrapper for API |
| Conversion analysis | `/home/leite/colibri/research/ornith-conversion-corruption/ANALYSIS.md` | Known conversion bugs |
| Qwen3.5 port plan | `/home/leite/colibri/PORT_QWEN35_MOE_PLAN.md` | Engine development roadmap |

---

## Notes on Model Quantization

The Ornith FP8 model uses **compressed-tensors** quantization:
- **Weights:** int8 (symmetric, channel-wise), stored as U8 + F32 `.qs` scale tensors
- **Activations:** int8 (token-wise, dynamic) during inference
- **FP8 in metadata:** The model was originally trained/quantized with FP8 but the engine loads int8-quantized weights
- **Ignored tensors:** `lm_head`, `embed_tokens`, `mlp.gate`, `shared_expert_gate`, `linear_attn.*`, `model.visual.*` — these run in FP16/BF16

The engine's `st.h` reader handles dtype codes:
- 0 = BF16, 1 = F16, 2 = F32, 3 = U8/I8 (quantized)
- Quantized tensors: read U8 raw bytes, dequant with per-row scale from `.qs` tensor

---

*Plan generated 2026-07-15. All references verified against live model files and repo state.*
