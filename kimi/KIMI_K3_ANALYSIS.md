# Kimi K3 — Non-Safetensors File Analysis & Quantization Strategy

**Date:** 2026-07-28  
**Model:** moonshotai/Kimi-K3 (HuggingFace)  
**Total model size:** 1.56 TB (96 safetensors files)  
**Non-safetensors analysis size:** 62.8 MB (19 files)  
**Status:** Brainstorming / Strategy Phase

---

## 1. Model Architecture Summary

| Parameter | Value |
|---|---|
| Architecture | `KimiK3ForConditionalGeneration` |
| Base dtype | bfloat16 |
| Hidden size | 7,168 |
| Intermediate size (dense MLP) | 33,792 |
| Expert FFN width (routed) | 3,584 |
| MoE intermediate size | 3,072 |
| Number of hidden layers | 93 |
| Attention heads | 96 |
| KV heads | 96 |
| Head dimensions | qk_nope=128, qk_rope=64, v=128 |
| q_lora_rank | 1,536 |
| kv_lora_rank | 512 |
| QKV hidden size | 1,536 |
| Number of experts | 896 |
| Experts per token | 16 |
| Shared experts | 2 |
| Num expert groups | 1 |
| Top-k routing | 16 (topk_method: noaux_tc) |
| Max position embeddings | 1,048,576 |
| Vocabulary size | 163,840 |
| Vision tower layers | 27 |
| Vision hidden size | 1,024 |
| Vision attention heads | 12 |
| Vision intermediate size | 4,096 |
| mm_hidden_size (projector) | 1,024 |
| Text hidden size (projector) | 7,168 |
| Quantization | MXFP4-pack-quantized (group_size=32, symmetric) |
| Quant excluded | self_attn, shared_experts, mlp.gate/up/down_proj, lm_head, vision_tower, mm_projector |

### Architecture Type: Linear Attention MoE

Kimi K3 uses a **hybrid architecture**:
- **Linear attention** for most layers (KDA layers: 1-3, 5-7, 9-11, 13-15, ...)
- **Full attention** for select layers (4, 8, 12, 16, 20, 24, 28, 32, 36, 40, 44, 48, 52, 56, 60, 64, 68, 72, 76, 80, 84, 88, 92, 93)
- **Mixture of Experts** with 896 routed experts + 2 shared experts
- **MLA (Multi-Latent Attention)** with nope and output gate
- **SiLU activation** with situ (scaled activation)

### Layer Distribution

| Layer Type | Count | Layer Indices |
|---|---|---|
| Full attention | 24 | 4,8,12,16,20,24,28,32,36,40,44,48,52,56,60,64,68,72,76,80,84,88,92,93 |
| Linear attention (KDA) | 69 | All others except 0, 93 |
| MoE layers | 93 (all) | Every layer has MoE (moe_layer_freq=1) |

---

## 2. Non-Safetensors File Inventory

### 2.1 Model Code Files (Python)

| File | Size | Purpose |
|---|---|---|
| `modeling_kimi_k3.py` | 53,444 B | Main model class — KimiK3ForConditionalGeneration |
| `modeling_kimi_linear.py` | 51,506 B | Linear attention implementation, KimiLinearModel |
| `encoding_k3.py` | 22,827 B | Token encoding/decoding for K3 tokenizer |
| `media_utils.py` | 13,844 B | Image/media processing utilities |
| `configuration_kimi_k3.py` | 11,343 B | Model configuration class |
| `kimi_k3_processor.py` | 7,660 B | Preprocessing pipeline |
| `kimi_k3_vision_processing.py` | 6,686 B | Vision tower processing |

### 2.2 Configuration Files (JSON)

| File | Size | Purpose |
|---|---|---|
| `config.json` | 7,006 B | Model architecture config |
| `generation_config.json` | 53 B | Generation parameters |
| `tokenizer_config.json` | 3,478 B | Tokenizer configuration |
| `preprocessor_config.json` | 1,011 B | Image preprocessor config |

### 2.3 Tokenizer & Embedding

| File | Size | Purpose |
|---|---|---|
| `tiktoken.model` | 2,795,286 B | Tokenizer vocabulary (tiktoken format) |
| `model.safetensors.index.json` | 59,764,096 B | **Safetensors weight map** (NOT safetensors — this is the index) |

### 2.4 Documentation & Metadata

| File | Size | Purpose |
|---|---|---|
| `README.md` | 45,261 B | Model card with benchmarks |
| `LICENSE` | 3,065 B | kimi-k3 license |
| `.gitattributes` | 1,639 B | Git LFS attributes |
| `.eval_results` | 0 B | Community evaluation results |

### 2.5 Directory

| Path | Purpose |
|---|---|
| `assets/` | Additional assets (likely images or evaluation data) |

---

## 3. Safetensors Tensor Layout Analysis

### 3.1 Tensor Count

| Category | Count |
|---|---|
| **Total tensors** | **497,220** |
| block_sparse_moe (per layer) | 5,384 each × 93 layers = 500,712 (includes some) |
| self_attn (per layer) | 14 each × 69 layers + 8 each × 24 layers = 1,134 |
| norm/layernorm (per layer) | ~6 each × 93 = 558 |
| vision_tower | 163 + 2 = 165 |
| mm_projector | 2 + 1 = 3 |
| language_model | 4 + 1 = 5 |
| **Approximate total** | **~503,577** |

### 3.2 Key Tensor Groups (by role)

| Tensor Pattern | Count | Role |
|---|---|---|
| `layers.{N}.block_sparse_moe.*` | 5,384 per layer | All MoE expert weights (gate, up, down, shared gate, shared up, shared down, router) |
| `layers.{N}.self_attn.*` | 14 per KDA layer, 8 per full-attn | Attention projections (q_proj, k_proj, v_proj, o_proj, q_a, q_b, k_a, k_b, q_nope, q_rope, kv_nope, kv_rope, out_proj, v_proj) |
| `layers.{N}.input_layernorm.*` | 1 per layer | Pre-attention normalization |
| `layers.{N}.post_attention_layernorm.*` | 1 per layer | Post-attention normalization |
| `layers.{N}.mlp_res_norm.*` | 1 per layer | MLP residual norm |
| `layers.{N}.mlp_res_proj.*` | 1 per layer | MLP residual projection |
| `layers.{N}.self_attention_res_norm.*` | 1 per layer | Attention residual norm |
| `layers.{N}.self_attention_res_proj.*` | 1 per layer | Attention residual projection |
| `vision_tower.encoder.*` | 163 | Vision transformer weights |
| `vision_tower.patch_embed.*` | 2 | Vision patch embedding |
| `mm_projector.proj` | 2 | Multimodal projector |
| `mm_projector.post_norm` | 1 | Projector post-norm |
| `language_model.model.*` | 4 | Final embedding/layernorm |
| `language_model.lm_head` | 1 | Output projection (vocab=163,840) |

### 3.3 Non-Safetensors Files — What They Are

The 19 non-safetensors files are:
1. **7 Python model code files** — the architecture implementation
2. **4 JSON config files** — model/tokenizer/generation settings
3. **1 tiktoken model file** — tokenizer vocabulary
4. **1 safetensors index JSON** — maps tensor names to safetensors files (59.7 MB because it stores tensor shapes/dtypes for all 497K+ tensors)
5. **1 README** — model card
6. **1 LICENSE** — license file
7. **1 .gitattributes** — Git LFS config
8. **1 .eval_results** — empty file for community evals
9. **1 assets/ directory** — additional assets

**Key insight:** The `model.safetensors.index.json` at 59.7 MB is NOT a safetensors file itself — it's the weight map that tells the loader which safetensors shard contains which tensor. This is standard for large models split across 96 shards.

---

## 4. Expert Tensor Sizes for Kimi K3

Based on the config and architecture:

### 4.1 Expert FFN Projections (routed experts, 896 total)

| Projection | Shape [out, inner] | Size per expert (BF16) | Size per expert (INT8) | Total (896 experts) BF16 | Total INT8 |
|---|---|---|---|---|---|
| gate_proj | [3,072, 3,584] | 22.1 MB | 11.1 MB | 19.8 GB | 9.9 GB |
| up_proj | [3,072, 3,584] | 22.1 MB | 11.1 MB | 19.8 GB | 9.9 GB |
| down_proj | [3,584, 3,072] | 22.1 MB | 11.1 MB | 19.8 GB | 9.9 GB |
| **Per expert FFN total** | | **66.3 MB** | **33.2 MB** | **59.4 GB** | **29.8 GB** |

### 4.2 Shared Expert FFN Projections (2 total)

| Projection | Shape [out, inner] | Size (BF16) | Size (INT8) |
|---|---|---|---|
| gate_proj | [3,072, 7,168] | 44.2 MB | 22.1 MB |
| up_proj | [3,072, 7,168] | 44.2 MB | 22.1 MB |
| down_proj | [7,168, 3,072] | 44.2 MB | 22.1 MB |
| **Shared FFN total** | | **132.6 MB** | **66.3 MB** |

### 4.3 MoE Router

| Projection | Shape [out, inner] | Size (BF16) | Size (INT8) |
|---|---|---|---|
| router gate | [896, 7,168] | 12.9 MB | 6.4 MB |

### 4.4 Attention Projections (per layer)

For KDA layers (69 layers):

| Projection | Shape [out, inner] | Size (BF16) | Size (INT8) |
|---|---|---|---|
| q_proj (full) | [7,168, 7,168] | 103.4 MB | 51.7 MB |
| k_proj | [512, 7,168] | 7.3 MB | 3.6 MB |
| v_proj | [512, 7,168] | 7.3 MB | 3.6 MB |
| o_proj | [7,168, 7,168] | 103.4 MB | 51.7 MB |
| q_a_proj (MLA) | [1,536, 7,168] | 22.1 MB | 11.1 MB |
| q_b_proj (MLA) | [128, 1,536] | 0.4 MB | 0.2 MB |
| k_a_proj | [128, 512] | 0.1 MB | 0.05 MB |
| k_b_proj (rope) | [64, 128] | 0.02 MB | 0.01 MB |
| v_proj (MLA) | [512, 7,168] | 7.3 MB | 3.6 MB |
| **Per KDA layer attn total** | | **251.1 MB** | **125.5 MB** |

For full-attn layers (24 layers): same projections but potentially different shapes.

### 4.5 Per-Layer Norms (93 layers)

| Norm Type | Shape | Size per layer (BF16) | Size per layer (INT8) |
|---|---|---|---|
| input_layernorm | [7,168] | 14.1 KB | 7.1 KB |
| post_attention_layernorm | [7,168] | 14.1 KB | 7.1 KB |
| mlp_res_norm | [7,168] | 14.1 KB | 7.1 KB |
| self_attention_res_norm | [7,168] | 14.1 KB | 7.1 KB |
| **Per layer norms total** | | **56.6 KB** | **28.3 KB** |

### 4.6 Global Tensors

| Tensor | Shape | Size (BF16) | Size (INT8) |
|---|---|---|---|
| embedding | [163,840, 7,168] | 2.35 GB | 1.17 GB |
| lm_head | [163,840, 7,168] | 2.35 GB | 1.17 GB |
| vision_tower (27 layers) | various | ~1.0 GB | ~0.5 GB |
| mm_projector | various | ~0.1 GB | ~0.05 GB |

---

## 5. Comparison: Kimi K3 vs. Existing Colibri Shape Set

### 5.1 Existing NPU Shape Set (for Qwen 3.5/3.6 MoE)

The colibri repo currently defines these projections for Qwen 3.5/3.6:

| Projection | inner | out | Role |
|---|---|---|---|
| Expert gate/up | 4,096 | 1,024 | expert FFN |
| Expert down | 1,024 | 4,096 | expert FFN |
| q_proj | 4,096 | 16,384 | attention |
| k/v_proj, router | 4,096 | 512 | attention + MoE |
| o_proj | 8,192 | 4,096 | attention |

Crossed with 3 row tiles (1, 32, 256) = **15 kernel artifacts**.

### 5.2 Kimi K3 Required Projections (NEW)

Kimi K3 has a **completely different geometry**. New projections needed:

| Projection | inner | out | Role | Per-layer? |
|---|---|---|---|---|
| **Expert gate_proj** | 3,584 | 3,072 | MoE FFN | 896 experts |
| **Expert up_proj** | 3,584 | 3,072 | MoE FFN | 896 experts |
| **Expert down_proj** | 3,072 | 3,584 | MoE FFN | 896 experts |
| **Shared gate_proj** | 7,168 | 3,072 | Shared FFN | 1 |
| **Shared up_proj** | 7,168 | 3,072 | Shared FFN | 1 |
| **Shared down_proj** | 3,072 | 7,168 | Shared FFN | 1 |
| **Router gate** | 7,168 | 896 | MoE routing | 1 |
| **q_proj** | 7,168 | 7,168 | Attention | 93 layers |
| **k_proj** | 7,168 | 512 | Attention | 93 layers |
| **v_proj** | 7,168 | 512 | Attention | 93 layers |
| **o_proj** | 7,168 | 7,168 | Attention | 93 layers |
| **q_a_proj (MLA)** | 7,168 | 1,536 | MLA compression | 93 layers |
| **q_b_proj (MLA)** | 1,536 | 128 | MLA output | 93 layers |
| **k_a_proj** | 512 | 128 | K compression | 93 layers |
| **k_b_proj (rope)** | 128 | 64 | K rope | 93 layers |
| **Embedding** | 7,168 | 163,840 | Token embedding | 1 |
| **lm_head** | 7,168 | 163,840 | Output head | 1 |

### 5.3 Overlap with Existing Set

| Existing Qwen Projection | Kimi K3 equivalent | Match? |
|---|---|---|
| (4096, 1024) expert gate/up | (3584, 3072) | ❌ Different |
| (1024, 4096) expert down | (3072, 3584) | ❌ Different |
| (4096, 16384) q_proj | (7168, 7168) | ❌ Different |
| (4096, 512) k/v_proj | (7168, 512) | ❌ Different inner |
| (8192, 4096) o_proj | (7168, 7168) | ❌ Different |

**Conclusion: Zero overlap.** Kimi K3 requires a completely new shape set.

---

## 6. Quantization Strategy: INT8 and INT4 for Experts

### 6.1 Current State

Kimi K3 is **already quantized** at the HuggingFace level:
- Format: `mxfp4-pack-quantized` (compressed-tensors)
- Group size: 32, symmetric
- Excluded from quantization: self_attn, shared_experts, mlp.gate/up/down_proj, lm_head, vision_tower, mm_projector

### 6.2 Proposed INT8 Quantization Strategy

#### 6.2.1 Experts (Primary Target)

| Strategy | Details |
|---|---|
| **Per-channel INT8** | Scale per output channel (most common, best accuracy) |
| **Per-tensor INT8** | Single scale per tensor (simpler, slightly worse accuracy) |
| **Per-group INT8** | Group size 128 or 256 (middle ground) |

**Recommended: Per-channel INT8 for all expert weights**

Rationale:
- 896 experts × 3 projections = 2,688 expert weight tensors
- Per-channel scales are cheap to store (one float per output channel)
- INT8 gives <1% perplexity degradation on most MoE models
- Compatible with existing colibri VNNI CPU backend

#### 6.2.2 Non-Expert Weights (Already Quantized or Excluded)

| Component | Current State | Action |
|---|---|---|
| self_attn | Excluded (BF16) | Keep BF16 — attention is sensitive |
| shared_experts | Excluded (BF16) | Keep BF16 — shared experts serve all tokens |
| mlp.gate/up/down_proj | Excluded (BF16) | Keep BF16 — dense MLP, not expert |
| lm_head | Excluded (BF16) | Keep BF16 — vocabulary projection |
| vision_tower | Excluded (BF16) | Keep BF16 — vision is sensitive |
| mm_projector | Excluded (BF16) | Keep BF16 — cross-modality bridge |

### 6.3 Proposed INT4 Quantization Strategy

#### 6.3.1 For Experts

| Strategy | Details |
|---|---|
| **Per-channel INT4** | Scale per output channel, 4-bit weights |
| **Block INT4** | Block size 32 or 64 (matches existing MXFP4) |
| **Per-tensor INT4** | Single scale (simplest, worst accuracy) |

**Recommended: Per-channel INT4 with block size 32**

Rationale:
- Matches existing MXFP4 group size (32)
- 4-bit gives ~40% size reduction vs INT8
- Expected 2-5% perplexity increase vs INT8
- Requires custom kernels (no existing VNNI INT4 support)

#### 6.3.2 Quantization-Aware Considerations

For INT4 experts, the following tensors need special handling:

| Tensor | Why Special |
|---|---|
| gate_proj (expert) | Routing sensitivity — small errors cause expert selection mistakes |
| up_proj / down_proj | Standard FFN — more tolerant of quantization |
| shared expert weights | Serve ALL tokens — quantization errors accumulate |
| router gate | Critical for routing — consider keeping BF16 |

---

## 7. Side Project: High-Precision Correction Tensors

### 7.1 Concept

For tensors where INT8/INT4 quantization exceeds a target error threshold, store **correction data** as additional tensors:

1. **f32 correction vector** — a full-precision residual vector added to the quantized weights
2. **Bit matrix** — a binary mask indicating which weight elements need correction
3. **Nibble correction set** — variable-length int8 nibbles (4-bit) for fine-grained correction to int4 weights

### 7.2 Error Threshold Framework

| Quantization | Target Max Error | Correction Trigger |
|---|---|---|
| INT8 | < 0.5% relative error | Store f32 correction if > threshold |
| INT4 | < 2% relative error | Store nibble correction if > threshold |

### 7.3 Correction Tensor Formats

#### 7.3.1 f32 Correction Vector

```
For each quantization-bad tensor:
  - Store as f32 array of same shape
  - Only non-zero entries (sparse) or full dense
  - Added to dequantized weights during inference
```

Storage overhead: Same as original tensor (if dense) or compressed (if sparse).

#### 7.3.2 Bit Matrix

```
For each tensor:
  - 1 bit per weight element
  - 1 = "this weight needs correction", 0 = "quantized value is fine"
  - Stored as packed uint8 array
```

Storage overhead: `num_elements / 8` bytes.

#### 7.3.3 Nibble Correction Set (for INT4 → INT8 refinement)

```
For each INT4 weight element flagged by bit matrix:
  - Store a signed 4-bit nibble (-8 to +7)
  - Added to the dequantized INT4 value
  - Variable set: only elements flagged by bit matrix get nibbles
```

Storage overhead: `num_flagged_elements` nibbles = `num_flagged_elements / 2` bytes.

### 7.4 Implementation Plan

1. **Phase 1:** Quantize all expert tensors to INT8, measure per-tensor error
2. **Phase 2:** Identify tensors exceeding error threshold
3. **Phase 3:** Generate correction tensors (f32 residual or nibble set)
4. **Phase 4:** Integrate correction into inference pipeline
5. **Phase 5:** Benchmark accuracy vs. size tradeoff

### 7.5 Expected Results

| Component | INT8 Size | +f32 Correction | INT4 Size | +Nibble Correction |
|---|---|---|---|---|
| Expert FFN (896 experts) | 29.8 GB | +~5 GB (sparse) | 14.9 GB | +~2 GB (nibbles) |
| Shared FFN | 66.3 MB | +~10 MB | 33.2 MB | +~5 MB |
| Router | 6.4 MB | +~1 MB | 3.2 MB | +~0.5 MB |
| **Total experts** | **~30.5 GB** | **+~6.6 GB** | **~15.3 GB** | **+~2.5 GB** |

---

## 8. NPU Shape Set Requirements for Kimi K3

### 8.1 New Projections Needed

Based on the analysis, Kimi K3 requires these (inner, out) projections for the NPU:

| # | inner | out | Role | Notes |
|---|---|---|---|---|
| 1 | 3,584 | 3,072 | Expert gate_proj | New |
| 2 | 3,584 | 3,072 | Expert up_proj | Same shape as #1 |
| 3 | 3,072 | 3,584 | Expert down_proj | New |
| 4 | 7,168 | 3,072 | Shared gate_proj | New, large |
| 5 | 7,168 | 3,072 | Shared up_proj | Same shape as #4 |
| 6 | 3,072 | 7,168 | Shared down_proj | New, large |
| 7 | 7,168 | 896 | Router gate | New, unusual ratio |
| 8 | 7,168 | 7,168 | q_proj | New |
| 9 | 7,168 | 512 | k_proj / v_proj | New |
| 10 | 7,168 | 7,168 | o_proj | New |
| 11 | 7,168 | 1,536 | q_a_proj (MLA) | New |
| 12 | 1,536 | 128 | q_b_proj (MLA) | New, small |
| 13 | 512 | 128 | k_a_proj | New, small |
| 14 | 128 | 64 | k_b_proj (rope) | New, tiny |
| 15 | 7,168 | 163,840 | Embedding | New, very large |
| 16 | 7,168 | 163,840 | lm_head | New, very large |

### 8.2 Unique Projections (deduplicated)

| inner | out | Roles |
|---|---|---|
| 3,072 | 3,584 | Expert down_proj |
| 3,584 | 3,072 | Expert gate_proj, up_proj |
| 512 | 128 | k_a_proj |
| 7,168 | 512 | k_proj, v_proj |
| 7,168 | 896 | Router gate |
| 7,168 | 1,536 | q_a_proj (MLA) |
| 7,168 | 3,072 | Shared gate_proj, up_proj |
| 7,168 | 7,168 | q_proj, o_proj |
| 7,168 | 163,840 | Embedding, lm_head |
| 128 | 64 | k_b_proj (rope) |
| 1,536 | 128 | q_b_proj (MLA) |

**11 unique projections** × 3 row tiles = **33 kernel artifacts** needed for NPU.

### 8.3 NPU Feasibility Concerns

| Shape | Rows | Inner | Out | Operand Bytes | NPU Fit? |
|---|---|---|---|---|---|
| Expert FFN | 256 | 3,584 | 3,072 | ~5.6 MB | Likely yes |
| Expert FFN | 32 | 3,584 | 3,072 | ~0.7 MB | Yes |
| Expert down | 256 | 3,072 | 3,584 | ~5.9 MB | Likely yes |
| Shared FFN | 256 | 7,168 | 3,072 | ~10.5 MB | Tight — may exceed heap |
| q_proj | 256 | 7,168 | 7,168 | ~20.5 MB | Likely exceeds heap |
| Embedding | 256 | 7,168 | 163,840 | ~380 MB | **No** — way too large |
| lm_head | 256 | 7,168 | 163,840 | ~380 MB | **No** — way too large |

**Conclusion:** The NPU is suitable for expert FFN and attention projections but **not** for embedding/lm_head (too large) or possibly shared experts (borderline).

---

## 9. CPU (VNNI) Shape Set for Kimi K3

The CPU backend has no shape restrictions — any (rows, inner, out) works. Required shapes:

Same as NPU projections but without row tile constraints:

| inner | out | Min rows (decode) | Max rows (prefill batch) |
|---|---|---|---|
| 3,072 | 3,584 | 1 | 512+ |
| 3,584 | 3,072 | 1 | 512+ |
| 7,168 | 512 | 1 | 512+ |
| 7,168 | 896 | 1 | 512+ |
| 7,168 | 1,536 | 1 | 512+ |
| 7,168 | 3,072 | 1 | 512+ |
| 7,168 | 7,168 | 1 | 512+ |
| 7,168 | 163,840 | 1 | 512+ |
| 512 | 128 | 1 | 512+ |
| 128 | 64 | 1 | 512+ |
| 1,536 | 128 | 1 | 512+ |

---

## 10. GPU (Vulkan/RADV) Shape Set

Same as CPU — Vulkan compute handles any shape. The bottleneck is memory bandwidth and kernel efficiency, not shape constraints.

---

## 11. File One-at-a-Time Quantization Strategy

### 11.1 Approach

Since the model is split across 96 safetensors shards, quantize one file at a time:

1. **Download** one safetensors shard
2. **Read** the weight map index to identify which tensors are in this shard
3. **Quantize** each tensor in the shard (INT8 or INT4 for experts, BF16 for excluded)
4. **Generate** correction tensors if needed
5. **Save** the quantized shard + correction metadata
6. **Update** the index JSON to point to new quantized shards
7. **Repeat** for all 96 shards

### 11.2 Metadata per Shard

Each quantized shard should include:

```json
{
  "shard": "model-00001-of-000096.safetensors",
  "tensors": [
    {
      "name": "model.layers.0.block_sparse_moe.experts.0.gate_proj.weight",
      "shape": [3072, 3584],
      "dtype_original": "float32",
      "dtype_quantized": "int8",
      "scales": [0.01, 0.02, ...],
      "correction": null  // or {"type": "f32_sparse", "indices": [...], "values": [...]}"
    }
  ]
}
```

### 11.3 Implementation Tool

Use `safetensors` Python library:

```python
from safetensors.torch import load_file, save_file
import torch

# Load shard
tensors = load_file("model-00001-of-000096.safetensors")

# Quantize expert tensors
for name, tensor in tensors.items():
    if "block_sparse_moe" in name and "weight" in name:
        # Per-channel INT8 quantization
        scales = tensor.abs().amax(dim=-1, keepdim=True) / 127.0
        quantized = (tensor / scales).round().clamp(-128, 127).to(torch.int8)
        tensors[name] = quantized
        # Store scales separately
```

---

## 12. Open Questions & Next Steps

### 12.1 Questions

1. **Error threshold:** What relative error threshold triggers correction tensors?
2. **Correction storage:** Dense f32 or sparse? Nibble set size for INT4 refinement?
3. **Router quantization:** Keep router gate in BF16 or quantize?
4. **Shared experts:** Keep in BF16 or quantize to INT8?
5. **NPU kernel compilation:** Can we compile kernels for the larger shapes (shared FFN, embedding)?
6. **Batch size:** What decode batch sizes are target? Affects row tile selection.

### 12.2 Next Steps

1. [ ] Write Python script to quantize one safetensors shard and measure error
2. [ ] Determine error distribution across expert tensors
3. [ ] Profile NPU kernel compilation time for new shapes
4. [ ] Design correction tensor format and integration into inference
5. [ ] Benchmark INT8 vs INT4 vs INT4+nibble on actual hardware
6. [ ] Update colibri shape_profile.h and npu_shapes.h with Kimi K3 projections

---

*This document will be updated as the strategy evolves.*
