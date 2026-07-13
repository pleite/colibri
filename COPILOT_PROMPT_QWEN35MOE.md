COPILOT_PROMPT_START
# Copilot Prompt: Rewrite qwen35_moe.c as a Full Quantized Qwen3.5 MoE Engine

## Context
Rewrite /home/leite/colibri/c/qwen35_moe.c. Current file is a skeleton with wrong key paths and no quantization. Rewrite it for Qwen3.5 MoE (Ornith 397B).

## Architecture Facts
- Model type: qwen3_5_moe, Qwen3_5MoeForConditionalGeneration
- Layers: 60 text layers
- Experts: 512 routed per layer, top-10 per token
- Hidden size: 4096
- Expert FFN width: 1024
- Shared expert: on ALL layers, width 1024
- Vocab: 248320
- Attention heads: 32, KV heads: 2 (GQA)
- Head dim: 256
- Partial rotary: 0.25
- Layer types: every 4th layer is full_attention (3,7,11,15,19,23,27,31,35,39,43,47,51,55,59), rest are linear_attention
- MTP: 1 layer at index 60 (skip for now)
- Vision: present in config but NOT in text shards (skip)

## Weight Layout
Mixed precision checkpoint:

EXPERT WEIGHTS (BF16 -> int4 quantized):
  Keys: model.language_model.layers.{N}.mlp.experts.{E}.{gate_proj|up_proj|down_proj}.weight
  512 experts x 60 layers x 3 projs = 92160 tensors
  gate_proj [1024,4096], up_proj [1024,4096], down_proj [4096,1024]
  No FP8 scales. Quantized: {name} U8 packed int4 + {name}.qs F32 scale

ATTN WEIGHTS (FP8 -> int8 quantized, full-attn layers only):
  Keys: model.language_model.layers.{N}.self_attn.{q_proj|k_proj|v_proj|o_proj}.weight
  Also: self_attn.q_norm.weight, self_attn.k_norm.weight (F32 1D)
  q_proj [16384,4096], k_proj [512,4096], v_proj [512,4096], o_proj [4096,8192]
  Quantized: {name} U8 int8 + {name}.qs F32 scale

SHARED EXPERT (FP8 -> int8 quantized, ALL layers):
  Keys: model.language_model.layers.{N}.mlp.shared_expert.{gate_proj|up_proj|down_proj}.weight
  gate_proj [1024,4096], up_proj [1024,4096], down_proj [4096,1024]
  Quantized: {name} U8 int8 + {name}.qs F32 scale

MLP GATE/ROUTER (F32, full-attn layers only):
  Keys: model.language_model.layers.{N}.mlp.gate.weight [512,4096]
  NOT quantized

SHARED EXPERT GATE (F32, ALL layers):
  Keys: model.language_model.layers.{N}.mlp.shared_expert_gate.weight [1,4096]
  NOT quantized

NORMS (F32, ALL layers):
  Keys: model.language_model.layers.{N}.input_layernorm.weight, post_attention_layernorm.weight [4096]
  NOT quantized

LINEAR ATTENTION (F32, linear-attn layers only):
  Keys: model.language_model.layers.{N}.linear_attn.{in_proj_a|in_proj_b|in_proj_qkv|in_proj_z|out_proj|norm}.weight
  Also: linear_attn.A_log, linear_attn.dt_bias, linear_attn.conv1d.weight
  NOT quantized (BF16 in source)
  in_proj_qkv [12288,4096], in_proj_z [8192,4096], out_proj [4096,8192]
  conv1d [12288,1,4], norm [128]

TOP-LEVEL:
  model.embed_tokens.weight [248320,4096] -> int8 quantized
  lm_head.weight [248320,4096] -> int8 quantized
  model.norm.weight [4096] -> F32

## KEY NAMING (CRITICAL)
ALL keys use prefix: model.language_model.layers.{N}.
NOT model.layers.{N}. (that is GLM-5.2 naming, wrong for Ornith)

## EXISTING INFRASTRUCTURE (DO NOT REWRITE)
st.h provides: shards struct, st_init, st_find, st_has, st_read_f32, st_read_raw, st_read_slice_f32, st_prefetch
dtype codes: 0=BF16, 1=F16, 2=F32, 3=U8/I8
json.h provides minimal JSON parser

## QUANTIZATION
int8: read U8, multiply each row by scale factor
int4: read U8, unpack 2 nibbles/byte, subtract 8 for signed, multiply by scale
Dequant at runtime during matmul

## LAYER TYPE DETECTION
Read text_config.layer_types from config.json. Array of 60 strings.

## IMPLEMENTATION PLAN

1. Model struct with per-layer fields for both full-attn and linear-attn
2. Config parsing: read all text_config fields including layer_types
3. Tensor loading: handle U8 (dequant), BF16/F16 (convert), F32 (copy), missing (zero-init)
4. Key paths: ALL must use model.language_model.layers.{N}. prefix
5. Forward pass: embed -> layer loop -> final norm -> lm_head -> argmax
6. Full attention: QKV proj, attention, output proj
7. Linear attention: state-space forward pass
8. Shared expert on ALL layers: gate->silu->up->down->residual
9. Expert routing: router->softmax->top-k->weighted expert sum
10. Quantized matmul kernels
11. Error handling: missing tensors = zero-init with warning
12. CLI: --model DIR, --prompt TEXT, --steps N, --threads N

## CONSTRAINTS
- Single file: qwen35_moe.c only
- No external deps: stdlib + json.h + st.h + optional OpenMP
- CPU-only: no CUDA/ROCm/Vulkan
- OpenMP: #pragma omp parallel for where beneficial
- Pre-allocate buffers, no malloc/free in hot loop
- No vision, no MTP, no kv_cache, no RoPE
- Partial model support: zero-init missing tensors

## WHAT NOT TO DO
- Don't add vision support
- Don't implement MTP
- Don't add GPU backends
- Don't change st.h or json.h
- Don't add new files
- Don't use CUDA/ROCm intrinsics
- Don't implement kv_cache or RoPE

## PRIORITY ORDER
1. Fix key naming (language_model. prefix)
2. Add layer type detection
3. Quantized loading (int8 resident, int4 experts)
4. Shared expert on all layers
5. Linear attention
6. Full self-attention
7. Expert routing
8. Forward pass
9. Error handling
10. Optimize (OpenMP)
COPILOT_PROMPT_END
