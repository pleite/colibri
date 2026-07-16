# vLLM Conservative Memory Configuration

**Date:** 2026-07-17

## Current System State

**Memory:** 64 GB total, 48 GB used, 11 GB free, 13 GB available
**Swap:** 8 GB total, 243 MB used
**llama-server service:** Active (running, 22.8 GB memory limit)

## Conservative Memory Configuration

**Goal:** Leave enough memory for OS + llama-server service + vLLM container

**Memory budget:**
- OS + system services: ~8 GB
- llama-server service: ~22 GB (current limit)
- vLLM container: ~20 GB (conservative)
- **Total: ~50 GB** (leaves 14 GB free for safety)

## Podman Command for vLLM (Conservative)

```bash
podman run --rm -it \
  --device /dev/dri \
  --device /dev/kfd \
  --cap-add SYS_PTRACE \
  --security-opt label=disable \
  --memory 20g \
  --memory-swap 24g \
  -v /opt/models/ornith-int8:/opt/models/ornith-int8:ro \
  -v /opt/logs:/opt/logs:z \
  -e HSA_OVERRIDE_GFX_VERSION=11.5.1 \
  -e GGML_HIP_ENABLE_UNIFIED_MEMORY=1 \
  -p 8000:8000 \
  localhost/tb-vllm-toolbox:latest \
  vllm serve /opt/models/ornith-int8 \
    --load-format safetensors \
    --quantization compressed-tensors \
    --offload-backend prefetch \
    --offload-group-size 16 \
    --offload-num-in-group 4 \
    --dtype float16 \
    --max-model-len 2048 \
    --host 0.0.0.0 \
    --port 8000
```

**Key changes for conservatism:**
- `--memory 20g` — Hard limit of 20 GB for vLLM
- `--memory-swap 24g` — Allow 4 GB swap
- `--offload-group-size 16` — Larger groups, fewer layers offloaded
- `--offload-num-in-group 4` — Offload 4 layers per group
- `--max-model-len 2048` — Shorter context to reduce KV cache memory

## Available Podman Containers/Toolboxes

**Running containers:**
1. `llama-vulkan-radv` — llama-server (vulkan-radv toolbox)
2. `ornith-test` — colibri engine (exited)
3. `colibri-toolbox` — colibri-all (running, sleep infinity)

**Available images:**
1. `localhost/tb-vllm-toolbox:latest` — 35.4 GB (vLLM)
2. `ghcr.io/pleite/tb-vllm-toolbox:dev` — 35.5 GB (vLLM)
3. `localhost/colibri:latest` — 9.75 GB (colibri)
4. `ghcr.io/pleite/colibri-all:latest` — 9.75 GB (colibri)
5. `localhost/colibri-server:latest` — 10.9 GB (colibri server)

## Is This a Replacement for llama-server?

**No, it's a complement:**
- **llama-server** — Runs the 35B dense model (ornith-1.0-35b-Q8_0.gguf), already active
- **vLLM** — Would run the 397B MoE model (ornith-int8), different model

**They can run simultaneously:**
- llama-server: 22 GB memory limit
- vLLM: 20 GB memory limit
- Total: 42 GB (leaves 22 GB free for OS + safety)

## How to Test

1. **Start vLLM container** (above command)
2. **Test API endpoint:**
   ```bash
   curl http://localhost:8000/v1/completions \
     -H "Content-Type: application/json" \
     -d '{
       "model": "/opt/models/ornith-int8",
       "prompt": "Hello, world!",
       "max_tokens": 100
     }'
   ```
3. **Monitor memory:**
   ```bash
   podman stats vllm-container
   free -h
   ```

## Next Steps

1. **Run the podman command above** to start vLLM with conservative memory
2. **Test the API endpoint** to verify it works
3. **Monitor memory usage** to ensure stability
4. **Adjust parameters** if needed (e.g., `--offload-group-size`, `--max-model-len`)
