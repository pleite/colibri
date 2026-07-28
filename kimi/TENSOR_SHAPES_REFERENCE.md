# Kimi K3 Tensor Shape Reference

**Date:** 2026-07-28  
**Model:** moonshotai/Kimi-K3  
**Config source:** https://huggingface.co/moonshotai/Kimi-K3/raw/main/config.json

---

## 1. Model Architecture Parameters

| Parameter | Value | Source |
|---|---|---|
| `hidden_size` | 7,168 | text_config |
| `intermediate_size` | 33,792 | text_config (dense MLP) |
| `routed_expert_hidden_size` | 3,584 | text_config (MoE expert FFN) |
| `moe_intermediate_size` | 3,072 | text_config (MoE expert FFN inner) |
| `num_hidden_layers` | 93 | text_config |
| `num_attention_heads` | 96 | text_config |
| `num_key_value_heads` | 96 | text_config |
| `qk_nope_head_dim` | 128 | text_config |
| `qk_rope_head_dim` | 64 | text_config |
| `v_head_dim` | 128 | text_config |
| `q_lora_rank` | 1,536 | text_config (MLA query compression) |
| `kv_lora_rank` | 512 | text_config (MLA KV compression) |
| `qkv_hidden_size` | 1,536 | vision_config (also used for MLA) |
| `num_experts` | 896 | text_config |
| `num_experts_per_token` | 16 | text_config |
| `num_shared_experts` | 2 | text_config |
| `num_expert_group` | 1 | text_config |
| `vocab_size` | 163,840 | text_config |
| `max_position_embeddings` | 1,048,576 | text_config |

### Vision Config

| Parameter | Value | Source |
|---|---|---|
| `vt_hidden_size` | 1,024 | vision_config |
| `vt_intermediate_size` | 4,096 | vision_config |
| `vt_num_hidden_layers` | 27 | vision_config |
| `vt_num_attention_heads` | 12 | vision_config |
| `mm_hidden_size` | 1,024 | vision_config |
| `text_hidden_size` | 7,168 | vision_config |
| `patch_size` | 14 | vision_config |

### Linear Attention Config

| Parameter | Value | Source |
|---|---|---|
| `head_dim` | 128 | linear_attn_config |
| `num_heads` | 96 | linear_attn_config |
| `full_attn_layers` | [4,8,12,...,92,93] | 24 layers |
| `kda_layers` | [1,2,3,5,...,91] | 69 layers |
| `short_conv_kernel_size` | 4 | linear_attn_config |

---

## 2. Expert FFN Tensor Shapes

### 2.1 Routed Experts (896 experts)

Each expert has 3 FFN projections:

| Projection | Shape [out, inner] | BF16 Size | INT8 Size | INT4 Size |
|---|---|---|---|---|
| `gate_proj` | [3,072, 3,584] | 22.1 MB | 11.1 MB | 5.5 MB |
| `up_proj` | [3,072, 3,584] | 22.1 MB | 11.1 MB | 5.5 MB |
| `down_proj` | [3,584, 3,072] | 22.1 MB | 11.1 MB | 5.5 MB |
| **Per expert total** | | **66.3 MB** | **33.2 MB** | **16.6 MB** |
| **896 experts total** | | **59.4 GB** | **29.8 GB** | **14.9 GB** |

**Tensor name pattern:**
```
model.layers.{layer}.block_sparse_moe.experts.{expert_idx}.{gate_proj|up_proj|down_proj}.weight
```

### 2.2 Shared Experts (2 experts)

Each shared expert has 3 FFN projections (using full hidden_size=7168):

| Projection | Shape [out, inner] | BF16 Size | INT8 Size |
|---|---|---|---|
| `gate_proj` | [3,072, 7,168] | 44.2 MB | 22.1 MB |
| `up_proj` | [3,072, 7,168] | 44.2 MB | 22.1 MB |
| `down_proj` | [7,168, 3,072] | 44.2 MB | 22.1 MB |
| **Per shared expert total** | | **132.6 MB** | **66.3 MB** |
| **2 shared experts total** | | **265.2 MB** | **132.6 MB** |

**Tensor name pattern:**
```
model.layers.{layer}.block_sparse_moe.shared_experts.{gate_proj|up_proj|down_proj}.weight
```

### 2.3 MoE Router

| Projection | Shape [out, inner] | BF16 Size |
|---|---|---|
| `router.gate_proj` | [896, 7,168] | 12.9 MB |

**Tensor name pattern:**
```
model.layers.{layer}.block_sparse_moe.router.gate_proj.weight
```

**Total per layer (router + 2 shared + 896 routed):**
- BF16: 12.9 MB + 265.2 MB + 59.4 GB ≈ **60.0 GB**
- INT8 (routed only): 6.4 MB + 132.6 MB + 29.8 GB ≈ **30.0 GB**

---

## 3. Attention Tensor Shapes

### 3.1 KDA Layers (69 layers)

KDA = Kernelized Delta Attention (linear attention variant)

| Projection | Shape [out, inner] | BF16 Size | INT8 Size |
|---|---|---|---|
| `q_proj` | [7,168, 7,168] | 103.4 MB | 51.7 MB |
| `k_proj` | [512, 7,168] | 7.3 MB | 3.6 MB |
| `v_proj` | [512, 7,168] | 7.3 MB | 3.6 MB |
| `o_proj` | [7,168, 7,168] | 103.4 MB | 51.7 MB |
| `q_a_proj` (MLA compression) | [1,536, 7,168] | 22.1 MB | 11.1 MB |
| `q_b_proj` (MLA output) | [128, 1,536] | 0.4 MB | 0.2 MB |
| `k_a_proj` | [128, 512] | 0.1 MB | 0.05 MB |
| `k_b_proj` (rope) | [64, 128] | 0.02 MB | 0.01 MB |
| **Per KDA layer total** | | **243.8 MB** | **121.9 MB** |
| **69 KDA layers total** | | **16.8 GB** | **8.4 GB** |

### 3.2 Full Attention Layers (24 layers)

Same projections as KDA, but with full softmax attention.

**Per full-attn layer total:** ~243.8 MB (BF16)

**24 full-attn layers total:** ~5.9 GB (BF16)

### 3.3 Attention Norms (all 93 layers)

| Norm | Shape | BF16 Size per layer |
|---|---|---|
| `input_layernorm.weight` | [7,168] | 14.1 KB |
| `post_attention_layernorm.weight` | [7,168] | 14.1 KB |
| `self_attention_res_norm.weight` | [7,168] | 14.1 KB |
| `self_attention_res_proj.weight` | [7,168] | 14.1 KB |
| **Per layer norms total** | | **56.6 KB** |
| **93 layers total** | | **5.3 MB** |

---

## 4. Global Tensor Shapes

| Tensor | Shape [out, inner] | BF16 Size | INT8 Size |
|---|---|---|---|
| `model.embed_tokens.weight` | [163,840, 7,168] | 2.35 GB | 1.17 GB |
| `lm_head.weight` | [163,840, 7,168] | 2.35 GB | 1.17 GB |
| `model.norm.weight` | [7,168] | 14.1 KB | 7.1 KB |

---

## 5. Vision Tower Tensor Shapes

Based on vision_config (27 layers, hidden=1024, heads=12, intermediate=4096):

| Component | Approximate Size (BF16) |
|---|---|
| Patch embedding | ~0.1 GB |
| 27 transformer layers | ~1.0 GB |
| Layer norms | ~0.05 GB |
| **Total vision tower** | **~1.15 GB** |

**Tensor name pattern:**
```
vision_tower.encoder.layers.{layer}.{component}.weight
```

---

## 6. Multimodal Projector Shapes

| Tensor | Shape | BF16 Size |
|---|---|---|
| `mm_projector.proj.weight` | [1024, 1024] | 2.0 MB |
| `mm_projector.proj.bias` | [1024] | 2.0 KB |
| `mm_projector.post_norm.weight` | [1024] | 2.0 KB |

---

## 7. Summary by Component

| Component | BF16 Size | INT8 Size (experts only) | INT4 Size (experts only) |
|---|---|---|---|
| Expert FFN (896 routed) | 59.4 GB | 29.8 GB | 14.9 GB |
| Shared Expert FFN (2) | 265.2 MB | 132.6 MB | — |
| MoE Router | 12.9 MB | — | — |
| Attention (93 layers) | 22.7 GB | — | — |
| Norms (93 layers) | 5.3 MB | — | — |
| Embedding | 2.35 GB | — | — |
| lm_head | 2.35 GB | — | — |
| Vision tower | ~1.15 GB | — | — |
| mm_projector | ~2.0 MB | — | — |
| **TOTAL** | **~88.1 GB** | **~30.0 GB** | **~14.9 GB** |

**Note:** The HuggingFace model is 1.56 TB BF16. The difference (88 GB vs 1,560 GB) is because:
1. The model includes KV cache projections and other auxiliary tensors
2. Some tensors are stored with higher precision than BF16
3. The safetensors index and shard overhead

---

## 8. Mapping to Colibri Shape Set

### 8.1 Existing Colibri Shapes (Qwen 3.5/3.6)

| inner | out | Role |
|---|---|---|
| 4,096 | 1,024 | Expert gate/up |
| 1,024 | 4,096 | Expert down |
| 4,096 | 16,384 | q_proj |
| 4,096 | 512 | k/v_proj, router |
| 8,192 | 4,096 | o_proj |

### 8.2 Kimi K3 Required Shapes (NEW — No Overlap)

| inner | out | Role | NPU Feasible? |
|---|---|---|---|
| 3,072 | 3,584 | Expert down | ✅ Yes |
| 3,584 | 3,072 | Expert gate/up | ✅ Yes |
| 512 | 128 | k_a_proj | ✅ Yes (tiny) |
| 7,168 | 512 | k_proj, v_proj | ✅ Yes |
| 7,168 | 896 | Router gate | ⚠️ Unusual ratio |
| 7,168 | 1,536 | q_a_proj (MLA) | ⚠️ Large |
| 7,168 | 3,072 | Shared gate/up | ⚠️ Large |
| 7,168 | 7,168 | q_proj, o_proj | ⚠️ Large |
| 7,168 | 163,840 | Embedding, lm_head | ❌ No (too large) |
| 128 | 64 | k_b_proj (rope) | ✅ Yes (tiny) |
| 1,536 | 128 | q_b_proj (MLA) | ✅ Yes (small) |

### 8.3 NPU Kernel Requirements

| Category | Projections | Row Tiles | Kernels |
|---|---|---|---|
| Expert FFN | 2 unique (3584,3072) + (3072,3584) | 3 (1, 32, 256) | 6 |
| Attention (small) | 4 unique (512,128), (7168,512), (128,64), (1536,128) | 3 | 12 |
| Attention (large) | 2 unique (7168,7168), (7168,1536) | 3 | 6 |
| Shared FFN | 2 unique (7168,3072), (3072,7168) | 3 | 6 |
| Router | 1 (7168,896) | 3 | 3 |
| **Total NPU kernels** | | | **33** |

**Excluded from NPU:** Embedding, lm_head (too large for NPU heap)

---

## 9. Quantization-Aware Shape Notes

### 9.1 Tensors Requiring Special Handling

| Tensor | Why | Recommended Approach |
|---|---|---|
| Router gate [896, 7168] | Unusual aspect ratio, critical for routing | Keep BF16, or per-channel INT8 with low threshold |
| Shared expert FFN | Serves all tokens, errors accumulate | Keep BF16 (as per current quant config) |
| q_proj [7168, 7168] | Large square matrix, attention sensitive | Keep BF16 |
| o_proj [7168, 7168] | Large square matrix, attention output | Keep BF16 |
| Embedding [163840, 7168] | Vocabulary projection, very large | Keep BF16 |
| lm_head [163840, 7168] | Output projection, very large | Keep BF16 |

### 9.2 Expert Tensors — Quantization Priority

| Priority | Tensor | Reason |
|---|---|---|
| 1 (quantize first) | Expert down_proj | Largest error tolerance |
| 2 | Expert up_proj | Standard FFN |
| 3 (quantize last) | Expert gate_proj | Routing sensitivity |

---

*This document will be updated as tensor shapes are verified from actual safetensors shards.*
