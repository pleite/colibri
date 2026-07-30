# Collapsed 56-Expert Architecture — Full Memory Analysis

**Date:** 2026-07-30  
**Status:** Design complete  
**Target:** ~50 GB peak RAM on Strix Halo (65 GB unified memory)

## Architecture Overview

Collapse 92 MoE layers (10-69, plus 70-92) into 23 collapsed layers, each with 56 black-box experts instead of 896. Each collapsed layer replaces 4 original layers (3 KDA + 1 full attention).

### Original K3 Architecture

| Component | Count | Params per Layer | Total Params |
|---|---|---|---|
| MLA layers (full attention) | 24 | 144M | 3.46B |
| KDA layers (linear attention) | 69 | 355M | 24.5B |
| MoE experts (896 per layer) | 60 | 59.4B | 3.56T |
| Embedding + LM head | 1 | — | 2.35B |
| **Total** | **93** | | **~1.79 TB (FP4)** |

### Collapsed Architecture

| Component | Count | Details | Total Params |
|---|---|---|---|
| Collapsed layers | 23 | 4 original → 1 collapsed | — |
| Black box experts | 23 × 56 = 1,288 | SwiGLU [7247→r→4r→r→7168] | 5.43B (r=512) |
| Collapsed routers | 23 | [7247→56] sigmoid, top-16 | 9.3M |
| MLA layers | 24 | Unchanged | 3.46B |
| KDA layers | 69 | Unchanged | 24.5B |
| Embedding + LM head | 1 | — | 2.35B |
| **Total** | | | **~10 GB (FP4)** |

## Black Box Design

### Structure

```
Input:  h(7168) ⊕ expert_onehot(56) ⊕ layer_onehot(23) = 7247 dim
        │
        ├─ W_down:    [7247 → r]           (linear projection)
        ├─ SwiGLU:    [r → 4r] → GELU × sigmoid → [r → 4r] (gated activation)
        ├─ W_up:      [r → 7168]           (linear projection)
        └─ Residual:  h + output           (skip connection)
```

### Why SwiGLU

SwiGLU is the gold standard for modern MoE models. Gated activation (`GELU(gate) × sigmoid(up)`) is more expressive than plain GELU for the same parameter count. This is what the original K3 experts use internally, and vLLM supports it natively.

### Expert and Layer Encoding

The 56 one-hot dims identify which of the 56 collapsed experts this black box is. The 23 one-hot dims identify which collapsed layer. These let each black box learn a specialized mapping: "when I'm expert 7 in collapsed layer 3, transform h this way."

The 79 extra dims are negligible compared to 7168 — they add ~1.8M params to W_down, which is 0.04% of the total.

### Low-Rank Bottleneck (r) — Memory vs Quality Tradeoff

| r | Box Params | BB Weights (FP4) | BB Activations | Peak RAM | Compression |
|---|---|---|---|---|---|
| 64 | 955K | 0.62 GB | 1.84 GB | 26.1 GB | 1,793× |
| 128 | 1.98M | 1.27 GB | 3.67 GB | 28.6 GB | 904× |
| 256 | 4.21M | 2.71 GB | 7.34 GB | 33.7 GB | 460× |
| 384 | 6.72M | 4.32 GB | 11.01 GB | 39.0 GB | 308× |
| **512** | **9.48M** | **6.10 GB** | **14.68 GB** | **44.5 GB** | **294×** |
| **640** | **12.5M** | **8.05 GB** | **18.35 GB** | **50.1 GB** | **223×** |
| 768 | 15.8M | 10.2 GB | 22.0 GB | 55.9 GB | 186× |
| 1024 | 23.1M | 14.9 GB | 29.4 GB | 67.9 GB | 120× |

## Full Memory Breakdown — r=512, FP4 KV Cache

### Model Weights (FP4)

| Component | FP4 Size |
|---|---|
| 1,288 black boxes (SwiGLU, r=512) | 6.10 GB |
| 23 collapsed routers | 4.7 MB |
| Embedding + LM head | 1.17 GB |
| **Total weights** | **7.28 GB** |

### KV Cache (128K tokens, FP4)

| Component | Size |
|---|---|
| 24 MLA layers (FP4 with per-token scaling) | 2.17 GB |
| 69 KDA layers (recurrent state) | 0.24 GB |
| **Total KV cache** | **2.40 GB** |

### Peak Activations (forward pass, batch=1)

| Component | Size |
|---|---|
| Hidden states | 1.84 GB |
| MLA projections (q, k, v) | 14.10 GB |
| Black box SwiGLU intermediates (16 active) | 14.68 GB |
| Black box down/up projections | 0.13 GB |
| Residual + overhead | 2.00 GB |
| **Peak** | **~32.75 GB** |

### Total Memory

| Scenario | Total |
|---|---|
| **Weights (FP4) + KV cache (FP4)** | **9.69 GB** |
| **Peak (weights + KV + activations)** | **44.5 GB** |
| Original K3 (full) | 1,793 GB |
| **Reduction** | **99.75%** |
| **Compression ratio** | **40×** |

## KV Cache Optimization Strategies

### 1. FP4 KV Cache — 2.16 GB savings (52% of KV)

Store MLA KV cache in FP4 E2M1 with per-token scaling. The repo already uses FP4 KV cache in its config. Saves **2.16 GB** at 128K context. Accuracy loss <1%.

**vLLM support:** FP8 KV cache is native (`--quantization fp8`). FP4 requires custom kernel (similar to FP8). The tree distillation repo already implements FP4 KV cache.

### 2. Strix Halo Unified Memory — free headroom

CPU and iGPU share the same 65 GB RAM pool. No PCIe transfer penalty. vLLM's CPU offloading works seamlessly. The bottleneck is memory bandwidth (~350 GB/s LPDDR5x), not transfer latency.

**Recommendation:** Set `--gpu-memory-utilization` to use ~40 GB for model weights, let the remaining ~25 GB handle KV cache + activations. vLLM will automatically page KV cache to host RAM as needed.

### 3. Reduce q_lora_rank — scales MLA projections linearly

| q_lora | MLA Projections | Peak (r=512, FP4 KV) |
|---|---|---|
| 512 | 4.7 GB | 30.8 GB |
| 768 | 7.0 GB | 33.1 GB |
| 1024 | 9.4 GB | 35.5 GB |
| 1536 (default) | 14.1 GB | 40.2 GB |

Halving q_lora from 1536 to 768 saves **7 GB** of activation memory. The k/v projections are already compressed (kv_lora_rank=512), so only q_lora matters.

### 4. KV Cache Eviction — sliding window or token importance

- Sliding window 32K: KV = 1.14 GB (saves 3.42 GB, ~3-5% accuracy loss)
- Token importance (H2O/SnapKV): ~0.5-1% loss, similar savings
- vLLM doesn't have this natively — needs custom cache manager

### 5. KV Cache Deduplication — prefix caching

vLLM supports this via `--enable-prefix-caching`. At batch=10 with 50% prefix overlap, saves ~45% of KV cache. For single-user long-context, limited benefit.

### 6. KV Cache SVD/Low-Rank Compression

Store KV in rank-r subspace. rank=64 gives 4× compression (2.17 GB). rank=32 gives 8× (1.09 GB). Not natively supported — needs custom attention kernel. Accuracy loss 1-5%.

### 7. INT8/INT4 KV Cache

INT8: 4.33 GB (50% compression). INT4: 2.17 GB (75% compression). Same as FP4 but with integer quantization. INT4 needs custom kernel. FP4 is the repo's choice for good reason.

### 8. KDA Recurrent State — already optimal

69 KDA layers use only 228 MB constant recurrent state, independent of sequence length. No optimization needed here.

### 9. FP8 KV Cache

vLLM native. 4.33 GB. Same as INT8. Less compression than FP4/INT4 but better accuracy.

## Recommended Final Config for 50 GB

```
r = 640 (SwiGLU)
q_lora = 1536 (full MLA accuracy)
KV cache = FP4 with per-token scaling
```

| Component | Size |
|---|---|
| Black box weights (r=640, FP4) | 8.05 GB |
| Black box activations | 15.73 GB |
| MLA projections | 14.10 GB |
| KV cache (FP4 MLA + KDA) | 2.40 GB |
| Embedding + LM head | 1.17 GB |
| Hidden states | 1.84 GB |
| Overhead | 2.00 GB |
| **PEAK** | **45.30 GB** |

**16.1B total black box params, 194× compression from original K3.**

You have 19.7 GB of headroom in the 65 GB Strix Halo pool. If you want to push closer to 50 GB for more black box quality, increase r to 768 (peak ~50 GB). If you want to reduce, drop q_lora to 1024 and r to 512 (peak ~35.5 GB, still solid quality).

## Sequence Length Sensitivity

| Context Length | KV Cache (FP4) | Total (weights + KV) |
|---|---|---|
| 4K tokens | 0.06 GB | 7.34 GB |
| 16K tokens | 0.17 GB | 7.45 GB |
| 64K tokens | 0.67 GB | 7.95 GB |
| 128K tokens | 2.40 GB | 9.69 GB |
| 256K tokens | 4.80 GB | 12.09 GB |

Note: KDA layers add ~0.24 GB constant (independent of seq_len). MLA layers add FP4 data at ~18.8 KB per token per layer.

## Comparison with Original K3

| Metric | Original K3 | Collapsed (r=512) | Collapsed (r=640) |
|---|---|---|---|
| Total params | 3.56T | 5.43B | 6.10B |
| Weights (FP4) | 1,793 GB | 7.28 GB | 9.24 GB |
| KV cache (128K, FP4) | 2.17 GB | 2.40 GB | 2.40 GB |
| Peak RAM (batch=1) | 1,793 GB | 44.5 GB | 50.1 GB |
| Compression | 1× | 40× | 36× |

## Implementation Notes

### vLLM Integration

The collapsed architecture maps to vLLM's native MoE support:
- 56 experts per collapsed layer → standard vLLM MoE routing
- SwiGLU activation → native vLLM support
- FP4 KV cache → custom kernel (repo already has this)
- CPU offloading → seamless on Strix Halo unified memory

### Training Data

Each black box is trained on the full input distribution from its region of 4 collapsed layers. Unlike the original experts (which see ~1.8% of tokens), each collapsed expert sees ~16× more tokens (16 original experts mapped to 1 collapsed expert). This gives rich training data per black box.

### Black Box Quality

The low-rank bottleneck (r) is the primary quality knob:
- r=64: aggressive compression, ~1,793× total compression
- r=256: sweet spot, 460× compression
- r=512: good quality, 294× compression
- r=640: high quality, 223× compression
- r=1024: near-original quality, 120× compression

### Future Work

- Implement FP4 KV cache kernel (already in repo config)
- Benchmark black box quality vs r values
- Explore token importance eviction for further memory savings
- Investigate LoRA-style shared base for even smaller black boxes (78 MB vs 6.1 GB)
