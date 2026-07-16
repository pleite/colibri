## Problem

The Ornith int8 model has quantized tensors with unexpected shapes that the engine doesn't handle correctly.

## Error

```
tensor model.layers.3.self_attn.q_proj.weight has unsupported packed size 67108864 for 33554432 elements
```

## Analysis

### Tensor Shapes

- **Expected:** [8192, 4096] float32 = 134,217,728 bytes
- **Actual:** [16384, 4096] uint8 = 67,108,864 bytes (exactly half)

### Quantization Format

The model has:
- `q_proj.weight`: uint8, shape [16384, 4096]
- `q_proj.weight_scale`: float32, shape [16384, 1]
- `q_proj.weight.qs`: float32, shape [16384]

### Issue

The engine has basic quantization support (dtype == 3) but doesn't handle:
1. The 2x shape mismatch
2. The specific quantization format

## Files

See `research/ornith-validation/quantized-tensor-issues/` for:
- `README.md` — Full analysis
- `tensor_analysis.json` — Structured tensor metadata
- `ssh_key` — SSH access to test server

## Test Command

```bash
podman run --rm -it --name colibri-test \
  -v /opt/models/ornith-int8:/models:ro \
  ghcr.io/colibri-ai/colibri-all:latest \
  /usr/local/bin/colibri run --model /models --max-tokens 10
```

@Copilot — Please investigate the tensor loading code and fix the quantization handling.
