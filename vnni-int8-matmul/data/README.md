# Measured engine profile

This directory holds `strix_halo_profile.csv`: the per-shape, per-engine cost
table that `sched/backend_placement.c` reads to decide where a matmul runs.

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

Lines beginning with `#` are comments, and blank lines are ignored.

## Using it

```sh
COLI_PLACEMENT_PROFILE=/path/to/strix_halo_profile.csv ./tests/test_backends
```

See `docs/placement-policy.md` for how the table is turned into a decision, and
for the environment variables that override it.
