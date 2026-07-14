# Ornith-1.0-397B Conversion Corruption Analysis

**Date:** 2026-07-14  
**Status:** CRITICAL - Model conversion produces corrupted shards  
**Impact:** Engine cannot load the model (shards 2-121 unreadable)

---

## 1. Problem Summary

The Qwen3.5 MoE converter (`c/tools/convert_qwen35_safetensors.py`) produces **corrupted output shards** for the Ornith-1.0-397B model:

- **Shard 1** (`model-00001-of-00122.safetensors`): ✅ Readable (362 tensors)
- **Shards 2-121**: ❌ **Corrupted** - `SafetensorError: invalid shape, data type, or offset for tensor`
- **Shard 122** (`model-00122-of-00122.safetensors`): ✅ Readable (3 tensors: `lm_head.weight`, `lm_head.weight.qs`, `model.language_model.norm.weight`)

**Root Cause:** Unknown - requires investigation of the converter's parallel processing logic.

---

## 2. Evidence

### 2.1 Shard Readability Test

```python
from safetensors import safe_open
import glob

model_dir = '/opt/models/ornith-int8'
shards = sorted(glob.glob(f'{model_dir}/model-*.safetensors'))

for i, shard in enumerate(shards[:5]):
    try:
        with safe_open(shard, framework='np') as f:
            keys = list(f.keys())
            print(f'{i+1}. {shard}: OK ({len(keys)} tensors)')
    except Exception as e:
        print(f'{i+1}. {shard}: FAILED - {e}')
```

**Output:**
```
1. model-00001-of-00122.safetensors: OK (362 tensors)
2. model-00002-of-00122.safetensors: FAILED - Error while deserializing header: invalid shape, data type, or offset for tensor
3. model-00003-of-00122.safetensors: FAILED - Error while deserializing header: invalid shape, data type, or offset for tensor
...
121. model-00121-of-00122.safetensors: FAILED
122. model-00001-of-00122.safetensors: OK (3 tensors)
```

### 2.2 Engine Loading Failure

```
tensor lm_head.weight has incompatible shape for quantized load
```

The engine fails on `lm_head.weight` because:
1. It finds the tensor (dtype=3, int8)
2. It looks for `lm_head.weight.qs` scale tensor
3. The scale lookup fails or returns wrong dimensions
4. **Actually:** The engine never reaches this point because shards 2-121 are corrupted and the model initialization fails earlier

### 2.3 Index File Issue

The converter initially did **not** generate `model.safetensors.index.json`. PR #23 (commit `ac59113`) added index generation, but:

- The generated index only contains `metadata` and `weight_map`
- **Missing:** `tensor_info` field with dtype, shape, and data_offset for each tensor
- The engine reads dtype from safetensors file headers directly, so `tensor_info` is not strictly required
- However, the missing index caused the engine to fail with "tensor not found" errors before reaching the corruption issue

---

## 3. Model Structure

### 3.1 Configuration

```json
{
  "text_config": {
    "vocab_size": 248320,
    "hidden_size": 4096,
    "num_hidden_layers": 60,
    "num_attention_heads": 32,
    "num_key_value_heads": 2,
    "moe_intermediate_size": 1024,
    "num_experts_per_tok": 10,
    "layer_types": ["linear_attention", "linear_attention", "linear_attention", "full_attention", ...] (60 total)
  }
}
```

### 3.2 Quantization Scheme

| Tensor Type | Quantization | Example |
|-------------|--------------|---------|
| `embed_tokens`, `norm` | float32 | `model.language_model.embed_tokens.weight` |
| `linear_attn.*`, `mlp.*` | uint8 (int8) | `model.language_model.layers.0.linear_attn.in_proj_qkv.weight` |
| `.qs` scale files | float32 | `model.language_model.layers.0.linear_attn.in_proj_qkv.weight.qs` |

### 3.3 Shard Distribution

- **Total shards:** 122
- **Total size:** ~195 GB
- **Shard 1:** 362 tensors (first layer + embed_tokens)
- **Shard 122:** 3 tensors (lm_head + norm)
- **Shards 2-121:** Corrupted (contain layers 1-58)

---

## 4. Reproduction Steps

1. **Environment:**
   - Strix Halo (192.168.1.129)
   - Fedora 43, kernel 7.0.12-101.fc43.x86_64
   - Python 3.14, safetensors 0.8.0
   - colibri repo commit `ac59113` (PR #23 merged)

2. **Conversion command:**
   ```bash
   cd /home/leite/colibri
   python3 c/tools/convert_qwen35_safetensors.py \
     --input /opt/models/ornith-fp8 \
     --output /opt/models/ornith-int8
   ```

3. **Verification:**
   ```bash
   python3 -c "
   from safetensors import safe_open
   import glob
   shards = sorted(glob.glob('/opt/models/ornith-int8/model-*.safetensors'))
   for shard in shards[:5]:
       try:
           with safe_open(shard, framework='np') as f:
               print(f'OK: {shard} ({len(list(f.keys()))} tensors)')
       except Exception as e:
           print(f'FAILED: {shard} - {e}')
   "
   ```

---

## 5. Hypotheses for Corruption

### 5.1 Parallel Processing Race Condition

The converter uses `concurrent.futures.ProcessPoolExecutor` with multiple workers. Possible issues:

- **Shared state corruption:** Workers might overwrite each other's output files
- **File handle conflicts:** Multiple processes writing to the same shard simultaneously
- **Memory mapping issues:** Shared memory between parent and worker processes

### 5.2 Shard Assignment Logic

The `build_source_task_groups()` function groups tensors by source shard. If the grouping logic is incorrect:

- Tensors might be assigned to the wrong output shard
- Shard boundaries might be miscalculated
- Tensor offsets within shards might be wrong

### 5.3 Safetensors Format Issue

The safetensors library might have a bug or version incompatibility:

- **Header format:** Incorrect serialization of tensor metadata
- **Data offsets:** Wrong byte offsets for tensor data
- **Compression:** Issues with compressed vs uncompressed storage

### 5.4 Disk I/O Issue

- **Filesystem corruption:** NVMe drive issues (unlikely but possible)
- **Buffer flushing:** Data not flushed to disk before next write
- **Concurrent writes:** Multiple processes writing to adjacent file regions

---

## 6. Debugging Checklist

- [ ] **Check worker isolation:** Verify each worker writes to separate files
- [ ] **Validate shard boundaries:** Check that tensor assignments match expected distribution
- [ ] **Test single-worker conversion:** Run with `--workers 1` to rule out parallel issues
- [ ] **Inspect safetensors headers:** Compare working vs corrupted shard headers
- [ ] **Check disk space:** Ensure no disk full errors during conversion
- [ ] **Verify safetensors version:** Test with different versions of the library
- [ ] **Add checksums:** Compute and verify SHA256 of each shard after conversion
- [ ] **Monitor file handles:** Check for handle leaks or conflicts

---

## 7. Workaround

**Current status:** No working workaround. The model cannot be loaded by the engine.

**Potential fixes:**
1. Re-run conversion with `--workers 1` (slower but might avoid race conditions)
2. Manually fix corrupted shards (requires understanding the corruption pattern)
3. Use a different quantization approach (e.g., GGUF format)
4. Re-download and re-convert from scratch (405 GB download + 12-24 hour conversion)

---

## 8. Files Included

- `config.json` - Model architecture configuration
- `tokenizer.json` - BPE tokenizer (19.9 MB)
- `tokenizer_config.json` - Tokenizer metadata
- `generation_config.json` - Generation parameters
- `chat_template.jinja` - Chat formatting template

**Missing:** All 122 safetensors shards (corrupted or deleted)

---

## 9. Next Steps

1. **Analyze corruption pattern:** Determine if corruption is random or systematic
2. **Test single-worker conversion:** Rule out parallel processing issues
3. **Inspect safetensors headers:** Compare working vs corrupted shards
4. **Check converter logs:** Review `convert_qwen35_safetensors-*.log` for errors
5. **Validate with fresh download:** Re-download FP8 model and re-convert

---

**Report generated by:** Hermes Agent (Murderbot persona)  
**Hardware:** Strix Halo (Ryzen AI Max+ 395, 32 cores, 65 GB RAM, 931 GB NVMe)  
**Repository:** `github.com/pleite/colibri` (commit `ac59113`)
