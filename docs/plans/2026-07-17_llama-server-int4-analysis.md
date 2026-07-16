# llama-server + int4/int8 Model Analysis

**Date:** 2026-07-17

## Current State

**Ornith int8 model exists at:** `/opt/models/ornith-int8/` (208 GB, 122 shards)
**Ornith FP8 model exists at:** `/opt/models/ornith-fp8/` (8.5 GB)
**Ornith Q8_0 GGUF exists at:** `/opt/models/ornith-1.0-35b-Q8_0.gguf` (35 GB)

**llama-server** is available in the `kyuz0/amd-strix-halo-toolboxes:vulkan-radv` container and supports:
- Flash Attention (`--flash-attn 1`)
- KV cache offloading (`--kv-unified`)
- SWA full cache (`--swa-full`)
- Slot-based inference (`--slot-prompt-similarity`, `--slot-save-path`)

## Can llama-server run the int4/int8 model?

**Yes, but with caveats:**

1. **llama-server expects GGUF format**, not safetensors. The int8 model is in safetensors format (122 shards).

2. **llama-server does NOT support MoE models natively.** Ornith is a 397B parameter MoE model with 128 experts. llama-server's GGUF format doesn't support MoE routing.

3. **The ornith-1.0-35b-Q8_0.gguf (35 GB) is a different model** — it's a 35B dense model, not the 397B MoE Ornith.

## What's missing for full functionality

**To run Ornith (397B MoE) with llama-server, you would need:**

1. **GGUF conversion with MoE support** — llama-server's GGUF format doesn't support MoE routing. You'd need a custom GGUF format or a different server.

2. **The int4 model doesn't exist yet** — Only int8 exists (208 GB). The int4 model would be ~104 GB (0.5 byte/param for dense weights, 1 byte/param for shared experts).

3. **ssd streaming** — llama-server supports `--cache-prompt`, `--cache-reuse`, `--cache-ram`, and `--slot-save-path` for memory management, but not ssd streaming for model weights.

## Recommended approach

**Use the colibri engine directly** (not llama-server) for Ornith MoE:

1. **Build the colibri toolbox container** with the int8/int4 model
2. **Run `coli_runtime` executable** with the model path and parameters
3. **Use the OpenAI server** (`c/openai_server.py`) for API access

**For llama-server**, use the existing `ornith-1.0-35b-Q8_0.gguf` (35B dense model) which already works.

## Podman command for testing int8 model

Since llama-server can't run MoE, here's a podman command for the colibri toolbox with the int8 model:

```bash
podman run --rm -it \
  --device /dev/dri \
  --device /dev/kfd \
  --cap-add SYS_PTRACE \
  --security-opt label=disable \
  -v /opt/models/ornith-int8:/opt/models/ornith-int8:ro \
  -v /opt/logs:/opt/logs:z \
  -e HSA_OVERRIDE_GFX_VERSION=11.5.1 \
  -e GGML_HIP_ENABLE_UNIFIED_MEMORY=1 \
  localhost/colibri:latest \
  bash -c "cd /opt/models/ornith-int8 && /opt/colibri/c/build/coli_runtime --model /opt/models/ornith-int8 --backend rocm --layers 999"
```

**But this won't work** because:
1. The colibri engine is not yet integrated with the backend runtime for int8/int4
2. The engine currently dequantizes everything to F32 (no memory savings)

## Next steps

1. **Implement Qwen3.5 engine quantization storage** — Store tensors in INT8/INT4 format in the `QLayer` struct, use backend runtime for matmul, dequant-on-use at matmul time.

2. **Build the colibri toolbox container** with the int8/int4 model support.

3. **Test with `coli_runtime` executable** directly (not llama-server).

4. **For llama-server**, continue using the existing `ornith-1.0-35b-Q8_0.gguf` (35B dense model).

## Summary

- **llama-server cannot run Ornith MoE** (no MoE support in GGUF format)
- **int4 model doesn't exist yet** (only int8 at 208 GB)
- **Use colibri engine directly** for MoE models, not llama-server
- **llama-server is for dense models** (like the existing 35B Q8_0 GGUF)
