#!/usr/bin/env python3
"""build_shape.py — compile one fixed-shape INT8 matmul kernel for XDNA 2.

The NPU backend dispatches ``C[rows][out] = A[rows][inner] . W[out][inner]^T``
with int8 operands. AIE-2 is fixed shape, so one artifact exists per
``(rows, inner, out)`` tuple of the enumerated set in ``sched/npu_shapes.c``.

This script does *not* reimplement a matmul design. It picks the upstream
mlir-aie design that can express the requested shape, works out a tiling that
satisfies that design's documented constraints and the AIE microkernel's MAC
dimensions, and runs the design's own ahead-of-time compile mode. The design and
the microkernels come from the pinned mlir-aie checkout in the toolchain image;
nothing about the AIE ABI is written out by hand here.

Mapping to the upstream designs
-------------------------------
Upstream computes ``C = A @ B`` with ``A`` (M, K) and ``B`` (K, N), or ``B``
(N, K) when ``--b-col-maj 1``. Weights in this tree are stored ``[out][inner]``,
so the mapping is::

    M = rows      A = activations   (rows,  inner)   int8
    K = inner     B = weights       (out,   inner)   int8, b_col_maj=1
    N = out       C = accumulators  (rows,  out)     int32

int8 x int8 accumulates into int32; the per-column f32 scale is applied by the
host after readback (see xdna2_matmul_int8_timed()).

Usage:
    build_shape.py --rows 256 --inner 4096 --out 1024 --build-dir build/...

Prints a single JSON object describing the build to stdout.
"""

from __future__ import annotations

import argparse
import json
import os
import shutil
import subprocess
import sys
from pathlib import Path

# The upstream designs live in the mlir-aie checkout the toolchain image pins.
MATMUL_EXAMPLES = "programming_examples/basic/matrix_multiplication"

# AIE-2 compute tile local memory is 64 KiB. The three live tiles (A, B and the
# int32 accumulator) have to fit alongside the ObjectFifo double buffering, so
# the search keeps them well inside it rather than discovering the limit as an
# aiecc failure halfway through a 15-shape build.
L1_TILE_BUDGET_BYTES = 32 * 1024

TILE_CANDIDATES = (64, 32, 16, 8, 4)
COLUMN_CANDIDATES = (8, 4, 2, 1)
N_AIE_ROWS = 4  # rows of compute tiles used by the whole-array design
TRANSFER_BLOCK_ROWS = 2  # whole_array's transfer-block granularity
SINGLE_CORE_ROW_BLOCK = 2  # single_core's C tile-group height (rows_per_block // 2)

# The shim ND-DMA encodes a stride in 32-bit words in a 20-bit field, so the
# outermost stride of a transfer may not exceed 2**20 words (mlir-aie verifies
# this in AIEX::verifyStridesWraps, "Stride 3 exceeds the [1:1048576] range").
# C is int32, so one accumulator element is exactly one word.
MAX_BD_STRIDE_WORDS = 1 << 20

# The same descriptor holds the outermost ("iteration") wrap in a 6-bit field,
# so no BD may repeat more than 64 times ("Size 3 exceeds the [1:64] range").
# Both designs stream A once per column block of B, which is the repeat count.
MAX_BD_ITERATIONS = 64


def mac_dims(m: int, k: int, n: int) -> tuple[int, int, int]:
    """MAC dimensions (r, s, t) of the int8 microkernel, from the toolchain."""
    import numpy as np
    from aie.iron import kernels

    kernel = kernels.mm(
        dim_m=m,
        dim_k=k,
        dim_n=n,
        input_dtype=np.int8,
        output_dtype=np.int32,
        b_col_maj=True,
    )
    r, s, t = kernel.mac_dims
    return int(r), int(s), int(t)


def l1_bytes(m: int, k: int, n: int) -> int:
    """Bytes the (A, B, C) tiles of one core occupy: int8 in, int32 out."""
    return m * k + k * n + 4 * m * n


def c_drain_stride_words(rows_per_transfer: int, out: int) -> int:
    """Outermost stride of the C drain, in 32-bit words.

    Both designs drain C by iterating over blocks of whole tile rows, so the
    outer stride is the number of C rows in one block times the row pitch. C is
    int32, hence one element per word.
    """
    return rows_per_transfer * out


def plan_tiling(rows: int, inner: int, out: int) -> dict | None:
    """Choose a design and tiling for this shape, or None when neither fits.

    Preference order is the whole-array design (32 compute tiles) over the
    single-core design, then the largest tile that satisfies every constraint:
    both maximise the work done per DMA transfer.
    """
    r, s, t = mac_dims(64, 64, 64)

    def tile_ok(m: int, k: int, n: int) -> bool:
        return (
            m % r == 0
            and k % s == 0
            and n % t == 0
            and l1_bytes(m, k, n) <= L1_TILE_BUDGET_BYTES
        )

    best: dict | None = None
    for m in TILE_CANDIDATES:
        for k in TILE_CANDIDATES:
            for n in TILE_CANDIDATES:
                if not tile_ok(m, k, n):
                    continue
                if inner % k != 0:
                    continue

                # whole_array: 4 rows of cores, transfer blocks of 2 row blocks.
                for cols in COLUMN_CANDIDATES:
                    if rows % (m * N_AIE_ROWS) != 0 or out % (n * cols) != 0:
                        continue
                    if (rows // m // N_AIE_ROWS) % TRANSFER_BLOCK_ROWS != 0:
                        continue
                    if (
                        c_drain_stride_words(m * N_AIE_ROWS, out)
                        > MAX_BD_STRIDE_WORDS
                    ):
                        continue
                    # A is re-streamed once per column block of B.
                    if out // n // cols > MAX_BD_ITERATIONS:
                        continue
                    # Every core must get the same number of output tiles.
                    if (rows // m) * (out // n) % (N_AIE_ROWS * cols) != 0:
                        continue
                    cand = {
                        "design": "whole_array",
                        "m": m,
                        "k": k,
                        "n": n,
                        "n_aie_cols": cols,
                        "cores": N_AIE_ROWS * cols,
                    }
                    if best is None or _score(cand) > _score(best):
                        best = cand

                # single_core: C is drained in groups of two tile rows.
                if rows % (m * SINGLE_CORE_ROW_BLOCK) == 0 and out % n == 0:
                    if (
                        c_drain_stride_words(m * SINGLE_CORE_ROW_BLOCK, out)
                        <= MAX_BD_STRIDE_WORDS
                        and out // n <= MAX_BD_ITERATIONS
                    ):
                        cand = {
                            "design": "single_core",
                            "m": m,
                            "k": k,
                            "n": n,
                            "n_aie_cols": 1,
                            "cores": 1,
                        }
                        if best is None or _score(cand) > _score(best):
                            best = cand
    return best


def _score(cand: dict) -> tuple[int, int, int]:
    return (cand["cores"], cand["m"] * cand["n"], cand["k"])


def design_command(
    plan: dict,
    rows: int,
    inner: int,
    out: int,
    xclbin: Path,
    insts: Path,
    mlir_aie_src: Path,
) -> list[str]:
    script = mlir_aie_src / MATMUL_EXAMPLES / plan["design"] / f"{plan['design']}.py"
    if not script.is_file():
        raise SystemExit(f"upstream design not found: {script}")
    cmd = [
        sys.executable,
        str(script),
        "--dev",
        "npu2",
        "-M",
        str(rows),
        "-K",
        str(inner),
        "-N",
        str(out),
        "-m",
        str(plan["m"]),
        "-k",
        str(plan["k"]),
        "-n",
        str(plan["n"]),
        "--b-col-maj",
        "1",
        "--dtype_in",
        "i8",
        "--dtype_out",
        "i32",
        "--xclbin-path",
        str(xclbin),
        "--insts-path",
        str(insts),
    ]
    if plan["design"] == "whole_array":
        cmd += ["--n-aie-cols", str(plan["n_aie_cols"])]
    return cmd


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--rows", type=int, required=True)
    p.add_argument("--inner", type=int, required=True)
    p.add_argument("--out", type=int, required=True)
    p.add_argument("--build-dir", type=Path, required=True)
    p.add_argument(
        "--mlir-aie-src",
        type=Path,
        default=Path(os.environ.get("MLIR_AIE_SRC", "/opt/mlir-aie")),
    )
    p.add_argument(
        "--plan-only",
        action="store_true",
        help="print the chosen design and tiling without compiling",
    )
    opts = p.parse_args()

    if not os.environ.get("PEANO_INSTALL_DIR"):
        raise SystemExit(
            "PEANO_INSTALL_DIR is unset: run this inside the AIE toolchain image "
            "(npu/aie/Containerfile.aie-toolchain), which sets it in "
            "/etc/profile.d/aie-toolchain.sh"
        )

    plan = plan_tiling(opts.rows, opts.inner, opts.out)
    if plan is None:
        r, s, t = mac_dims(64, 64, 64)
        # Not an error the build may paper over: an artifact for this shape
        # cannot be produced by this toolchain, and the NPU backend must go on
        # refusing the shape rather than run something compiled for another one.
        # Padding the row count host-side is not a way out either — a fixed-shape
        # kernel reads exactly the rows it was compiled for, so the padding rows
        # would be uninitialised activations.
        print(
            json.dumps(
                {
                    "rows": opts.rows,
                    "inner": opts.inner,
                    "out": opts.out,
                    "status": "unsupported",
                    "reason": (
                        "no tiling of the upstream int8 matmul designs divides "
                        f"({opts.rows}, {opts.inner}, {opts.out}): the AIE int8 "
                        f"MAC is {r}x{s}x{t}, so the row tile must be a multiple "
                        f"of {r}, and the row count a multiple of "
                        f"{r * SINGLE_CORE_ROW_BLOCK} (single core) or "
                        f"{r * N_AIE_ROWS * TRANSFER_BLOCK_ROWS} (whole array)"
                    ),
                }
            )
        )
        return 3

    build_dir = opts.build_dir.resolve()
    if build_dir.exists():
        shutil.rmtree(build_dir)
    build_dir.mkdir(parents=True)
    # The design runs with cwd=build_dir and resolves these paths itself, so
    # they have to be absolute: a relative path would land in a nested copy of
    # the build tree and the artifacts would look missing.
    xclbin = build_dir / "final.xclbin"
    insts = build_dir / "insts.bin"

    cmd = design_command(
        plan, opts.rows, opts.inner, opts.out, xclbin, insts, opts.mlir_aie_src
    )
    record = {
        "rows": opts.rows,
        "inner": opts.inner,
        "out": opts.out,
        "design": plan["design"],
        "tiling": {
            "m": plan["m"],
            "k": plan["k"],
            "n": plan["n"],
            "n_aie_cols": plan["n_aie_cols"],
            "cores": plan["cores"],
        },
        "dtype_in": "i8",
        "dtype_out": "i32",
        "b_col_maj": 1,
        "command": cmd,
    }

    if opts.plan_only:
        record["status"] = "planned"
        print(json.dumps(record))
        return 0

    print(f"[build_shape] {' '.join(cmd)}", file=sys.stderr)
    result = subprocess.run(cmd, cwd=build_dir)
    if result.returncode != 0:
        record["status"] = "failed"
        record["reason"] = f"design compile exited {result.returncode}"
        print(json.dumps(record))
        return 1
    if not xclbin.is_file() or not insts.is_file():
        record["status"] = "failed"
        record["reason"] = "design reported success but produced no xclbin/insts"
        print(json.dumps(record))
        return 1

    record["status"] = "built"
    record["xclbin"] = str(xclbin)
    record["insts"] = str(insts)
    record["insts_bytes"] = insts.stat().st_size
    print(json.dumps(record))
    return 0


if __name__ == "__main__":
    sys.exit(main())
