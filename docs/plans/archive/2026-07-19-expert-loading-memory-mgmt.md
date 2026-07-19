# Qwen3.5 MoE Expert Loading & Memory Management Implementation Plan

> **For Hermes:** Use subagent-driven-development skill to implement this plan task-by-task.

**Goal:** Enable the colibri engine to run Ornith 397B (512 experts/layer, 60 layers) on Strix Halo's 65 GB RAM by implementing on-demand expert loading with eviction, disk streaming, and memory accounting.

**Architecture:** The engine currently loads all expert tensors as F32 defaults then frees them, but never unloads experts after use. With 512 experts × 60 layers = 30,720 expert triples (each ~12 MB int8 = ~360 GB total), we need: (1) per-category memory tracking, (2) LRU eviction when RAM approaches limit, (3) disk-based streaming for non-resident experts, (4) RAM-limit integration that triggers eviction before OOM.

**Tech Stack:** C99, colibri engine (`c/qwen35_moe.c`), safetensors reader (`c/st.h`), backend runtime (`c/backend_runtime.h`), ROCm optional.

---

## File to Modify

**Single file:** `/home/leite/colibri/c/qwen35_moe.c` (1986 lines)

All changes are additive — no existing function signatures change. We add new structs, new functions, and wire them into existing call sites.

---

## Context for Implementers

### Current Expert Loading Flow (lines 1290-1315, 1640-1679)

1. `init_model()` calls `init_layer_defaults()` which allocates F32 identity matrices for all experts, then frees them and sets `expert_state[expert] = QWEN_EXPERT_STATE_UNLOADED`
2. `run_model()` calls `ensure_expert()` which loads 3 tensors (gate, up, down) from safetensors on first routing
3. Once loaded, experts stay in memory forever — no eviction
4. `QWEN_EXPERT_STATE_PINNED` is defined but never used

### Current Memory Tracking (lines 247-263)

- `g_ram_limit_bytes` — configurable via `--ram-limit-mb` or `COLI_QWEN_RAM_LIMIT_MB`
- `g_ram_peak_bytes` — monotonically increasing, never decremented
- `reserve_ram()` — checks if adding `bytes` would exceed limit, fails if so
- No per-category tracking

### Current QTensor Layout (lines 108-116)

```c
typedef struct {
    int fmt;              // 0=F32, 1=INT8, 2=INT4
    int O, I;             // output/input dimensions
    void *data;           // packed payload (INT8/INT4) or F32 array
    float *scales;        // per-output-row scales (INT8/INT4)
    ColiCudaTensor *handle; // backend-runtime cache handle
} QTensor;
```

### Expert Tensor Sizes (Ornith int8 config)

- `moe_intermediate_size` = 1024, `hidden_size` = 4096
- gate_proj: [1024, 4096] → 4 MB F32, 1 MB int8, 0.5 MB int4
- up_proj: [1024, 4096] → 4 MB F32, 1 MB int8, 0.5 MB int4
- down_proj: [4096, 1024] → 16 MB F32, 4 MB int8, 2 MB int4
- **One expert triple (int8): ~6 MB, (int4): ~3 MB**
- **512 experts × 60 layers × 6 MB = ~180 GB (int8) — won't fit in 65 GB RAM**

### safetensors Path Convention

Tensors live in `/models/ornith-int8/model-00001-of-00122.safetensors` etc.
Expert paths: `model.layers.{layer}.mlp.experts.{idx}.gate_proj.weight`
Scale paths: same + `.qs`

### Key Constants

```c
#define MAX_KV_SLOTS 16
#define LA_STATE_DECAY 0.6f
#define LA_STATE_UPDATE 0.4f
#define LA_CONV1D_WIDTH 4
```

### Function Signatures to Match

```c
static void *qwen_malloc_impl(size_t size, const char *what);
static void *qwen_calloc_impl(size_t nmemb, size_t size, const char *what);
static void model_debug(const char *fmt, ...);
static void fail(const char *fmt, ...);
```

---

## Task 1: Add Memory Accounting Structs and Per-Category Tracking

**Objective:** Add a memory accounting struct that tracks RAM usage per category (embed/norm, dense weights, shared experts, routed experts, KV cache) and integrate it with the existing `reserve_ram()` / `g_ram_peak_bytes` system.

**Files:**
- Modify: `/home/leite/colibri/c/qwen35_moe.c`

**What to add:**

1. A new enum for memory categories:
```c
typedef enum {
    MEM_CAT_EMBED_NORM = 0,
    MEM_CAT_DENSE_WEIGHTS,
    MEM_CAT_SHARED_EXPERTS,
    MEM_CAT_ROUTED_EXPERTS,
    MEM_CAT_KV_CACHE,
    MEM_CAT_TEMPORARY,
    MEM_CAT_COUNT
} mem_category_t;
```

2. A global accounting struct (replaces bare `g_ram_peak_bytes`):
```c
static size_t g_mem_used[MEM_CAT_COUNT] = {0};
static size_t g_mem_limit = 0;  // same as g_ram_limit_bytes
```

3. Replace `reserve_ram()` to use per-category tracking:
   - `reserve_ram(bytes, category, what)` — increments `g_mem_used[category]`, checks total
   - `release_ram(bytes, category)` — decrements `g_mem_used[category]`
   - `get_mem_used(category)` — returns current usage
   - `get_mem_total_used()` — returns sum of all categories

4. Wire into existing allocation: `qwen_malloc_impl` and `qwen_calloc_impl` should track under `MEM_CAT_TEMPORARY` (this catches all existing allocations without changing their semantics).

**Verification:** After Task 1, `g_mem_used[MEM_CAT_TEMPORARY]` should reflect all allocations. No behavior change for existing code — just adds tracking.

---

## Task 2: Add Expert Eviction Policy (LRU with PINNED State)

**Objective:** Implement LRU-based expert eviction so that when RAM usage approaches the limit, the least-recently-used experts are freed and their state set to `UNLOADED`. The `PINNED` state is used for experts that should never be evicted (e.g., top-K most frequent).

**Files:**
- Modify: `/home/leite/colibri/c/qwen35_moe.c`

**What to add:**

1. Extend `qwen_expert_state_t` enum — add `QWEN_EXPERT_STATE_EVICTED` (loaded on disk, not in RAM):
```c
typedef enum {
    QWEN_EXPERT_STATE_UNLOADED = 0,
    QWEN_EXPERT_STATE_RESIDENT = 1,
    QWEN_EXPERT_STATE_PINNED = 2,
    QWEN_EXPERT_STATE_EVICTED = 3,
} qwen_expert_state_t;
```

2. Add an LRU tracking array per layer:
```c
// In QLayer struct or as a global:
typedef struct {
    int layer;
    int expert;
    int64_t last_used;  // monotonic counter, incremented on each access
} expert_lru_entry_t;

static expert_lru_entry_t *g_expert_lru = NULL;
static int g_expert_lru_size = 0;
static int64_t g_lru_counter = 0;
```

3. Add `expert_lru_touch(layer, expert)` — called whenever an expert is accessed (loaded or used in matmul). Updates `last_used` to current counter.

4. Add `expert_evict_lru(model, max_resident_experts)` — finds the UNLOADED/RESIDENT expert with the lowest `last_used`, frees its 3 QTensors, sets state to EVICTED, decrements `g_mem_used[MEM_CAT_ROUTED_EXPERTS]`.

5. Add `ensure_expert_with_eviction(model, layer, expert_idx)` — wraps `ensure_expert()`:
   - If already RESIDENT/PINNED: touch LRU, return
   - If EVICTED: reload from disk (same path as UNLOADED), set RESIDENT, touch LRU
   - If UNLOADED: load, set RESIDENT, touch LRU
   - Before loading: if `g_mem_used[MEM_CAT_ROUTED_EXPERTS]` exceeds threshold (e.g., 80% of limit), call `expert_evict_lru()` until under threshold or only PINNED experts remain

6. In `init_model()`, allocate `g_expert_lru` with size `num_layers * num_experts`, initialize all `last_used = 0`.

7. In `free_model()`, free `g_expert_lru`.

**Verification:** After Task 2, calling `ensure_expert_with_eviction()` for many experts should cause older ones to be evicted when RAM fills up. PINNED experts are never evicted.

---

## Task 3: Add Disk-Based Expert Streaming Index

**Objective:** Create a mechanism to track which expert tensors are on disk and load them on demand. This is the foundation for streaming — experts that aren't resident are loaded from the safetensors shards when needed.

**Files:**
- Modify: `/home/leite/colibri/c/qwen35_moe.c`

**What to add:**

1. An expert tensor path cache (parallel to the safetensors shard reader):
```c
typedef struct {
    char layer_path[512];   // e.g., "model.layers.0.mlp.experts.0.gate_proj.weight"
    int layer;
    int expert_idx;
    int tensor_type;        // 0=gate, 1=up, 2=down
    int64_t shard_offset;   // byte offset in the safetensors file
    int64_t tensor_bytes;   // size of the tensor payload
    int dtype;              // safetensors dtype
} expert_tensor_ref_t;
```

2. A function to build the expert tensor index from the safetensors index file:
```c
static int build_expert_index(qwen35_model *m);
```
- Reads `model.safetensors.index.json` (or iterates shard headers)
- For each tensor matching `model.layers.{L}.mlp.experts.{E}.{gate|up|down}_proj.weight`:
  - Parse layer, expert_idx, tensor_type from the name
  - Store the reference in a flat array
- Returns 0 on success, -1 on failure

3. A function to load a single expert tensor by its index reference:
```c
static QTensor load_expert_tensor_by_ref(qwen35_model *m, const expert_tensor_ref_t *ref);
```
- Uses existing `st_read_raw()` / `st_read_f32()` from the shard reader
- Handles quantized tensors the same way as `load_qtensor()`

4. Wire `build_expert_index()` into `init_model()` — called after `st_init()`, before loading any layers. If the index file isn't available, fall back to name-based lookup (current behavior).

**Verification:** After Task 3, `build_expert_index()` populates the index. `load_expert_tensor_by_ref()` can load any expert tensor without knowing the shard filename.

---

## Task 4: Add RAM Limit Enforcement with Expert Eviction Trigger

**Objective:** Integrate the memory accounting and eviction policy so that the engine proactively evicts experts BEFORE hitting the RAM limit, rather than failing catastrophically.

**Files:**
- Modify: `/home/leite/colibri/c/qwen35_moe.c`

**What to add:**

1. A proactive eviction check function:
```c
static void check_and_evict_if_needed(qwen35_model *m) {
    size_t expert_usage = g_mem_used[MEM_CAT_ROUTED_EXPERTS];
    size_t threshold = g_mem_limit * 80 / 100;  // 80% of total RAM limit
    if (expert_usage < threshold) return;
    
    size_t target = g_mem_limit * 60 / 100;  // evict down to 60%
    while (expert_usage > target) {
        // Find least-recently-used non-pinned expert
        int64_t min_time = INT64_MAX;
        int victim_layer = -1, victim_expert = -1;
        for (int l = 0; l < m->num_layers; l++) {
            for (int e = 0; e < m->num_experts; e++) {
                if (m->layers[l].expert_state[e] == QWEN_EXPERT_STATE_RESIDENT) {
                    if (g_expert_lru[l * m->num_experts + e].last_used < min_time) {
                        min_time = g_expert_lru[l * m->num_experts + e].last_used;
                        victim_layer = l;
                        victim_expert = e;
                    }
                }
            }
        }
        if (victim_layer < 0) break;  // all evicted or all pinned
        evict_single_expert(m, victim_layer, victim_expert);
        // Recalculate expert_usage after eviction
        expert_usage = g_mem_used[MEM_CAT_ROUTED_EXPERTS];
    }
}
```

2. Call `check_and_evict_if_needed()` at the start of `run_model()` and after each layer's FFN block.

3. Add a `--evict-threshold` CLI flag (percentage, default 80):
```c
static int g_evict_threshold_pct = 80;
// In main(): parse --evict-threshold N
// In check_and_evict_if_needed(): use g_evict_threshold_pct
```

4. Add `--pin-experts` CLI flag (comma-separated "layer,expert" pairs):
```c
// In main(): parse --pin-experts "0,5;0,12;1,3"
// In init_model(): set expert_state[pinned] = PINNED
```

**Verification:** After Task 4, running with `--ram-limit-mb 32768 --evict-threshold 80` should cause experts to be evicted when RAM usage exceeds 26 GB, bringing it back down to ~20 GB.

---

## Task 5: Add Layer Type Validation and Robust Parsing

**Objective:** Make the layer_types parsing robust: validate length, handle missing/short arrays, log each layer's type, and support future layer types gracefully.

**Files:**
- Modify: `/home/leite/colibri/c/qwen35_moe.c`

**What to change:**

1. In `init_model()`, replace the current layer_types parsing block (lines ~1287-1299) with:
```c
jval *layer_types = json_get(text_cfg, "layer_types");
int layer_types_count = 0;
if (layer_types && layer_types->t == J_ARR) {
    layer_types_count = layer_types->len;
}

if (layer_types_count != m->num_layers) {
    model_debug("layer_types: got %d entries for %d layers (expected %d)",
                layer_types_count, m->num_layers, m->num_layers);
    // If shorter, pad with FULL type
    // If longer, truncate
}

for (int layer = 0; layer < m->num_layers; layer++) {
    int type = LAYER_TYPE_FULL;
    if (layer_types && layer < layer_types->len &&
        layer_types->kids[layer] && layer_types->kids[layer]->t == J_STR) {
        if (strcmp(layer_types->kids[layer]->str, "linear_attention") == 0) {
            type = LAYER_TYPE_LINEAR;
        } else if (strcmp(layer_types->kids[layer]->str, "hybrid") == 0) {
            // TODO: support hybrid layers (full + linear attention combined)
            type = LAYER_TYPE_FULL;
            model_debug("layer %d: hybrid type not yet supported, treating as FULL", layer);
        }
        // Unknown types: log warning, default to FULL
        else if (strcmp(layer_types->kids[layer]->str, "full_attention") != 0) {
            model_debug("layer %d: unknown type '%s', defaulting to FULL",
                        layer, layer_types->kids[layer]->str);
        }
    }
    m->layer_types[layer] = type;
    model_debug("layer %d: type=%s", layer, type == LAYER_TYPE_LINEAR ? "LINEAR" : "FULL");
}
```

2. Add `LAYER_TYPE_HYBRID` to the enum (as a placeholder for future use):
```c
typedef enum {
    LAYER_TYPE_LINEAR = 0,
    LAYER_TYPE_FULL = 1,
    LAYER_TYPE_HYBRID = 2,  // TODO: full + linear attention combined
} layer_type_t;
```

**Verification:** After Task 5, the engine logs each layer's type at startup. Unknown types get a debug warning but don't crash. Short layer_types arrays are padded with FULL.

---

## Task 6: Add Debug Logging for Expert Loading/Eviction

**Objective:** Add comprehensive debug logging so we can observe expert loading, eviction, and memory usage during a run. This is critical for tuning the eviction policy.

**Files:**
- Modify: `/home/leite/colibri/c/qwen35_moe.c`

**What to add:**

1. In `ensure_expert()` (or `ensure_expert_with_eviction()`), add logging:
```c
model_debug("expert %d.%d: state=%d -> loading from disk", layer, expert_idx, cur->expert_state[expert_idx]);
// After loading:
model_debug("expert %d.%d: loaded (gate=%d bytes, up=%d bytes, down=%d bytes)",
            layer, expert_idx,
            cur->expert_gate_proj[expert_idx].O * cur->expert_gate_proj[expert_idx].I * qt_elem_size(cur->expert_gate_proj[expert_idx].fmt),
            cur->expert_up_proj[expert_idx].O * cur->expert_up_proj[expert_idx].I * qt_elem_size(cur->expert_up_proj[expert_idx].fmt),
            cur->expert_down_proj[expert_idx].O * cur->expert_down_proj[expert_idx].I * qt_elem_size(cur->expert_down_proj[expert_idx].fmt));
```

2. Add `qt_elem_size(fmt)` helper:
```c
static int qt_elem_size(int fmt) {
    switch (fmt) {
        case 0: return 4;  // F32
        case 1: return 1;  // INT8
        case 2: return 0;  // INT4 (fractional, handled specially)
        default: return 4;
    }
}
```

3. In `expert_evict_lru()` / eviction path, add logging:
```c
model_debug("evicting expert %d.%d (last_used=%ld, state was RESIDENT)", layer, expert, (long)old_last_used);
model_debug("expert %d.%d: freed (reclaimed ~%zu bytes)", layer, expert, bytes_freed);
```

4. Add periodic memory summary logging (every 10 layers or at start/end of run_model):
```c
if (step % 10 == 0 || step == 0 || step == steps - 1) {
    model_debug("memory: total=%zu/%zu embed=%zu dense=%zu shared=%zu experts=%zu kv=%zu temp=%zu",
                get_mem_total_used(), g_mem_limit,
                g_mem_used[MEM_CAT_EMBED_NORM], g_mem_used[MEM_CAT_DENSE_WEIGHTS],
                g_mem_used[MEM_CAT_SHARED_EXPERTS], g_mem_used[MEM_CAT_ROUTED_EXPERTS],
                g_mem_used[MEM_CAT_KV_CACHE], g_mem_used[MEM_CAT_TEMPORARY]);
}
```

**Verification:** Run with `--debug` and check that expert loading/eviction events are logged with sizes and timestamps.

---

## Task 7: Wire Everything Together in run_model

**Objective:** Integrate all previous tasks into the main inference loop. Ensure experts are loaded on-demand with eviction, memory is tracked, and the existing behavior is preserved for non-expert paths.

**Files:**
- Modify: `/home/leite/colibri/c/qwen35_moe.c`

**What to change:**

1. In `run_model()`, at the top, call `check_and_evict_if_needed()` once before the main loop.

2. In the FFN block (where `ensure_expert()` is called), replace with `ensure_expert_with_eviction()`:
```c
// Old: ensure_expert(m, cur, layer, expert_idx);
// New:
ensure_expert_with_eviction(m, cur, layer, expert_idx);
```

3. In the same FFN block, after using an expert (the matmul calls), call `expert_lru_touch(layer, expert_idx)` to update the LRU timestamp.

4. Add a memory summary at the end of `run_model()`:
```c
model_debug("run_model complete: total memory used=%zu/%zu",
            get_mem_total_used(), g_mem_limit);
```

5. In `free_model()`, ensure `g_expert_lru` is freed and all expert tensors are released.

**Verification:** Full end-to-end test. Run the engine with a small model or tiny subset, verify experts load on demand, evict when needed, and the model produces output.

---

## Execution Order

Tasks 1 → 2 → 3 → 4 → 5 → 6 → 7 (strictly sequential — each builds on the previous).

## Rollback Strategy

Each task is additive (new functions, new structs, new call sites). If a task fails, revert only that task's changes by reverting the specific function additions and call site modifications. The file is version-controlled in git.

## Testing

Since this is C code that compiles into a binary, "testing" means:
1. `cd /home/leite/colibri/c && make qwen35_moe` — must compile clean
2. Run with `--debug` flag to verify logging
3. Run with a small model to verify no crashes
4. Run with Ornith int8 model (if RAM permits) to verify expert loading/eviction

For CI/stricter validation: build inside the ROCm toolbox container:
```bash
podman run --rm -v /home/leite/colibri/c:/work:Z -w /work \
  docker.io/kyuz0/amd-strix-halo-toolboxes:rocm-7.2.4 \
  bash -c "make ROCM_HOME=/opt/rocm ROCM=1 NPU=1 qwen35_moe 2>&1"
```
