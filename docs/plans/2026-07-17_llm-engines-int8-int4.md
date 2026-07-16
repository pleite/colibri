# LLM Engines That Can Read Ornith int8/int4 (No GGUF)

**Date:** 2026-07-17

## Model Format

**Ornith int8 model:**
- Format: safetensors (122 shards, 208 GB)
- Architecture: `Qwen3_5MoeForConditionalGeneration` (MoE with 128 experts)
- Quantization: `compressed-tensors` (int8 channel-wise, symmetric)
- Config: `quantization_config` with `format: "float-quantized"`, `num_bits: 8`

**Ornith int4 model:** (not yet created, but would be similar with `num_bits: 4`)

## Engines That Can Read This Format

### 1. **colibri engine** (recommended)
- **Status:** Native support for safetensors + custom quantization
- **Pros:** Already built, supports MoE, supports int8/int4, GPU backend (ROCm)
- **Cons:** Not production-ready (still in development)
- **How to use:** `coli_runtime --model /opt/models/ornith-int8 --backend rocm`

### 2. **transformers + accelerate** (HuggingFace)
- **Status:** Native support for safetensors + compressed-tensors
- **Pros:** Mature, well-documented, PyTorch ecosystem
- **Cons:** Requires PyTorch + ROCm, memory-heavy (no optimized inference)
- **How to use:**
  ```python
  from transformers import AutoModelForCausalLM
  model = AutoModelForCausalLM.from_pretrained(
      "/opt/models/ornith-int8",
      device_map="auto",
      torch_dtype=torch.float16
  )
  ```

### 3. **vLLM**
- **Status:** Supports safetensors + compressed-tensors (with patching)
- **Pros:** Production-ready, optimized inference, OpenAI-compatible API
- **Cons:** Requires PyTorch + CUDA/ROCm, may need custom kernel for int8
- **How to use:**
  ```bash
  vllm serve /opt/models/ornith-int8 --dtype float16 --quantization compressed-tensors
  ```

### 4. **Ollama**
- **Status:** Does NOT support safetensors or MoE natively
- **Pros:** Easy to use, optimized for consumer hardware
- **Cons:** Requires GGUF format, no MoE support
- **Workaround:** Convert to GGUF first (but GGUF doesn't support MoE routing)

### 5. **llama.cpp / llama-server**
- **Status:** Does NOT support safetensors or MoE natively
- **Pros:** Optimized for consumer hardware, fast
- **Cons:** Requires GGUF format, no MoE support
- **Workaround:** Convert to GGUF first (but GGUF doesn't support MoE routing)

### 6. **DeepSpeed Inference**
- **Status:** Supports safetensors + custom quantization
- **Pros:** Production-ready, optimized for large models
- **Cons:** Requires PyTorch + CUDA/ROCm, complex setup
- **How to use:**
  ```python
  import deepspeed
  model = deepspeed.init_inference(
      model,
      dtype=torch.float16,
      tensor_parallel=1,
      replace_with_kernel_inject=True
  )
  ```

### 7. **Megatron-LM**
- **Status:** Supports safetensors + custom quantization
- **Pros:** Production-ready, optimized for large models
- **Cons:** Requires PyTorch + CUDA/ROCm, complex setup
- **How to use:** Requires custom training/inference scripts

## Recommendation

**For Ornith MoE (397B params):**

1. **Use colibri engine** — Already built, supports MoE, supports int8/int4, GPU backend (ROCm). This is the best option for now.

2. **Use transformers + accelerate** — If you need a quick test or benchmark. Requires PyTorch + ROCm.

3. **Use vLLM** — If you need production-ready inference with OpenAI-compatible API. Requires PyTorch + ROCm, may need custom kernel for int8.

**For dense models (like the 35B Q8_0 GGUF):**

1. **Use llama-server** — Already works, optimized for consumer hardware.

2. **Use Ollama** — Easy to use, optimized for consumer hardware.

## Summary

- **llama-server and Ollama cannot run Ornith MoE** (no GGUF/MoE support)
- **colibri engine is the best option** for Ornith MoE (already built, supports MoE + int8/int4)
- **transformers + accelerate** is a good alternative for testing/benchmarking
- **vLLM** is production-ready but may need custom kernel for int8
- **DeepSpeed/Megatron-LM** are options but complex to set up
