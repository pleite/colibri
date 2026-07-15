# Ornith Model Validation Report

**Date:** 2026-07-15  
**Status:** FAILED - Engine cannot load model  
**Impact:** Colibri engine crashes on tensor shape validation

---

## 1. Problem Summary

The Ornith-1.0-397B model conversion is **complete** (122 shards, 208 GB), but the colibri engine fails to load it with:

```
tensor model.embed_tokens.weight has incompatible shape for quantized load
```

---

## 2. Model Conversion Status

### Files Present
- ✅ `config.json` (5.2 KB) - Model architecture config
- ✅ `tokenizer.json` (19.9 MB) - BPE tokenizer
- ✅ `tokenizer_config.json` (1.2 KB) - Tokenizer metadata
- ✅ `generation_config.json` (244 B) - Generation parameters
- ✅ `chat_template.jinja` (7.8 KB) - Chat formatting
- ✅ `model.safetensors.index.json` (26.1 MB) - Shard index
- ✅ 122 safetensors shards (208 GB total)

### Quantization
- Expert weights: int4 (streaming)
- Attn weights: int8 (resident)
- Shared expert projs: int8 (resident)
- Norms/router/linear_attn: F32
- embed_tokens: int8 with .qs scale
- lm_head: int8 with .qs scale

---

## 3. Engine Failure Analysis

### Error Message
```
tensor model.embed_tokens.weight has incompatible shape for quantized load
```

### Root Cause
The engines function checks:
