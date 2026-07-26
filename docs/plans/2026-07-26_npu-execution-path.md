# Plan A — from "kernels compile" to "the NPU actually computes"

*Written down, deliberately not executed.* Two of the six items (A4, A6) were
already implemented on the `copilot/implement-kernels-for-vnni-code` branch; the
rest are recorded here so the branch does not carry an unwritten plan. Nothing
below is scheduled — each item is a separate change with its own validation.

## Where this branch leaves things

`vnni-int8-matmul/npu/aie/` compiles `.npukernel` artifacts for every enumerated
shape that has a valid AIE2P tiling, and CI builds and verifies them. That is a
*build* result. It is not an inference result, because the dispatch path that
would consume those artifacts is incomplete, and the two `c/` backends that the
model engine actually calls do not talk to the hardware at all.

| item | what it closes | state |
| --- | --- | --- |
| A1 | xclbin PDI partition never registered with the firmware | open |
| A2 | ERT command payload written against docs, not XRT's `ert.h` | open |
| A3 | `c/backend_npu.c` / `c/backend_vulkan.c` compute on the host | open |
| A4 | placement policy unanswerable at runtime | done (`0b15d00`) |
| A5 | two independent, unreconciled placement policies | open |
| A6 | docs asserting behaviour the code did not have | done (`0b15d00`) |

## A1 — Register the compiled xclbin's PDI before dispatch

`c/npu_kernels/xdna2_driver.c` creates a hardware context but never hands the
firmware the PDI partition contained in the xclbin the kernel was compiled from.
Until it does, a context exists and a dispatch has nothing valid to run on.

- The xclbin is already copied next to each `.npukernel`, so the input exists.
- Scope: parse the partition out of the xclbin, register it on the DRM path
  before `CREATE_HWCTX`, and fail loudly (never silently fall back) when the
  artifact and the registered partition disagree.
- Done when a compiled prefill shape produces int32 accumulators on hardware
  that match the VNNI CPU reference, on the self-hosted Strix Halo runner.

## A2 — Rebuild the ERT payload on XRT's real structures

`c/npu_kernels/xdna2_matmul.c` writes the command payload in the documented
field order and truncates buffer addresses to 32 bits. Both are guesses, and a
wrong payload is the failure mode that returns success and garbage.

- `npu/aie/ert_opcode_probe.c` already reads the opcode from `ert.h` at image
  build time; the layout should come from the same place rather than from prose.
- Scope: full 64-bit addresses, field order taken from the header, and a test
  that rejects a payload whose size or opcode does not match the probe output.
- A1 and A2 are the two gaps recorded in `npu/aie/README.md` under "Known gaps".
  Neither is closed by any amount of kernel building.

## A3 — Make the `c/` backends dispatch instead of emulating

`c/backend_npu.c` and `c/backend_vulkan.c` both compute with the scalar OpenMP
`matmul_host()`. The Vulkan one accelerates only if `COLI_VULKAN_KERNEL_LIB`
dlopens a plugin. So a run that reports "NPU" or "GPU" is measuring the CPU.

- Scope: route these backends to the real `vnni-int8-matmul` engines
  (`npu/xdna2_backend.c`, `gpu/vulkan_backend.c`), and where no kernel exists
  for the requested shape, refuse the op rather than emulating it.
- The host path stays, but as the *CPU* backend's implementation, not as a
  silent stand-in for an accelerator.
- Depends on A1 and A2 for the NPU half; the Vulkan half is independent.

## A4 — Placement decidable and explainable at runtime *(done)*

Implemented in `0b15d00`: device-probed residency (`sched/engine_caps.c`),
permanent-vs-temporary refusals (`decision.rejected_permanent[]`), no estimation
across a row-tile class boundary, a warm-weight discount bounded by the measured
`upload_ns`, and `tools/placement_report` to print all of it per shape.

## A5 — Reconcile the two placement policies

There are two, and they do not know about each other:

- `c/backend_runtime.c`'s static `g_role_affinity` table, which splits output
  rows across lanes by role, and
- `vnni-int8-matmul/sched/backend_placement.c`'s `coli_choose_backend()`, which
  ranks engines from a measured CSV table.

Nothing in `c/` references the `sched/` policy. The result is that the engine
the measured policy would refuse can still be handed rows by the role table.

- Scope: one policy. The measured one is the one to keep; the role affinities
  become an input to it (or are dropped), and `c/` calls `coli_choose_backend()`.
- Prerequisite: A3, since a policy that ranks engines is meaningless while every
  engine runs the same host code.
- Done when a shape refused by `coli_choose_backend()` cannot receive rows from
  any `c/` dispatch path, and `tools/placement_report` describes what the model
  engine will actually do.

## A6 — Documentation that matches the code *(done)*

Implemented in `0b15d00`: `docs/placement-policy.md`, `vnni-int8-matmul/README.md`,
`docs/ARCHITECTURE.md` and `data/README.md` corrected where they described
behaviour the code did not have.

## Not on this list

**`rows = 1` on the NPU.** The AIE2P int8 MAC has an 8-row granularity and the
smallest tiling either upstream design can express is 16 rows (64 on the whole
array), so decode is not expressible — no artifact, driver fix or heap size
changes that. It is a permanent refusal, recorded as such by A4, and it is not
work to be scheduled.

**Batched prefill in the model engine.** `c/qwen35_moe.c` drives every matmul one
token at a time (`matmul_qt()` passes `S = 1`), so no prefill row tile ever
reaches a backend and the NPU's only usable shapes are unreachable from the
engine. This is a real blocker for measuring A1–A3 end to end, but it is an
engine-side change of its own and belongs in its own plan, not here.
