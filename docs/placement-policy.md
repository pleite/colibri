# Placement policy

Where does an int8 matmul run on Strix Halo — AVX-512 VNNI on the CPU, the
RADV GFX1151 iGPU, or the XDNA 2 NPU?

This document describes the single answer to that question,
`coli_choose_backend()` in `vnni-int8-matmul/sched/backend_placement.c`. There
is deliberately one such function: a placement rule scattered over three
backends is a rule nobody can audit.

## The shape of the decision

```
coli_engine_t coli_choose_backend(policy, caps, rows, inner, out, fmt, decision);
```

`rows` is the batch dimension `S`, `inner` is `I`, `out` is `O`, for
`y[rows][out] = x[rows][inner] · wᵀ`, weights `[out][inner]` int8.

`rows` is the interesting parameter. The same `(I, O)` pair behaves completely
differently at decode (`rows=1`), small prefill (`rows=8..64`) and bulk prefill
(`rows>=256`), because the fixed per-call cost of an accelerator — buffer
allocation, host→device sync, submit, syncobj wait, readback — is paid once per
call regardless of how much arithmetic the call contains. At `rows=1` on the NPU
that fixed cost is the whole story.

## 1. Hard constraints, first and unconditionally

These are not preferences and are not ranked against anything:

* **Availability.** An engine that did not initialise is never chosen.
* **An exact NPU kernel artifact.** AIE-2 kernels are compiled for one shape.
  The NPU is only a candidate when `caps.npu_kernel_exists(rows, inner, out,
  fmt)` says a `.npukernel` for *exactly* that shape is loaded. There is no
  widening to a larger compiled shape: that would dispatch a kernel whose tiling
  does not describe the operands, and the hardware will not tell you.
* **Residency.** When `caps.npu_resident_bytes` is known, an operand set larger
  than the NPU can hold removes the NPU from the candidates, leaving the iGPU —
  the engine with the memory to spare, on a machine where all three share DRAM.

A refusal is recorded per engine in `decision.rejected[]`, so the caller can
print *which* constraint removed an engine rather than "it did not get picked".

## 2. Measured ranking

With a table loaded (`data/strix_halo_profile.csv`, see
`vnni-int8-matmul/data/README.md`), the surviving engines are ranked by measured
cost and the cheapest wins.

Records are ranked in two tiers:

1. **Exact** — a record taken at this exact `(engine, rows, inner, out, fmt)`.
   `decision.source` is `measured`.
2. **Estimated** — no exact record, so the nearest measured shape for that
   engine is scaled: its `fixed_ns` is kept as-is (it does not scale with the
   array) and the remainder is scaled by the MAC ratio. `decision.source` is
   `estimated`.

If *any* surviving engine has an exact record, only exact records compete. An
extrapolation never outranks a measurement — extrapolating downwards from a
large-batch record makes an engine look arbitrarily cheap, precisely because
what it drops is the fixed cost.

## 3. Structural default, when there is no table

Absent any usable record the decision is an argument about data movement, not an
observation, and it is labelled `structural` so that it can never be mistaken
for a measurement. The argument is arithmetic reuse:

```
reuse = MACs / operand_bytes = (rows·inner·out) / (rows·inner + out·inner + rows·out)
```

* `reuse` below the threshold (`COLI_PLACEMENT_MIN_REUSE`, default 32) — the
  call is dominated by moving operands, not by multiplying them, so it goes to
  the **CPU**, which has no transfer to pay at all. This is where decode lands.
* At or above the threshold, an accelerator can amortise its fixed cost:
  the **NPU** when it passed the constraints above (a compiled kernel exists and
  the operands fit), otherwise the **GPU**.

An operator who considers this unacceptable can set
`COLI_PLACEMENT_REQUIRE_PROFILE=1`, which makes the same call return
`COLI_ENGINE_NONE` with a reason pointing at `bench/backend_bench` instead of
guessing.

## 4. Never a silent fallback

If the chosen engine cannot run the shape, the call fails and names the
constraint that was violated. In particular a pinned engine
(`COLI_PLACEMENT=npu`) that fails a constraint yields `COLI_ENGINE_NONE`; it is
never quietly replaced by a different engine. Silently computing on the CPU
after asking for the NPU produces a correct number and a false measurement, and
these backends exist to be measured.

The same rule applies one level up: `coli_moe_plan_build()` reports groups it
could not place in `plan.unplaced` rather than parking them somewhere.

## Environment variables

| Variable | Values | Effect |
| --- | --- | --- |
| `COLI_PLACEMENT` | `auto` (default), `cpu`, `gpu`, `npu` | Pin the engine. A pin that fails a hard constraint refuses, it does not substitute. |
| `COLI_PLACEMENT_PROFILE` | path | Measured table to load. Default `data/strix_halo_profile.csv`. |
| `COLI_PLACEMENT_REQUIRE_PROFILE` | `0` (default), `1` | Refuse any decision weaker than a measurement. |
| `COLI_PLACEMENT_MIN_REUSE` | integer, default `32` | Arithmetic-reuse threshold for the structural default. |
| `COLI_MOE_CPU_LANES` | integer, default `1` | Concurrent CPU expert groups. |
| `COLI_MOE_GPU_LANES` | integer, default `1` | Concurrent GPU expert groups. |

There is no `COLI_MOE_NPU_LANES`. The runtime creates one AIE partition, hence
one hardware context, and a second context on the same partition fails at
`CREATE_HWCTX`. Concurrent NPU work has to go through the single context as
multiple submissions, which is a separate feature, not a lane count.

## MoE: grouping comes before placement

For Qwen 3.6 MoE (512 experts, top-10) the placement question is only
answerable *after* the router output has been grouped by expert
(`sched/moe_schedule.c`). Dispatching per token gives every engine a `rows=1`
matmul, the one shape where an accelerator's fixed cost amortises over nothing.
Grouping turns the same work into a few contiguous `rows=N` matmuls, and then
each group gets its own placement decision because each group has its own row
count — which is the entire point:

* groups whose row count reaches a prefill tile go to the NPU,
* the long tail of 1–2 token groups stays on the VNNI CPU,
* large concurrent expert sets go to the iGPU, where dispatch is cheap.

Expert weights are re-used across tokens and steps while activations are not, so
`coli_moe_residency_*` keeps hot experts' weight buffers alive under an LRU
budget. Re-uploading a 4 MiB expert per group costs more than the matmul it
feeds; that upload policy matters more than the kernel's FLOPs.

## Hardware evidence

Everything above is falsifiable on the machine and inert everywhere else:

* `tests/test_placement` and `tests/test_moe_schedule` are pure host arithmetic
  and run anywhere — the decision logic is exactly the part that must be
  verifiable without the silicon.
* `tests/npu_device_test` and `bench/backend_bench` need Strix Halo and report a
  reason when they do not have it. They never fall back to the CPU.
* CI asserts that an NPU "OK" cannot appear without a loaded `.npukernel`.
