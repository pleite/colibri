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

## What is not built, and why

The AIE int8 MAC has a row granularity of 8, and the upstream designs require
the M tile to be a multiple of it. The decode row tile (`rows = 1`) therefore
has no valid tiling, and `build_shape.py` reports it as `unsupported` rather
than padding: padding would feed the array uninitialised activation rows, and
the accumulator for the real row would be correct only by luck of what the
buffer happened to contain.

So a full-set build is partial by construction, and CI passes `--allow-partial`.
The consequence is visible and safe: no artifact exists for those shapes, the
runtime refuses them with `-ENOENT`, and the placement policy keeps decode off
the NPU. That is the documented "no silent fallback" behaviour.

## Known gaps

These are real, and they are why a successful build is not yet a working
inference path:

- **Partition configuration.** The DRM dispatch path in
  `c/npu_kernels/xdna2_driver.c` creates a hardware context but never registers
  the compiled xclbin's PDI with the firmware. The xclbin is copied next to each
  artifact so that step has something to consume once it is implemented.
- **ERT payload layout.** The command payload the runtime writes
  (`xdna2_matmul.c`) was written against the documented field order, not against
  XRT's `ert.h` structures, and truncates buffer addresses to 32 bits. It needs
  to be rebuilt on the real header before a dispatch can be trusted.
- **rows = 1.** See above.

Until the first two are closed, the artifacts this directory produces are
verified for *shape and container correctness* only — `pack_npukernel.py
--verify` and the CI step that runs it check exactly that, and claim nothing
more.
