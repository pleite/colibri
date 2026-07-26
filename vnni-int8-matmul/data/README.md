# Measured engine profile

**The table does not exist yet.** This directory holds this README and nothing
else: no `strix_halo_profile.csv` has been produced on a Strix Halo host and
committed. Until one is, `coli_choose_backend()` reports
`source=structural` for 100% of decisions — an argument about data movement,
not an observation. `tools/placement_report` prints exactly that, per shape, for
whatever host it is run on.

This file describes the table that *will* live here: the per-shape, per-engine
cost table that `sched/backend_placement.c` reads to decide where a matmul
runs.

**The table is a measurement, not a configuration.** It is produced by
`bench/backend_bench` running on the Strix Halo machine, and it is committed
from the CI artifact of a run that had the silicon. Nothing in this repository
may write plausible-looking numbers into it: a fabricated row would make
`coli_choose_backend()` report `source=measured` for a decision nobody measured,
which is worse than having no table at all — that case is already handled, and
labelled `structural`.

There is deliberately no checked-in default file. With no table present the
placement policy falls back to the documented structural order and says so, and
an operator who wants to forbid that can set `COLI_PLACEMENT_REQUIRE_PROFILE=1`.

Note also that the NPU columns of any table produced today are not trustworthy:
the DRM dispatch path has not yet registered a PDI with firmware and its ERT
payload truncates buffer addresses to 32 bits (`npu/aie/README.md`, "Known
gaps"). CPU and GPU-f32 rows are the honest baseline until those close.

## Producing it

On the Strix Halo host:

```sh
make bench/backend_bench
./bench/backend_bench data/strix_halo_profile.csv
```

or through the container harness, which does this as part of the run:

```sh
make podman-test
```

The bench sweeps the Qwen 3.6 MoE projections (`sched/npu_shapes.c`) across
decode (`rows=1`), small prefill (`rows=8..64`) and bulk prefill
(`rows>=256`). Engines that are not present on the host emit a `# skip` comment
with the reason instead of a row, so a partial table is obvious rather than
silently biased.

## Format

A header line, then one record per `(backend, rows, inner, out, fmt)`:

```
backend,rows,inner,out,fmt,iters,total_ns,fixed_ns,alloc_ns,upload_ns,submit_ns,wait_ns,readback_ns,teardown_ns
```

* `backend` — `cpu`, `gpu` or `npu`.
* `total_ns` — mean wall time for one call, including everything below.
* `fixed_ns` — the part that does not scale with the array: buffer allocation
  and mapping, host→device sync, submit, wait, readback, teardown. This column
  is the point of the whole exercise. On the NPU at `rows=1` it dominates
  `total_ns`, and it is what makes per-token expert dispatch a losing trade.
* the remaining columns break `fixed_ns` down per stage.
* `-1` means "not measured separately on this engine" — only the CPU and GPU
  paths legitimately leave the breakdown out; a `-1` is never treated as zero.
* `upload_ns` does double duty: it is the stage breakdown, and it is what
  `coli_profile_upload_ns()` charges (or credits) when placement is told the
  weights are already resident on an engine. Every row is *cold* — the bench
  re-uploads its operands on each iteration — so an engine holding the weights
  is over-charged by exactly the weight share of this column, and no more. A row
  that leaves it at `-1` yields no residency credit at all.

Lines beginning with `#` are comments, and blank lines are ignored.

## Using it

```sh
COLI_PLACEMENT_PROFILE=/path/to/strix_halo_profile.csv ./tests/test_backends
```

To see what the current host would decide, with or without a table:

```sh
make tools/placement_report
./tools/placement_report          # cold weights
./tools/placement_report --warm   # weights already resident on every engine
```

See `docs/placement-policy.md` for how the table is turned into a decision, and
for the environment variables that override it.
