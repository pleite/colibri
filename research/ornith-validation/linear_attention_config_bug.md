# Linear Attention Config Bug: linear_num_key_heads Mismatch

**Date**: 2026-07-16
**Model**: Ornith 397B (Qwen3_5MoeForConditionalGeneration) int8 quantized
**Engine**: colibri c/qwen35_moe.c
**PR**: #40 — fix/ornith-linear-attention-dimensions

---

## Executive Summary

The config.json for the Ornith int8 model has an incorrect value for linear_num_key_heads.
The config says 16, but the actual safetensors weight tensors for in_proj_a and in_proj_b
have a first dimension of 64. The engine PR #40 correctly makes the engine read config
values instead of hardcoded numbers, but the config value itself is wrong.

**This is a config.json bug, not an engine bug.** The fix is to correct linear_num_key_heads
from 16 to 64 in config.json.

---

## Evidence

### Config values (text_config in /opt/models/ornith-int8/config.json)

  linear_num_key_heads: 16
  linear_key_head_dim: 128
  linear_num_value_heads: 64
  linear_value_head_dim: 128
  num_key_value_heads: 2
  head_dim: 256
  hidden_size: 4096
  num_hidden_layers: 60

### Actual tensor shapes (from safetensors shard model-00001-of-00122.safetensors)

  linear_attn.in_proj_a.weight:     (64, 4096)     -> engine wants 16*4096=65536 floats  -> WRONG, reads 1/4 of tensor
  linear_attn.in_proj_b.weight:     (64, 4096)     -> engine wants 16*4096=65536 floats  -> WRONG, reads 1/4 of tensor
  linear_attn.in_proj_qkv.weight:   (12288, 4096)  -> engine wants 3*4096*4096=50331648  -> OK
  linear_attn.in_proj_z.weight:     (8192, 4096)   -> engine wants 64*128*4096=33554432  -> OK
  linear_attn.out_proj.weight:      (4096, 8192)   -> engine wants 4096*64*128=33554432  -> OK
  linear_attn.norm.weight:          (128,)         -> engine wants 128 (linear_key_head_dim) -> OK
  linear_attn.A_log.weight:         (64,)          -> engine wants 64 (linear_num_value_heads) -> OK
  linear_attn.dt_bias.weight:       (64,)          -> engine wants 64 (linear_num_value_heads) -> OK
  linear_attn.conv1d.weight:        (12288, 1, 4)  -> engine wants 3*4096*1*4=49152      -> OK

### The math

  Config says linear_num_key_heads = 16
  Engine computes in_proj_a size = linear_num_key_heads * hidden_size = 16 * 4096 = 65,536 floats
  Actual tensor in_proj_a has shape (64, 4096) = 262,144 floats
  THE ENGINE WOULD READ ONLY THE FIRST 65,536 FLOATS (first 16 rows) AND IGNORE THE REMAINING 48 ROWS
  This means the linear attention K-projection weights are 75% missing at runtime

### If fixed to 64

  linear_num_key_heads = 64
  Engine computes 64 * 4096 = 262,144 floats
  Actual tensor: (64, 4096) = 262,144 floats
  MATCH

---

## Why the config is wrong

The original FP8 config (/opt/models/ornith-fp8/config.json) also has linear_num_key_heads: 16.
This is the model author original config from HuggingFace, and it appears to be incorrect.

Looking at the tensor shapes:
  - in_proj_a and in_proj_b (linear attention K-projections): first dim = 64
  - in_proj_z and out_proj (linear attention V-projections): first dim = 64 * 128 = 8192
  - linear_num_value_heads = 64

The in_proj_a/b first dimension (64) matches linear_num_value_heads (64), not linear_num_key_heads (16).

Most likely: The HuggingFace config for Ornith has a bug where linear_num_key_heads was set to 16
(possibly copied from a different model or template default) when it should be 64. The actual
trained weights have 64 key heads, matching the value heads. The linear_key_head_dim = 128 is
still valid and used in the computation kernel — it just does not affect the projection tensor size.

---

## Impact on the engine

### Before PR #40 (hardcoded values)
  in_proj_a/b: hardcoded to 128 * 4096 = 524,288 floats. Tensor has 262,144.
  Engine would read past the end of the tensor -> crash or garbage.
  This was the original crash: tensor shape mismatch.

### After PR #40 (config-driven, with current config)
  in_proj_a/b: reads 16 * 4096 = 65,536 floats. Tensor has 262,144.
  Engine reads only 25% of the weights -> silent incorrect computation, no crash.
  The model would load but produce garbage outputs.

### After PR #40 + config fix (linear_num_key_heads = 64)
  in_proj_a/b: reads 64 * 4096 = 262,144 floats. Tensor has 262,144. CORRECT

---

## PR #40 status

### What the PR fixes (engine side)
  - Makes ALL linear attention tensor sizes config-driven (no more hardcoded magic numbers)
  - Adds linear_num_value_heads and linear_value_head_dim parsing
  - Fixes the critical A_log/dt_bias mismatch (128 -> 64)
  - Fixes in_proj_qkv/conv1d (12288 -> 3*hidden)
  - Fixes in_proj_z/out_proj (8192 -> num_value_heads * value_head_dim)
  - Fixes norm (128 -> linear_key_head_dim)

### What the PR does NOT fix
  - The config.json value of linear_num_key_heads is still 16
  - After merging PR #40, the engine will correctly read the wrong config value
  - The engine will silently load with 75% missing K-projection weights

---

## Required additional fix

Fix config.json: Change linear_num_key_heads from 16 to 64 in BOTH:
  - /opt/models/ornith-int8/config.json (text_config.linear_num_key_heads)
  - /opt/models/ornith-fp8/config.json (text_config.linear_num_key_heads)

Then rebuild the model directory.

---

## Files involved

  File                          Path                                          Action
  Engine source                 c/qwen35_moe.c                                PR #40 fixes this (OK)
  Config (int8)                 /opt/models/ornith-int8/config.json           Fix linear_num_key_heads: 16 -> 64
  Config (FP8 source)           /opt/models/ornith-fp8/config.json            Fix linear_num_key_heads: 16 -> 64
  Converter                     c/tools/convert_qwen35_safetensors.py         May need fix to write correct config

---

## Notes for Copilot

The PR #40 code changes are correct — they make the engine config-driven. But this PR should
NOT be merged until the config.json fix is also applied. The correct sequence is:

1. Fix config.json (linear_num_key_heads: 64)
2. Merge PR #40 (engine now reads config correctly)
3. Rebuild container
4. Test with Ornith int8 model
5. Verify the model produces coherent outputs

The linear_key_head_dim = 128 value is CORRECT and used in the computation kernel.
Only linear_num_key_heads is wrong.
