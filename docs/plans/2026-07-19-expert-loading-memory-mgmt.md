# Qwen3.5 MoE Expert Loading & Memory Management — Implementation Plan

**Date:** 2026-07-19  
**Status:** In Progress  
**File:** `c/qwen35_moe.c` (2162 lines after Task 2)

## Goal

Enable the colibri engine to run Ornith 397B (512 experts/layer, 60 layers) on Strix Halo's 65 GB RAM by implementing on-demand expert loading with eviction, disk streaming, and memory accounting.

## Current State (Validated 2026-07-19)

| Task | Status | Details |
|------|--------|---------|
| 1. Memory accounting | ✅ DONE | `mem_category_t` enum, `g_mem_used[MEM_CAT_COUNT]`, `release_ram()`, `get_mem_used()`, `get_mem_total_used()` wired into `reserve_ram()` and malloc/calloc |
| 2. LRU eviction | ✅ DONE | `QWEN_EXPERT_STATE_EVICTED`, `expert_lru_entry_t`, `expert_lru_init/free/touch`, `evict_single_expert`, `expert_evict_lru_from_layer`, `qt_data_bytes`, eviction check in `ensure_expert()` |
| 3. Expert index | ❌ NOT DONE | No `build_expert_index`, `g_expert_index`, or `expert_tensor_ref_t` in code |
| 4. RAM enforcement | ✅ DONE | `check_and_evict_if_needed()` stub replaced with real eviction logic via Task 2 |
| 5. Layer validation | ✅ DONE | `validate_layer_config()` added, called from `init_model()` |
| 6. Debug logging | ❌ NOT DONE | No `qt_elem_size()` helper or expert loading/eviction debug logs (partial: eviction logs added in Task 2) |
| 7. Wire together | ✅ DONE | All components wired: init/free in lifecycle, eviction in ensure_expert, LRU touch after loading |

## Compilation Status

- `make qwen35_moe` compiles clean (no warnings, no errors)
- `make check` fails on `test_qwen35_moe` (forward pass still a scaffold)
- All other tests pass (json, safetensors, tier, grammar, backend_runtime, backend_parallel, backend_vulkan, backend_npu)

## Next Steps (Priority Order)

### Priority 1: Complete Task 3 — Expert Index (USEFUL)

This provides a lookup table for fast expert tensor access without string matching.

**What to implement:**
- Add `expert_tensor_ref_t` struct with `name`, `layer`, `expert_idx`, `tensor_type`, `dtype`, `numel`, `nbytes`
- Add `g_expert_index` array, `g_expert_index_count`, `g_expert_index_cap`
- Add `build_expert_index()` that scans safetensors shards and builds index
- Add `find_expert_tensor_ref()` helper for O(1) lookup
- Wire into `init_model()` after `st_init()`
- Free in `free_model()`

**Verification:**
- Compile clean
- Run with `--debug` and verify index is built (logs "indexed N expert tensors")

### Priority 2: Complete Task 6 — Debug Logging (USEFUL)

This provides visibility into expert loading/eviction for tuning.

**What to implement:**
- Add `qt_elem_size()` helper for bytes per element by format
- Add debug logs in `ensure_expert()` for loading events (partial: eviction logs exist)
- Add periodic memory summary logging (every 10 layers)

**Verification:**
- Compile clean
- Run with `--debug` and verify logs show expert loading/eviction events

## Execution Strategy

**Delegate one task at a time.** Each task is additive (new functions, new structs, new call sites). If a task fails, revert only that task's changes.

**For each task:**
1. Read the current `c/qwen35_moe.c` to understand context
2. Apply changes via SSH + sed/heredoc
3. Compile with `make qwen35_moe` to verify clean build
4. Report diff and compilation result

## Rollback Strategy

Each task is additive. If a task fails, revert only that task's changes by reverting the specific function additions and call site modifications. The file is version-controlled in git.

## Testing

Since this is C code that compiles into a binary, "testing" means:
1. `cd /home/leite/colibri/c && make qwen35_moe` — must compile clean
2. Run with `--debug` flag to verify logging
3. Run with a small model to verify no crashes
4. Run with Ornith int8 model (if RAM permits) to verify expert loading/eviction

## Archive Notice

Older planning documents have been moved to `docs/plans/archive/`:
- `2026-07-15_copilot-ornith-feature-completeness.md`
- `2026-07-15_engine-capability-audit.md`
- `2026-07-16_backend-parity-gap-list.md`
- `2026-07-17_ci-framework-migration.md`
- `2026-07-17_ci-test-integration.md`
- `2026-07-17_llama-server-int4-analysis.md`
- `2026-07-17_rocm-package-fix.md`
- `2026-07-17_status_review.md`
- `2026-07-17_toolchain-setup.md`

This plan supersedes all of the above for the expert loading workstream.
