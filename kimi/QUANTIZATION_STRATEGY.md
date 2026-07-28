# Kimi K3 Quantization Strategy — File-at-a-Time INT8/INT4

**Date:** 2026-07-28  
**Status:** Brainstorming / Strategy Phase  
**Target:** Kimi K3 (1.56 TB, 93 layers, 896 experts, 163K vocab)

---

## 1. Overview

### 1.1 Goal

Quantize Kimi K3 to INT8 and INT4 for expert weights, one safetensors shard at a time, while:
- Keeping non-expert weights (attention, shared experts, lm_head, vision) in BF16
- Measuring per-tensor quantization error
- Generating correction tensors for tensors that exceed error thresholds
- Producing a working quantized model that runs on Strix Halo (CPU VNNI, GPU Vulkan, NPU XDNA2)

### 1.2 Why File-at-a-Time?

- 96 safetensors shards, ~16 GB each
- Cannot load full model into memory (65 GB RAM, model is 1.56 TB BF16)
- Each shard can be processed independently
- Enables parallel processing of shards
- Allows incremental verification

### 1.3 Quantization Targets

| Component | INT8 | INT4 | Correction |
|---|---|---|---|
| Expert gate_proj (896) | ✅ | ✅ | f32 residual if needed |
| Expert up_proj (896) | ✅ | ✅ | f32 residual if needed |
| Expert down_proj (896) | ✅ | ✅ | f32 residual if needed |
| Shared gate_proj (2) | BF16 only | — | — |
| Shared up_proj (2) | BF16 only | — | — |
| Shared down_proj (2) | BF16 only | — | — |
| Router gate (1) | BF16 only | — | — |
| Attention (all layers) | BF16 only | — | — |
| lm_head (1) | BF16 only | — | — |
| Embedding (1) | BF16 only | — | — |
| Vision tower | BF16 only | — | — |
| mm_projector | BF16 only | — | — |

---

## 2. Quantization Methods

### 2.1 INT8 Quantization

#### 2.1.1 Per-Channel INT8 (Recommended)

```
For each weight tensor W [out, inner]:
  1. Compute per-output-channel scale:
     scale[c] = max(|W[c, :]|) / 127.0
  2. Quantize:
     W_int8[c, i] = round(W[c, i] / scale[c])
  3. Clip to [-128, 127]
  4. Store: W_int8 (int8), scales (float32, shape [out])
```

**Storage:** `out * inner` bytes (weights) + `out * 4` bytes (scales)

**Accuracy:** Typically <0.5% perplexity degradation for MoE FFN weights

#### 2.1.2 Per-Tensor INT8 (Alternative)

```
For each weight tensor W:
  1. Compute single scale:
     scale = max(|W|) / 127.0
  2. Quantize all elements with same scale
```

**Storage:** `out * inner` bytes (weights) + 4 bytes (single scale)

**Accuracy:** Typically 1-2% perplexity degradation

### 2.2 INT4 Quantization

#### 2.2.1 Per-Channel INT4 (Recommended)

```
For each weight tensor W [out, inner]:
  1. Compute per-output-channel scale:
     scale[c] = max(|W[c, :]|) / 7.0
  2. Quantize:
     W_int4[c, i] = round(W[c, i] / scale[c])
  3. Clip to [-8, 7]
  4. Pack two int4 values into one byte
  5. Store: W_int4_packed (uint8), scales (float32, shape [out])
```

**Storage:** `out * inner / 2` bytes (packed weights) + `out * 4` bytes (scales)

**Accuracy:** Typically 2-5% perplexity degradation

#### 2.2.2 Block INT4 (Block Size 32)

Matches existing MXFP4 group size:

```
For each weight tensor W:
  1. Reshape to [out, inner/32, 32]
  2. For each block:
     scale = max(|block|) / 7.0
     block_int4 = round(block / scale)
  3. Store: int4 weights + scales per block
```

**Storage:** `out * inner / 2` bytes + `out * (inner/32) * 4` bytes (scales)

---

## 3. Correction Tensor Strategy

### 3.1 Error Measurement

For each quantized tensor, compute:

```
relative_error = ||W_original - W_dequantized||_F / ||W_original||_F
```

Where `||.||_F` is the Frobenius norm.

### 3.2 Error Thresholds

| Quantization | Threshold | Action |
|---|---|---|
| INT8 | < 0.5% | Accept as-is |
| INT8 | 0.5% - 1.0% | Store sparse f32 correction |
| INT8 | > 1.0% | Flag for review, consider per-tensor INT8 instead |
| INT4 | < 2.0% | Accept as-is |
| INT4 | 2.0% - 5.0% | Store nibble correction set |
| INT4 | > 5.0% | Flag for review, consider INT8 instead |

### 3.3 Correction Tensor Formats

#### 3.3.1 Sparse f32 Correction (for INT8)

```python
# Compute residual
residual = W_original - W_dequantized_int8

# Find non-zero entries (above threshold)
threshold = 1e-6
mask = abs(residual) > threshold
indices = where(mask)
values = residual[mask]

# Store as sparse tensor
correction = {
    "type": "f32_sparse",
    "indices": indices,  # [num_nonzero, 2]
    "values": values     # [num_nonzero], float32
}
```

**Storage:** `num_nonzero * (2*4 + 4)` bytes = `12 * num_nonzero` bytes

**Typical sparsity:** 5-15% of elements need correction

#### 3.3.2 Bit Matrix (for INT4)

```python
# Compute per-element error
element_error = abs(W_original - W_dequantized_int4)
threshold = 0.5 * scale  # half a quantization step

# Bit matrix: 1 = needs correction
bit_matrix = (element_error > threshold).astype(uint8)

# Pack bits into bytes
packed = pack_bits(bit_matrix)

correction = {
    "type": "bit_matrix",
    "packed": packed,  # [num_elements / 8] bytes
    "num_elements": out * inner
}
```

**Storage:** `num_elements / 8` bytes

**Typical density:** 20-40% of elements flagged

#### 3.3.3 Nibble Correction Set (for INT4 refinement)

```python
# For each element flagged by bit matrix:
# Compute the correction needed to bring it closer to original
correction_nibbles = round((W_original - W_dequantized_int4) / scale_int4)
correction_nibbles = clip(correction_nibbles, -8, 7)

# Store only for flagged elements
correction = {
    "type": "nibble_set",
    "bit_matrix": packed_bit_matrix,
    "nibbles": correction_nibbles[flagged_elements]  # [num_flagged], int8 (stored as nibbles)
}
```

**Storage:** `num_flagged / 2` bytes (nibbles) + `num_elements / 8` bytes (bit matrix)

**Typical overhead:** ~10-20% of original tensor size

### 3.4 Inference Integration

```python
def dequantize_with_correction(W_int8, scales, correction):
    """Dequantize INT8 weight with optional correction."""
    W = dequantize(W_int8, scales)  # Standard INT8 dequant
    
    if correction is None:
        return W
    
    if correction["type"] == "f32_sparse":
        # Add sparse correction
        W_flat = W.flatten()
        W_flat[correction["indices"]] += correction["values"]
        return W_flat.reshape(W.shape)
    
    return W
```

---

## 4. File-at-a-Time Processing Pipeline

### 4.1 Pipeline Steps

```
For each shard (1 to 96):
  1. Download shard from HuggingFace
  2. Load tensor index to identify tensors in this shard
  3. For each tensor:
     a. Determine quantization target (INT8/INT4/BF16) based on tensor name
     b. Load tensor from shard
     c. Quantize to target format
     d. Measure quantization error
     e. If error exceeds threshold:
        - Generate correction tensor
        - Store correction alongside quantized weight
     f. Save quantized tensor to output shard
  4. Save quantized shard
  5. Update master index JSON
  6. Verify shard integrity
  7. Delete input shard (save disk space)
```

### 4.2 Tensor Classification

Based on tensor name patterns:

```python
def classify_tensor(name):
    """Classify tensor for quantization target."""
    if any(x in name for x in ["block_sparse_moe", "expert"]):
        if "shared" in name:
            return "BF16"  # Keep shared experts in BF16
        return "INT8"  # Default to INT8 for routed experts
    elif any(x in name for x in ["self_attn", "q_proj", "k_proj", "v_proj", "o_proj"]):
        return "BF16"  # Keep attention in BF16
    elif "lm_head" in name:
        return "BF16"
    elif "embed" in name or "embedding" in name:
        return "BF16"
    elif "vision" in name or "mm_projector" in name:
        return "BF16"
    elif "router" in name:
        return "BF16"  # Keep router in BF16
    else:
        return "BF16"  # Default to BF16 for safety
```

### 4.3 Output Structure

```
kimi-k3-quantized/
├── quantized_shards/
│   ├── model-00001-of-000096.safetensors
│   ├── model-00002-of-000096.safetensors
│   └── ...
├── correction_tensors/
│   ├── model-00001-of-000096.corrections.json
│   ├── model-00002-of-000096.corrections.json
│   └── ...
├── quantized_index.json
├── config.json (unchanged)
├── tokenizer files (unchanged)
└── README.md
```

### 4.4 Quantized Index Format

```json
{
  "metadata": {
    "quantization": "INT8",
    "correction_strategy": "sparse_f32",
    "error_threshold": 0.005,
    "total_shards": 96
  },
  "weight_map": {
    "model.layers.0.block_sparse_moe.experts.0.gate_proj.weight": "model-00001-of-000096.safetensors",
    ...
  },
  "tensors": {
    "model.layers.0.block_sparse_moe.experts.0.gate_proj.weight": {
      "shape": [3072, 3584],
      "dtype_original": "float32",
      "dtype_quantized": "int8",
      "scales_file": "model-00001-of-000096.scales.bin",
      "correction_file": "model-00001-of-000096.corrections.json"
    },
    ...
  }
}
```

---

## 5. Parallel Processing

### 5.1 Shard Parallelism

- 96 shards can be processed in parallel
- Each shard is ~16 GB, fits in memory with INT8 (50% reduction)
- Use multiprocessing or distributed processing

### 5.2 Estimated Processing Time

| Metric | Value |
|---|---|
| Shards | 96 |
| Tensors per shard (avg) | ~5,200 |
| Quantization time per tensor (INT8) | ~10 ms |
| Total quantization time | ~52 minutes |
| Correction generation time | ~26 minutes |
| **Total estimated time** | **~1.5 hours** |

### 5.3 Memory Requirements

| Stage | Memory |
|---|---|
| Load shard (BF16) | ~16 GB |
| Quantize to INT8 | ~8 GB |
| Store correction | ~1-2 GB |
| **Peak per shard** | **~25 GB** |
| **With 2 parallel shards** | **~50 GB** |

Fits within 65 GB RAM with 1-2 parallel shards.

---

## 6. Verification & Testing

### 6.1 Per-Shard Verification

After quantizing each shard:
1. Load quantized shard
2. Dequantize a sample of tensors
3. Compare with original (if available) or check for NaN/Inf
4. Verify tensor shapes and dtypes match index

### 6.2 End-to-End Testing

1. Load quantized model
2. Run inference on sample inputs
3. Compare outputs with BF16 baseline
4. Measure perplexity degradation
5. Verify correction tensors are applied correctly

### 6.3 Performance Benchmarking

| Benchmark | Metric | Target |
|---|---|---|
| INT8 inference speed | tok/s | >90% of BF16 |
| INT4 inference speed | tok/s | >95% of INT8 |
| INT4+nibble speed | tok/s | >85% of INT8 |
| Memory usage | GB | <50% of BF16 |
| Perplexity degradation | % | <2% (INT8), <5% (INT4) |

---

## 7. Risks & Mitigations

| Risk | Impact | Mitigation |
|---|---|---|
| Quantization error too high | Model quality degradation | Use correction tensors, fall back to BF16 for sensitive tensors |
| NPU kernel compilation failure | Cannot use NPU for some shapes | Fall back to CPU/GPU for unsupported shapes |
| Memory overflow | Processing fails | Process 1 shard at a time, use sparse corrections |
| Correction tensor size too large | Storage overhead | Use sparser thresholds, compress corrections |
| Inference compatibility | Model doesn't load | Use standard safetensors format, document custom loader |

---

## 8. Next Actions

1. **Immediate:** Write Python quantization script for one shard
2. **Short-term:** Test on shard 1, measure error distribution
3. **Medium-term:** Scale to all 96 shards with parallel processing
4. **Long-term:** Integrate correction tensors into colibri inference pipeline

---

*This document will be updated as the strategy evolves.*
