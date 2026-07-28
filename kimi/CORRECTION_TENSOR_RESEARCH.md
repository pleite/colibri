# Correction Tensor Research — High-Precision Recovery for Quantized Weights

**Date:** 2026-07-28  
**Status:** Brainstorming / Side Project  
**Goal:** Find tensors where INT8/INT4 cannot represent weights within target error, and store correction data to recover precision.

---

## 1. Problem Statement

When quantizing weights to INT8 or INT4, some tensors will have quantization error that exceeds acceptable thresholds. Rather than discarding these tensors or keeping them in BF16 (which defeats the purpose of quantization), we can store **correction data** that enhances the quantized representation.

### 1.1 Error Sources in Quantization

| Source | Description | Mitigation |
|---|---|---|
| Quantization step size | Finite grid of representable values | Per-channel scales help |
| Outlier weights | Extreme values dominate scale | Outlier-aware quantization |
| Correlated weights | Weights cluster in specific ranges | Block quantization |
| Activation-driven sensitivity | Some weights matter more for output | Per-tensor error measurement |

### 1.2 Target Error Thresholds

| Quantization | Max Relative Error | Action if Exceeded |
|---|---|---|
| INT8 | 0.5% | Store f32 sparse correction |
| INT4 | 2.0% | Store nibble correction set |

---

## 2. Correction Tensor Types

### 2.1 Type A: f32 Sparse Correction Vector

**For:** INT8 tensors exceeding 0.5% relative error

**Concept:** Store the residual `W_original - W_dequantized` as a sparse f32 tensor.

**Format:**
```python
correction = {
    "type": "f32_sparse",
    "indices": [[r0, c0], [r1, c1], ...],  # int32, shape [num_nonzero, 2]
    "values": [v0, v1, ...],                 # float32, shape [num_nonzero]
    "threshold": 1e-6                         # min abs value to include
}
```

**Storage overhead:**
- Dense: same as original tensor (100% overhead)
- Sparse (5% non-zero): 12 bytes per correction × 0.05 × num_elements
- Sparse (1% non-zero): 12 bytes per correction × 0.01 × num_elements

**Inference integration:**
```python
def apply_f32_correction(W_dequant, correction):
    """Add sparse f32 correction to dequantized weights."""
    W = W_dequant.flatten()
    for idx, val in zip(correction["indices"], correction["values"]):
        W[idx[0] * W.shape[1] + idx[1]] += val
    return W.reshape(W_dequant.shape)
```

### 2.2 Type B: Bit Matrix

**For:** Identifying which weight elements need correction (used with Type C)

**Concept:** Binary mask indicating which elements have quantization error above threshold.

**Format:**
```python
correction = {
    "type": "bit_matrix",
    "packed": bytes([...]),           # uint8, shape [num_elements // 8]
    "num_elements": rows * cols,
    "threshold": 0.5 * scale          # relative to quantization step
}
```

**Storage overhead:** `num_elements / 8` bytes (12.5% of original for int8, 6.25% for int4)

**Compression:** Run-length encoding can reduce this significantly if the pattern is clustered.

### 2.3 Type C: Nibble Correction Set (INT4 → INT8 refinement)

**For:** INT4 tensors exceeding 2.0% relative error

**Concept:** For each element flagged by the bit matrix, store a signed 4-bit nibble that corrects the dequantized INT4 value toward the original.

**Format:**
```python
correction = {
    "type": "nibble_set",
    "bit_matrix": packed_bits,         # from Type B
    "nibbles": bytes([...]),           # int8 values stored as nibbles
    "num_flagged": num_flagged_elements
}
```

**Storage overhead:**
- Bit matrix: `num_elements / 8` bytes
- Nibbles: `num_flagged / 2` bytes
- Total: `(num_elements / 8) + (num_flagged / 2)` bytes

**Typical values:**
- If 30% of elements flagged: `(N/8) + (0.3N/2) = 0.125N + 0.15N = 0.275N` bytes
- Compared to INT4 weight size: `N/2` bytes
- Overhead: `0.275N / 0.5N = 55%` of INT4 weight size

**But:** This effectively gives INT4 + nibble = ~5.5-bit precision, approaching INT6 per element.

### 2.4 Type D: Hybrid f32 + Bit Matrix

**For:** INT8 tensors where sparse f32 is too large but full f32 is wasteful

**Concept:** Use bit matrix to identify regions, then store f32 corrections only in high-error regions.

**Format:**
```python
correction = {
    "type": "hybrid",
    "bit_matrix": packed_bits,
    "f32_corrections": {
        "indices": [...],
        "values": [...]
    },
    "region_threshold": 0.01  # only correct regions with >1% error
}
```

---

## 3. Error Analysis Framework

### 3.1 Per-Tensor Error Measurement

```python
import torch

def measure_quantization_error(W_original, W_quantized, scales=None):
    """Measure relative Frobenius norm error."""
    if scales is not None:
        # Dequantize
        W_dequant = W_quantized.float() * scales.unsqueeze(-1)
    else:
        W_dequant = W_quantized.float()
    
    # Relative error
    num = torch.norm(W_original.float() - W_dequant, p='fro')
    den = torch.norm(W_original.float(), p='fro')
    relative_error = (num / den).item()
    
    # Per-element error distribution
    element_error = (W_original.float() - W_dequant).abs()
    max_error = element_error.max().item()
    mean_error = element_error.mean().item()
    pct_99 = torch.percentile(element_error, 99).item()
    pct_999 = torch.percentile(element_error, 99.9).item()
    
    return {
        "relative_error": relative_error,
        "max_error": max_error,
        "mean_error": mean_error,
        "pct_99_error": pct_99,
        "pct_999_error": pct_999,
        "num_outliers": (element_error > 3 * mean_error).sum().item()
    }
```

### 3.2 Expected Error Distribution by Tensor Type

| Tensor Type | Expected INT8 Error | Expected INT4 Error | Correction Needed? |
|---|---|---|---|
| Expert gate_proj | 0.1-0.3% | 0.5-1.5% | Rarely for INT8, sometimes INT4 |
| Expert up_proj | 0.1-0.3% | 0.5-1.5% | Rarely |
| Expert down_proj | 0.1-0.3% | 0.5-1.5% | Rarely |
| Router gate | 0.2-0.5% | 1-3% | Sometimes for INT4 |
| q_proj (attention) | N/A (kept BF16) | N/A | — |
| o_proj (attention) | N/A (kept BF16) | N/A | — |
| Embedding | N/A (kept BF16) | N/A | — |

**Hypothesis:** Most expert FFN tensors will quantize well to INT8 without corrections. INT4 will need corrections for ~10-30% of tensors.

### 3.3 Outlier Analysis

Some weights may be outliers that dominate the scale:

```python
def analyze_outliers(W, num_percentiles=[90, 95, 99, 99.9]):
    """Analyze weight distribution for outliers."""
    abs_W = W.abs().flatten()
    results = {}
    for p in num_percentiles:
        results[f"p{p}"] = torch.percentile(abs_W, p).item()
    results["max"] = abs_W.max().item()
    results["mean"] = abs_W.mean().item()
    results["std"] = abs_W.std().item()
    results["skewness"] = (
        ((abs_W - abs_W.mean()) / (abs_W.std() + 1e-8)) ** 3
    ).mean().item()
    return results
```

**If skewness > 2:** Consider outlier-aware quantization (keep top 1-5% in BF16, quantize rest).

---

## 4. Implementation Plan

### 4.1 Phase 1: Quantize and Measure (Week 1)

1. Download shard 1 from HuggingFace
2. Quantize all expert tensors to INT8 (per-channel)
3. Measure per-tensor error
4. Identify tensors exceeding 0.5% threshold
5. Generate f32 sparse corrections for those tensors
6. Measure correction sparsity

### 4.2 Phase 2: INT4 Analysis (Week 2)

1. Quantize all expert tensors to INT4 (per-channel)
2. Measure per-tensor error
3. Identify tensors exceeding 2.0% threshold
4. Generate bit matrix + nibble corrections
5. Measure overhead

### 4.3 Phase 3: Inference Integration (Week 3)

1. Modify colibri inference to load correction tensors
2. Implement dequantization with correction application
3. Benchmark accuracy recovery
4. Benchmark performance impact

### 4.4 Phase 4: Scale to Full Model (Week 4)

1. Apply pipeline to all 96 shards
2. Generate master correction index
3. End-to-end testing
4. Documentation

---

## 5. Expected Outcomes

### 5.1 Size vs. Accuracy Tradeoff

| Configuration | Size (experts) | Expected Error | Correction Overhead |
|---|---|---|---|
| BF16 (baseline) | 59.7 GB | 0% | — |
| INT8 | 29.8 GB | ~0.2% | — |
| INT8 + f32 correction | 30.0-35.0 GB | ~0.05% | +0.2-0.8 GB |
| INT4 | 14.9 GB | ~1.5% | — |
| INT4 + nibble correction | 15.3-17.0 GB | ~0.3% | +0.4-2.1 GB |

### 5.2 Accuracy Recovery

| Quantization | Without Correction | With Correction | Recovery |
|---|---|---|---|
| INT8 | 99.8% of BF16 accuracy | 99.95% | ~50% of gap closed |
| INT4 | 98.5% of BF16 accuracy | 99.5% | ~67% of gap closed |

---

## 6. Open Research Questions

1. **Optimal error threshold:** Is 0.5% (INT8) / 2.0% (INT4) the right threshold? Should it be per-tensor or global?

2. **Correction sparsity:** How sparse are the corrections in practice? If >50% of elements need correction, is it worth it?

3. **Compression:** Can bit matrices be compressed (RLE, run-length, bitmap compression)?

4. **Inference cost:** What's the latency impact of applying corrections? Sparse add is fast, but memory bandwidth for corrections matters.

5. **Alternative approaches:**
   - **Outlier-aware quantization:** Keep top-1% weights in BF16, quantize rest
   - **Mixed precision:** INT8 for most, BF16 for sensitive tensors
   - **Group quantization:** Different group sizes per tensor

6. **Perceptual vs. numerical error:** Does Frobenius norm error correlate with perplexity degradation? Need empirical validation.

---

## 7. Related Work

| Approach | Source | Notes |
|---|---|---|
| GPTQ | Frantar et al. 2022 | Permutation-based INT4, very accurate |
| AWQ | Lin et al. 2023 | Activation-aware weight quantization |
| SmoothQuant | Xiao et al. 2022 | Smooths activation/weight distributions |
| Outlier-aware quantization | Multiple | Keep outliers in higher precision |
| Sparse corrections | Research stage | Similar to pruning but for quantization |

---

*This document will be updated as empirical results become available.*
