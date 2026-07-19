# Qwen3.5 MoE Expert Loading & Memory Management — Implementation Plan

**Date:** 2026-07-19  
**Status:** In Progress  
**File:** `c/qwen35_moe.c` (2253 lines after Task 3)

## Goal

Enable the colibri engine to run Ornith 397B (512 experts/layer, 60 layers) on Strix Halo's 65 GB RAM by implementing on-demand expert loading with eviction, disk streaming, and memory accounting.

## Current State (Validated 2026-07-19)

| Task | Status | Details |
|------|--------|---------|
| 1. Memory accounting | ✅ DONE | `mem_category_t` enum, `g_mem_used[MEM_CAT_COUNT]`, `release_ram()`, `get_mem_used()`, `get_mem_total_used()` wired into `reserve_ram()` and malloc/calloc |
| 2. LRU eviction | ✅ DONE | `QWEN_EXPERT_STATE_EVICTED`, `expert_lru_entry_t`, `expert_lru_init/free/touch`, `evict_single_expert`, `expert_evict_lru_from_layer`, `qt_data_bytes`, eviction check in `ensure_expert()` |
| 3. Expert index | ✅ DONE | `expert_tensor_ref_t`, `g_expert_index`, `build_expert_index()`, `find_expert_tensor_ref()` |
| 4. RAM enforcement | ✅ DONE | Real eviction logic via Task 2 (replaces stub) |
| 5. Layer validation | ✅ DONE | `validate_layer_config()` added, called from `init_model()` |
| 6. Debug logging | ⚠️ PARTIAL | Eviction logs added in Task 2; missing: `qt_elem_size()`, loading logs, periodic memory summary |
| 7. Wire together | ✅ DONE | All components wired: init/free in lifecycle, eviction in ensure_expert, LRU touch after loading |

## Compilation Status

- `make qwen35_moe` compiles clean (no warnings, no errors)
- `make check` fails on `test_qwen35_moe` (forward pass still a scaffold)
- All other tests pass (json, safetensors, tier, grammar, backend_runtime, backend_parallel, backend_vulkan, backend_npu)

## Next Steps

### Priority 1: Complete Task 6 — Debug Logging (OPTIONAL)

This provides visibility into expert loading/eviction for tuning.

**What to implement:**
- Add `qt_elem_size()` helper for bytes per element by format
- Add debug logs in `ensure_expert()` for loading events
- Add periodic memory summary logging (every 10 layers)

**Verification:**
- Compile clean
- Run with `--debug` and verify logs show expert loading/eviction events

## Execution Summary

**Tasks 1-5, 7:** Completed in previous sessions (commits e85c1d1, 89d9338, 96f83e0)  
**Task 6:** Remaining — optional debug logging enhancement

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
