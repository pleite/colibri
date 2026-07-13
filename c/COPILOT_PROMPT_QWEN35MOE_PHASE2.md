COPILOT_PROMPT_START
# Phase 2: Add Missing Qwen3.5 MoE Features to qwen35_moe.c

## WHAT WAS ALREADY DONE (Phase 1)
Phase 1 (commit a3d17ae) added:
- find_tensor() bridging model.layers. and model.language_model.layers. prefixes
- Quantized tensor loading (int8 U8 + F32 .qs scales, int4 packed U8 + F32 .qs scales)
- Missing tensor handling (zero-init, don't crash)
- --threads CLI flag

## WHAT IS MISSING (This Phase)

The engine currently treats ALL 60 layers identically. Qwen3.5 MoE has two layer types.
You must add: layer type detection, linear attention, shared expert, GQA norms, correct
expert FFN dimensions, and proper top-k routing.

## ARCHITECTURE FACTS (Ornith 397B)

- text_config.num_hidden_layers: 60
- text_config.num_experts: 512
- text_config.num_experts_per_tok: 10
- text_config.moe_intermediate_size: 1024 (expert FFN hidden width)
- text_config.shared_expert_intermediate_size: 1024
- text_config.num_attention_heads: 32
- text_config.num_key_value_heads: 2 (GQA)
- text_config.head_dim: 256
- text_config.partial_rotary_factor: 0.25
- text_config.layer_types: array of 60 strings, "full_attention" or "linear_attention"
  Full-attention layers: indices 3,7,11,15,19,23,27,31,35,39,43,47,51,55,59 (15 layers)
  Linear-attention layers: remaining 45 layers
- text_config.mtp_num_hidden_layers: 1 (layer at index 60, skip for now)
- text_config.linear_conv_kernel_dim: 4
- text_config.linear_key_head_dim: 128
- text_config.linear_num_key_heads: 16
- text_config.linear_num_value_heads: 64
- text_config.linear_value_head_dim: 128

## EXACT KEY PATHS TO LOAD

### Norms (F32, ALL layers)
- model.language_model.layers.{N}.input_layernorm.weight [4096]
- model.language_model.layers.{N}.post_attention_layernorm.weight [4096]

### Full-attention layer only:

#### GQA norms (F32)
- model.language_model.layers.{N}.self_attn.q_norm.weight [256]
- model.language_model.layers.{N}.self_attn.k_norm.weight [256]

#### Attention projections (int8 quantized)
- model.language_model.layers.{N}.self_attn.q_proj.weight [16384, 4096] (= num_heads * head_dim x hidden)
- model.language_model.layers.{N}.self_attn.k_proj.weight [512, 4096] (= num_kv_heads * head_dim x hidden)
- model.language_model.layers.{N}.self_attn.v_proj.weight [512, 4096] (= num_kv_heads * head_dim x hidden)
- model.language_model.layers.{N}.self_attn.o_proj.weight [4096, 8192] (= hidden x num_heads * head_dim)

#### Router (F32)
- model.language_model.layers.{N}.mlp.gate.weight [512, 4096] (= num_experts x hidden)
  This IS the router, NOT a dense gate_proj. It produces expert selection logits.

#### Shared expert (int8 quantized, ALL layers)
- model.language_model.layers.{N}.mlp.shared_expert.gate_proj.weight [1024, 4096]
- model.language_model.layers.{N}.mlp.shared_expert.up_proj.weight [1024, 4096]
- model.language_model.layers.{N}.mlp.shared_expert.down_proj.weight [4096, 1024]
- model.language_model.layers.{N}.mlp.shared_expert_gate.weight [1, 4096] (F32, gating scalar)

#### Dense MLP (int8 quantized, full-attn layers only)
- model.language_model.layers.{N}.mlp.gate_proj.weight [1024, 4096]
- model.language_model.layers.{N}.mlp.up_proj.weight [1024, 4096]
- model.language_model.layers.{N}.mlp.down_proj.weight [4096, 1024]

#### Experts (int4 quantized, full-attn layers only)
- model.language_model.layers.{N}.mlp.experts.{E}.gate_proj.weight [1024, 4096] for E in 0..511
- model.language_model.layers.{N}.mlp.experts.{E}.up_proj.weight [1024, 4096]
- model.language_model.layers.{N}.mlp.experts.{E}.down_proj.weight [4096, 1024]

### Linear-attention layer only:

#### Linear attention projections (F32)
- model.language_model.layers.{N}.linear_attn.in_proj_a.weight [128, 4096]
- model.language_model.layers.{N}.linear_attn.in_proj_b.weight [128, 4096]
- model.language_model.layers.{N}.linear_attn.in_proj_qkv.weight [12288, 4096]
- model.language_model.layers.{N}.linear_attn.in_proj_z.weight [8192, 4096]
- model.language_model.layers.{N}.linear_attn.out_proj.weight [4096, 8192]
- model.language_model.layers.{N}.linear_attn.norm.weight [128]
- model.language_model.layers.{N}.linear_attn.A_log.weight [128]
- model.language_model.layers.{N}.linear_attn.dt_bias.weight [128]
- model.language_model.layers.{N}.linear_attn.conv1d.weight [12288, 1, 4]

### Top-level (already loaded in Phase 1)
- model.embed_tokens.weight [248320, 4096] -> int8
- lm_head.weight [248320, 4096] -> int8
- model.norm.weight [4096] -> F32

## WHAT TO CHANGE IN qwen35_moe.c

### 1. Model struct (lines 38-62): REPLACE the flat arrays with per-layer struct

Current flat arrays (lines 50-62) must become a single layers array:

typedef struct {
    // Norms (F32, ALL layers)
    float *in_ln;
    float *post_ln;

    // Full-attention only fields (NULL on linear-attn layers)
    bool is_full_attn;
    float *q_norm, *k_norm;              // F32 [head_dim]
    float *q_proj, *k_proj, *v_proj, *o_proj;  // F32 (dequantized from int8)
    float *router;                        // F32 [num_experts, hidden]
    float *sh_gate, *sh_up, *sh_down;    // F32 (dequantized from int8)
    float *mlp_gate_proj, *mlp_up_proj, *mlp_down_proj;  // F32 (dequantized from int8)
    float ***expert_gate_proj;           // [num_experts] x [intermediate, hidden]
    float ***expert_up_proj;
    float ***expert_down_proj;

    // Linear-attention only fields (NULL on full-attn layers)
    float *la_in_proj_a, *la_in_proj_b;           // F32 [128, hidden]
    float *la_in_proj_qkv, *la_in_proj_z;          // F32
    float *la_out_proj;                            // F32 [hidden, 2*intermediate]
    float *la_norm;                                // F32 [128]
    float *la_A_log, *la_dt_bias;                  // F32 [128]
    float *la_conv1d;                              // F32 [12288, 1, 4]
} QLayer;

Add to model struct:
- QLayer *layers (replaces all the flat float** arrays)
- int moe_intermediate_size
- int shared_expert_intermediate_size
- int num_kv_heads
- int head_dim
- int *layer_types (int array: 0=linear, 1=full)

### 2. Config parsing (around line 239): ADD new fields

Read from config.json text_config:
- moe_intermediate_size (default: hidden_size)
- shared_expert_intermediate_size (default: moe_intermediate_size)
- num_key_value_heads (default: num_attention_heads)
- head_dim (default: hidden_size / num_attention_heads)
- layer_types: parse the array of strings into int array (0=linear, 1=full)

### 3. init_model (lines 228-380): COMPLETE REWRITE

Replace the flat-array loading loop with per-layer type-aware loading:

For each layer N:
a. Allocate QLayer, zero-init
b. Load in_ln and post_ln (F32, both layer types)
c. Determine is_full_attn from layer_types[N]
d. If full_attn:
   - Load q_norm, k_norm (F32 [head_dim])
   - Load q_proj [num_heads*head_dim, hidden], k_proj [num_kv_heads*head_dim, hidden],
     v_proj [num_kv_heads*head_dim, hidden], o_proj [hidden, num_heads*head_dim] (int8)
   - Load router [num_experts, hidden] (F32)
   - Load mlp.gate_proj, mlp.up_proj, mlp.down_proj (int8, [intermediate, hidden])
   - Load shared_expert.gate_proj, shared_expert.up_proj, shared_expert.down_proj (int8)
   - Load shared_expert_gate (F32 [1, hidden])
   - Allocate and load expert projs for all 512 experts (int4)
e. If linear_attn:
   - Load linear_attn.* tensors (F32)

### 4. free_model (lines 382-419): UPDATE for new struct

Free QLayer fields instead of flat arrays.

### 5. run_model forward pass (lines 500-595): ADD linear attention and shared expert

Current forward pass only does self-attention + dense MLP + expert routing.
Add:
a. After self-attention residual add (line 528): If is_full_attn, skip. If linear_attn,
   run linear attention forward pass and add residual.
b. After residual add: Run shared expert on ALL layers:
   - gate_out = silu(shared_expert_gate * shared_expert_gate_proj @ x)
   - up_out = silu(shared_expert_up_proj @ x)
   - shared_out = shared_expert_down_proj @ (gate_out * up_out)
   - hidden += shared_out
c. If full_attn: existing expert routing code (lines 541-568) but fix:
   - Expert FFN uses moe_intermediate_size, NOT hidden_size
   - Use top-k selection instead of all-experts
   - gate_proj/up_proj/down_proj are the DENSE MLP, not the router

### 6. Linear attention forward pass (NEW FUNCTION)

Implement a simplified linear attention / state-space model:
1. qkv = in_proj_qkv @ x -> split into q, k, v
2. z = in_proj_z @ x
3. k = in_proj_b @ k (project to key space)
4. Apply conv1d to k (depthwise 1D conv, kernel_dim=4)
5. Apply A_log and dt_bias for exponential decay
6. State accumulation: S += k^T * v (outer product accumulated)
7. Output: y = S @ v + (q * z) with proper scaling
8. out = out_proj @ y

This is an approximation. The exact Qwen3.5 linear attention uses a more complex
state-space formulation. For now, implement the basic SSM pattern.

### 7. Top-k expert selection (MODIFY existing routing)

Current code (lines 541-568) processes ALL experts. Change to:
- Compute router logits
- Select top-k (k = experts_per_tok, default 10) using partial sort
- Only compute FFN for selected experts
- Weight by router softmax probabilities

### 8. Expert FFN dimensions (FIX)

Current code uses hidden_size for expert FFN dimensions (lines 354, 361, 368).
Correct: use moe_intermediate_size for gate_proj/up_proj (intermediate x hidden)
and hidden_size for down_proj (hidden x intermediate).

## EXISTING INFRASTRUCTURE (DO NOT TOUCH)
- find_tensor() at line 107: bridges both key prefixes
- load_tensor_f32() at line 141: handles U8/int8/int4 dequantization
- st.h: shards, st_read_f32, st_read_raw, etc.
- json.h: minimal JSON parser

## CONSTRAINTS
- Single file: qwen35_moe.c
- No new .h files
- CPU-only, OpenMP where beneficial
- Pre-allocate all buffers
- Missing tensors: zero-init with warning (don't crash)
- No vision, no MTP, no kv_cache, no RoPE

## PRIORITY ORDER
1. Model struct: replace flat arrays with QLayer
2. Config: read moe_intermediate_size, shared_expert_intermediate_size, num_kv_heads, head_dim, layer_types
3. init_model: per-layer type-aware loading with correct key paths and dimensions
4. free_model: update for new struct
5. Expert FFN dimensions: fix to use moe_intermediate_size
6. Top-k routing: partial sort, only compute selected experts
7. Shared expert: load and apply on all layers
8. Linear attention: load tensors and implement forward pass
9. Forward pass: integrate all components correctly
10. Cleanup: remove dead code, fix warnings
COPILOT_PROMPT_END
