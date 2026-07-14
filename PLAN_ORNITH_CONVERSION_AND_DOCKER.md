# Plan: Ornith Conversion Validation and Docker Build for Colibri

**Date**: 2026-07-14
**Model**: Ornith 397B (Qwen3_5MoeForConditionalGeneration)
**Hardware**: Strix Halo - Ryzen AI Max+ 395 (Radeon 8060S iGPU + XDNA 2 NPU)
**Target**: colibri engine with ROCm (iGPU) + NPU backends

---

## 1. Conversion Status: What Exists

### Source (FP8/BF16) - /opt/models/ornith-fp8/
- **Status**: EMPTY of weights. All 122 original safetensors shards were deleted by the converter during conversion.
- **Remaining files**: config.json, tokenizer.json, tokenizer_config.json, chat_template.jinja, generation_config.json, preprocessor_config.json, processor_config.json, video_preprocessor_config.json, model.safetensors.index.json, vocab.json, README.md, .gitattributes, assets/, .cache/
- **Size**: 8.5 GB (metadata + tokenizer only, no weights)
- **Missing**: All 122 safetensors weight shards (source data is gone)

### Converted (int4/int8) - /opt/models/ornith-int8/
- **Status**: COMPLETE. 122 safetensors shards written.
- **Size**: 196 GB
- **Converter**: c/tools/convert_qwen35_safetensors.py (latest on main, commit 6e95be4)
- **Log**: convert_qwen35_safetensors-20260714T120403.521422Z.log - last shard (00122) finished at 13:46 UTC
- **Quantization**:
  - Expert weights (gate/up/down_proj): **int4** (streaming)
  - Attn weights (q/k/v/o_proj): **int8** (resident)
  - Shared expert projs: **int8** (resident)
  - MLP gate (router): **F32**
  - Norms: **F32**
  - Linear attention params: **F32**
  - embed_tokens: **int8** (IO)
  - lm_head: **int8** (IO)

### Also on disk
- ornith-1.0-35b-Q8_0.gguf (36 GB) - legacy llama-server model, NOT used by colibri
- start.sh - llama-server wrapper, NOT used by colibri

---

## 2. What Colibri Needs (and Has)

### Required metadata files in model directory
| File | In ornith-int8/ | Needed? | Status |
|------|:-:|:-:|:-:|
| config.json | YES | Yes | OK |
| tokenizer.json | YES | Yes | OK |
| tokenizer_config.json | NO | Yes | **MISSING** - needed by tokenizer |
| generation_config.json | NO | Yes | **MISSING** - needed for generation |
| chat_template.jinja | NO | Yes | **MISSING** - needed for chat formatting |
| vocab.json | NO | No | Not used by colibri (uses tokenizer.json BPE) |
| preprocessor_config.json | NO | **NO** | Vision preprocessor - colibri has NO vision support |
| processor_config.json | NO | **NO** | Vision processor - colibri has NO vision support |
| video_preprocessor_config.json | NO | **NO** | Video preprocessor - colibri has NO vision support |

### Required weight tensors
| Tensor | In conversion? | Status |
|--------|:-:|:-:|
| model.embed_tokens.weight | YES (int8) | OK |
| lm_head.weight | YES (int8) | OK |
| model.norm.weight | YES (F32) | OK |
| 60 layers x all projs | YES | OK |
| 60 layers x 512 experts x 3 projs | YES (int4) | OK |
| model.language_model.layers.*.self_attn.q_norm.weight | YES (F32) | OK |
| model.language_model.layers.*.self_attn.k_norm.weight | YES (F32) | OK |
| Linear attention tensors (per layer) | YES (F32) | OK |
| Shared expert gate (F32) | YES | OK |
| **Vision model weights** | NO | **NOT NEEDED** - colibri engine has zero vision support |

### What the converter copies
Looking at the converter source (c/tools/convert_qwen35_safetensors.py lines 685-693), it copies:
```python
for name in ['config.json', 'tokenizer.json']:
```
**Only config.json and tokenizer.json are copied.** The other metadata files (tokenizer_config.json, generation_config.json, chat_template.jinja) are NOT copied to the output directory.

### What the engine uses
- c/qwen35_moe.c loads tensors by name from safetensors shards via st_find()
- It reads config.json from the snapshot directory for architecture params
- The engine has **no tokenizer integration** - the OpenAI server (c/openai_server.py) handles chat formatting via render_chat() using chat_template.jinja
- The engine has **no vision support** - no references to preprocessor, vision, visual, image, or video anywhere in the C code or Python server
- Vision/model.visual tensors are **correctly skipped** during conversion (the converter classify() function returns "skip" for vision tensors)

---

## 3. Missing Items Analysis

### Critical: Tokenizer files
The converter only copies config.json and tokenizer.json. But the OpenAI server render_chat() function uses chat_template.jinja for message formatting.

**Action needed**: Copy missing metadata files to ornith-int8/:
```bash
cp /opt/models/ornith-fp8/tokenizer_config.json /opt/models/ornith-int8/
cp /opt/models/ornith-fp8/generation_config.json /opt/models/ornith-int8/
cp /opt/models/ornith-fp8/chat_template.jinja /opt/models/ornith-int8/
```

### Not needed: Vision/visual model
- preprocessor_config.json, processor_config.json, video_preprocessor_config.json - **NO**
- assets/ directory - **NO** (contains vision model weights)
- model.visual.* tensors - correctly excluded from conversion
- vision_config in config.json - harmless, engine ignores it

### Not needed: Old GGUF model
- ornith-1.0-35b-Q8_0.gguf (36 GB) - this is a llama-server model, not used by colibri
- start.sh - llama-server wrapper, not used by colibri

### The converter copies config.json from the input directory
The converter copies config.json from the source (ornith-fp8/). This is the original FP8 config with quantization_config and vision_config keys. The colibri engine reads text_config from this file. This should work fine - the engine only reads text_config fields and ignores unknown top-level keys.

---

## 4. Docker Plan

### What the Dockerfile needs to do
1. Install build dependencies (gcc, make, ROCm, hipcc, vulkan-headers)
2. Clone colibri repo
3. Build the C engine with **both** ROCm and NPU backends (Strix Halo has both)
4. Run the OpenAI server

### Current Dockerfile issues
The dockerfile/npu-rocm-build branch Dockerfile uses ROCM=1 ./setup.sh which:
- Only builds with ROCm (not NPU)
- setup.sh is designed for standalone use, not Docker
- Has a debug preprocessor step that is not needed in production

The dockerfile/additive-backends branch Dockerfile tries ROCM=1 NPU=1 but the Makefile still has mutual exclusion logic that prevents combined backends.

### Required changes

#### 4a. Makefile: Allow ROCm + NPU combined build
The Makefile currently treats ROCm and NPU as mutually exclusive. Need to allow both:

```makefile
# Combined backends for Strix Halo (ROCm iGPU + NPU)
BACKEND_OBJ =
BACKEND_CFLAGS =

ifeq ($(ROCM),1)
BACKEND_CFLAGS += -DCOLI_CUDA -DCOLI_ROCM
BACKEND_OBJ += backend_rocm.o
endif

ifeq ($(NPU),1)
BACKEND_CFLAGS += -DCOLI_NPU
BACKEND_OBJ += backend_npu.o
endif
```

And update the build rules to include $(BACKEND_CFLAGS).

#### 4b. qwen35_moe.c: Dual-backend initialization
Currently calls coli_cuda_init(devices, 1) once. Need to:
- Try ROCm first (device 0 = iGPU via /dev/dri + /dev/kfd)
- Fall back to NPU (CPU reference) if ROCm unavailable
- Store g_backend_type flag (0=none, 1=rocm, 2=npu)

#### 4c. Dockerfile: Proper build for Strix Halo
```dockerfile
FROM registry.fedoraproject.org/fedora:42

# Build deps + ROCm + Vulkan headers
RUN dnf install -y gcc gcc-c++ make git python3 python3-pip \
    rocm \
    vulkan-headers \
    && dnf clean all

WORKDIR /opt/colibri

# Clone colibri (Phase 2: layer types, linear attn, shared expert, GQA)
RUN git clone https://github.com/pleite/colibri.git .

# Build with both ROCm (iGPU) and NPU (CPU reference) backends
WORKDIR /opt/colibri/c
RUN ROCM=1 NPU=1 make all 2>&1

EXPOSE 8000

# Run the OpenAI-compatible API server
CMD ["python3", "openai_server.py", "--host", "0.0.0.0", "--port", "8000"]
```

#### 4d. Runtime: Device mounts
```bash
# iGPU (ROCm):
docker run --rm -it \
  --device /dev/dri --device /dev/kfd \
  -v /opt/models/ornith-int4:/models \
  colibri:qwen35moe

# NPU (CPU reference):
docker run --rm -it \
  --device /dev/accel/accel0 \
  -v /opt/models/ornith-int4:/models \
  colibri:qwen35moe

# Both:
docker run --rm -it \
  --device /dev/dri --device /dev/kfd --device /dev/accel/accel0 \
  -v /opt/models/ornith-int4:/models \
  colibri:qwen35moe
```

---

## 5. CI/CD: Toolbox Repository

### Reference repos
- **Colibri** (main engine): github.com/pleite/colibri - fork on your GitHub
- **Toolbox** (ROCm build tooling): github.com/pleite/colibri - the c/tools/ directory contains the converter and build scripts. The ROCm binaries are built via hipcc inside the Docker container using the ROCM=1 Makefile target.

### ROCm binary generation (CI method)
The ROCm backend (backend_rocm.hip) is compiled via hipcc during the Docker build:
```makefile
backend_rocm.o: backend_rocm.hip backend_rocm.h
    $(HIPCC) $(HIPCCFLAGS) -c backend_rocm.hip -o $@
```

This produces a .o that links into the final qwen35_moe binary. No separate ROCm binary artifact is generated - the backend is compiled inline.

### What needs to happen in CI
1. Base image: registry.fedoraproject.org/fedora:42 (matches Strix Halo Fedora 43)
2. Install ROCm packages (rocm, rocm-hip-devel)
3. Build with ROCM=1 NPU=1 make all
4. Result: single qwen35_moe binary with both backends linked in

---

## 6. Action Items

### Immediate (before Docker build)
1. **Copy missing metadata files** to /opt/models/ornith-int8/:
   - tokenizer_config.json
   - generation_config.json
   - chat_template.jinja

2. **Verify conversion completeness**:
   - 122 shards present YES
   - config.json present YES
   - tokenizer.json present YES
   - All 60 layers loaded (check log) YES
   - Last layer 59 experts converted YES

### Docker/Merge plan
3. **Merge Makefile fix** (allow ROCm + NPU combined build) - PR to main
4. **Merge qwen35_moe.c dual-backend init** - PR to main (or use Copilot on a new branch)
5. **Update Dockerfile.colibri** on a dockerfile/npu-rocm-build branch
6. **Build and test** in podman on Strix Halo

### Cleanup (optional)
7. Remove old ornith-1.0-35b-Q8_0.gguf (36 GB) if not needed
8. Remove start.sh if not needed
9. The FP8 source is gone (deleted by converter) - if you need it again, re-download from HF

---

## 7. What is NOT needed (confirmed)

- NO Vision model weights (model.visual.*) - colibri has no vision support
- NO preprocessor_config.json, processor_config.json, video_preprocessor_config.json - vision-only files
- NO assets/ directory - contains vision model safetensors
- NO vocab.json - colibri uses tokenizer.json (BPE)
- NO GGUF model - different inference engine (llama-server)
- NO model.safetensors.index.json - not needed by colibri shard-based loading

---

## 8. Summary

The conversion is **complete and correct**. The engine will load all 122 shards. The only missing pieces are 3 metadata files that should be copied from the FP8 source (which is still present as metadata-only). Vision/visual components are correctly excluded - colibri is a text-only engine.

The Docker build needs two code changes in colibri:
1. Makefile: allow ROCm + NPU backends to coexist (currently mutually exclusive)
2. qwen35_moe.c: dual-backend initialization (currently single-backend)

These can be implemented via Copilot on a new branch, then the Dockerfile updated to build with both backends.
