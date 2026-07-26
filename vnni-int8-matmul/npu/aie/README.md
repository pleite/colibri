# XDNA 2 kernel toolchain

This directory builds the `.npukernel` artifacts the NPU backend loads at
runtime. Everything here is *build time*: no code in this directory runs on the
NPU, and none of it is linked into Colibri.

## Why a container

Compiling for the AIE array needs a toolchain that is not packaged anywhere:
a compiler that targets the AIE2P cores, the IRON/`mlir-aie` Python frontend,
and XRT's `xclbinutil`. Installing that on a developer machine is a long, easy
to get subtly wrong sequence, and the exact versions decide whether the emitted
instruction stream is valid for Strix Halo's NPU. So the toolchain is an image
built from `Containerfile.aie-toolchain`, pinned by `toolchain.lock`, and the
build runs the same way on a laptop and on the CI runner.

The idea of driving the open toolchain (rather than AMD's proprietary
`aiecompiler`, which has no public release for XDNA 2) is taken from
[atassis/xdna-engine](https://github.com/atassis/xdna-engine), which runs
IRON + Peano on Strix Halo. That project builds on bare metal; the container
here is the packaging that makes the same route reproducible.

## Layout

| file | role |
| --- | --- |
| `toolchain.lock` | every pinned version, one `KEY=VALUE` per line; the only place to bump them |
| `Containerfile.aie-toolchain` | the image: Peano, `mlir_aie`, XRT, and the build-time assertions that each is usable |
| `ert_opcode_probe.c` | prints the ERT opcodes read from XRT's `ert.h`, so the packer never guesses one |
| `build_shape.py` | plans a tiling for one shape and compiles the upstream `mlir-aie` matmul design ahead of time |
| `pack_npukernel.py` | writes the `.npukernel` container; also `--self-test` and `--verify` |
| `build_all.py` | builds every enumerated shape, writes `manifest.json`, decides the exit code |

## Building

```sh
cd vnni-int8-matmul
make npu-kernels                 # image + every shape, into npu/kernels/
make npu-toolchain-image         # just the image
make npu-kernels-check           # host-only checks, no container
./scripts/build-npu-kernels.sh --allow-partial 256x4096x1024
```

`CONTAINER_IMAGE=ghcr.io/pleite/colibri-aie-toolchain:latest` skips the image
build and pulls the one CI publishes.

The shape set is never written down here: `build_all.py` reads it from
`tools/npu_shapes_list`, which links `sched/npu_shapes.c`. Adding a shape to the
scheduler is all it takes for the build to pick it up.

## Tiling constraints

`plan_tiling()` in `build_shape.py` searches for a tile that satisfies, in this
order, everything the toolchain and the hardware impose:

- **MAC dimensions.** `m % r`, `k % s`, `n % t` — read from
  `aie.iron.kernels.mm(...).mac_dims`, which for int8 in / int32 out is `8x8x8`
  on AIE2P (Strix Halo) and `4x8x8` on AIE2 (Phoenix). IRON resolves that
  geometry from the *current device* and quietly falls back to AIE2 when none is
  set, so `build_shape.py` sets the device (`npu2`) before asking — the same
  constant it passes to the design as `--dev`. Reading the AIE2 geometry while
  compiling for AIE2P is what produced `assert m % r == 0` inside the design.
- **L1 budget.** `A + B + C` for one core kept inside 32 KiB of the 64 KiB
  compute-tile memory, leaving room for ObjectFifo double buffering.
- **Design row blocking.** `whole_array` drains C in transfer blocks of two
  `m * 4`-row blocks, so `rows % (8 * m) == 0`; `single_core` drains C in groups
  of two tile rows, so `rows % (2 * m) == 0`. Getting this wrong surfaces inside
  the design as `TensorTiler2D.group_tiler ... does not divide evenly into tile
  groups in dimension 0`.
- **Shim ND-DMA stride.** The outer stride of the C drain is `rows_per_block *
  out` int32 words and the descriptor field is 20 bits, so it may not exceed
  2^20. `256x4096x16384` is the shape this binds: `m = 32` needs a stride of
  2097152 words and is rejected by `mlir-aie`'s verifier, so the planner drops
  to `m = 16` (stride exactly 2^20).
- **Shim ND-DMA iteration count.** The same descriptor holds the outermost wrap
  in a 6 bit field, and A is re-streamed once per column block of B, so
  `out / n` (`out / n / cols` for the whole array) may not exceed 64. This is
  what rules the single-core design out for `out = 16384` at `n = 64`: 256
  repeats, rejected as `Size 3 exceeds the [1:64] range`. A shape that no
  ordinary tile satisfies is retried with wider output tiles (`n` up to 256),
  which is the only way `32x4096x16384` fits; the retry runs only after the
  ordinary search found nothing, so it can never re-plan a shape that already
  builds.
- **Even core occupancy.** `(rows / m) * (out / n)` must divide by the number of
  cores, so no core in the array is left without an output tile.

With `r = 8` the row constraint decides which design each row tile lands on:

| row tile | design | why |
| --- | --- | --- |
| 256 | `whole_array`, 8 columns, 32 cores | `256 % (8 * m) == 0` for `m = 32` (`m = 16` for `out = 16384`, where the C-drain stride binds) |
| 32 | `single_core`, 1 core | the whole array needs `rows % (8 * m) == 0` with `m` a multiple of 8, i.e. at least 64 rows; the single-core design needs only `rows % (2 * m) == 0`, i.e. 16. `m = 16, k = 64, n = 64`, except `out = 16384`, which needs the wide `n = 256` tile (`k = 32` to stay inside L1) |
| 1 | none | see below |

## What is not built, and why

The AIE2P int8 MAC has a row granularity of 8 (`r = 8`), and on top of it each
design needs whole row blocks: 8 tile rows for the whole-array design, 2 for the
single-core one. The smallest row count either design can express is therefore
16, and the smallest the whole array can express is 64. The decode row tile
(`rows = 1`) has no valid tiling, and `build_shape.py` reports it as
`unsupported` rather than padding: padding would feed the array uninitialised
activation rows, and the accumulator for the real row would be correct only by
luck of what the buffer happened to contain.

That is a property of the microkernel, not of this planner: no choice of `m`,
`k`, `n`, design or column count makes a 1-row int8 matmul expressible. Only a
kernel that accepts a row count below the MAC granularity — which the upstream
designs do not have — would change it.

So a full-set build is partial by construction, and CI passes `--allow-partial`.
The consequence is visible and safe: no artifact exists for those shapes, the
runtime refuses them with `-ENOENT`, and the placement policy keeps decode off
the NPU. That is the documented "no silent fallback" behaviour.

## Known gaps

- **rows = 1.** See above.

The artifacts this directory produces are still verified in CI for *shape and
container correctness* (`pack_npukernel.py --verify`), and runtime execution
must still be validated on Strix Halo hardware.
