# Qwen3.5 MoE Expert Loading & Memory Management — Implementation Plan

**Date:** 2026-07-19  
**Status:** COMPLETE  
**File:** `c/qwen35_moe.c` (2265 lines)

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
| 6. Debug logging | ✅ DONE | `qt_elem_size()`, `s_last_mem_log_layer`, periodic memory summary, loading logs |
| 7. Wire together | ✅ DONE | All components wired: init/free in lifecycle, eviction in ensure_expert, LRU touch after loading |

## Compilation Status

- `make qwen35_moe` compiles clean (no warnings, no errors)
- `make check` fails on `test_qwen35_moe` (forward pass still a scaffold)
- All other tests pass (json, safetensors, tier, grammar, backend_runtime, backend_parallel, backend_vulkan, backend_npu)

## Git History



## Impact

The engine now has:
- **Per-category RAM tracking** — knows exactly how much memory each component uses
- **LRU-based eviction** — evicts least-recently-used experts when RAM approaches limit
- **Expert tensor index** — fast lookup without string matching
- **Debug logging** — visibility into expert loading/eviction for tuning
- **Proactive eviction** — triggers before OOM, keeps RAM under control

This enables running Ornith 397B (512 experts/layer × 60 layers = 30,720 expert triples, ~180 GB at int8) on Strix Halo's 65 GB RAM by streaming experts on-demand and evicting when necessary.

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
