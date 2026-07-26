#!/usr/bin/env python3
"""build_all.py — compile and pack every enumerated XDNA 2 kernel artifact.

Runs inside the AIE toolchain image (npu/aie/Containerfile.aie-toolchain);
``scripts/build-npu-kernels.sh`` is what puts it there.

The shape set is read from ``tools/npu_shapes_list``, which prints what
``sched/npu_shapes.c`` enumerates, so the build and the runtime cannot disagree
about which artifacts should exist.

For each shape:

1. ``build_shape.py`` picks an upstream mlir-aie design and a tiling and
   compiles it ahead of time into ``final.xclbin`` + ``insts.bin``;
2. ``pack_npukernel.py`` packs the instruction stream, the ERT opcode read from
   XRT's ``ert.h`` and the CU mask read from the xclbin into
   ``<kernel-dir>/matmul_int8_<rows>x<inner>x<out>.npukernel``;
3. the xclbin is copied next to it, because the artifact is only reproducible
   together with the partition configuration it was compiled against.

A ``manifest.json`` records every shape with its status, so a partial build is
visible rather than implied by a missing file. Statuses are:

``built``        the artifact exists and was packed;
``failed``       the toolchain tried and could not produce it — always fatal;
``unsupported``  no tiling of the upstream designs expresses this shape — fatal
                 unless ``--allow-partial``, because silently shipping fewer
                 kernels than the model needs is how an NPU path turns into a
                 permanent fallback to another backend.
"""

from __future__ import annotations

import argparse
import json
import shutil
import subprocess
import sys
from pathlib import Path

HERE = Path(__file__).resolve().parent
VNNI_ROOT = HERE.parent.parent


def shape_set(shapes_tool: Path) -> list[dict]:
    if not shapes_tool.is_file():
        raise SystemExit(
            f"{shapes_tool} is missing; build it with `make tools/npu_shapes_list`"
        )
    out = subprocess.run(
        [str(shapes_tool), "--json"], check=True, capture_output=True, text=True
    ).stdout
    return json.loads(out)


def select(shapes: list[dict], wanted: str) -> list[dict]:
    tokens = [t for t in wanted.replace(",", " ").split() if t]
    if not tokens:
        return shapes
    keys = set()
    for token in tokens:
        try:
            rows, inner, out = (int(v) for v in token.lower().split("x"))
        except ValueError as exc:
            raise SystemExit(f"cannot parse shape '{token}'; use rowsXinnerXout") from exc
        keys.add((rows, inner, out))
    picked = [s for s in shapes if (s["rows"], s["inner"], s["out"]) in keys]
    missing = keys - {(s["rows"], s["inner"], s["out"]) for s in picked}
    if missing:
        raise SystemExit(
            "not members of the enumerated shape set: "
            + ", ".join(f"{r}x{i}x{o}" for r, i, o in sorted(missing))
        )
    return picked


def build_one(shape: dict, build_root: Path, kernel_dir: Path) -> dict:
    tag = f"{shape['rows']}x{shape['inner']}x{shape['out']}"
    build_dir = build_root / tag

    compile_cmd = [
        sys.executable,
        str(HERE / "build_shape.py"),
        "--rows",
        str(shape["rows"]),
        "--inner",
        str(shape["inner"]),
        "--out",
        str(shape["out"]),
        "--build-dir",
        str(build_dir),
    ]
    compiled = subprocess.run(compile_cmd, capture_output=True, text=True)
    sys.stderr.write(compiled.stderr)
    try:
        record = json.loads(compiled.stdout.strip().splitlines()[-1])
    except (ValueError, IndexError):
        return {
            **shape,
            "status": "failed",
            "reason": f"build_shape.py produced no record (exit {compiled.returncode})",
        }
    record.update({"artifact": shape["artifact"], "role": shape["role"]})
    if record.get("status") != "built":
        return record

    pack_cmd = [
        sys.executable,
        str(HERE / "pack_npukernel.py"),
        "--rows",
        str(shape["rows"]),
        "--inner",
        str(shape["inner"]),
        "--out",
        str(shape["out"]),
        "--insts",
        record["insts"],
        "--xclbin",
        record["xclbin"],
        "--output",
        str(kernel_dir / shape["artifact"]),
    ]
    packed = subprocess.run(pack_cmd, capture_output=True, text=True)
    sys.stderr.write(packed.stderr)
    if packed.returncode != 0:
        record["status"] = "failed"
        record["reason"] = f"packing failed (exit {packed.returncode})"
        return record

    record["packed"] = json.loads(packed.stdout.strip().splitlines()[-1])
    xclbin_copy = kernel_dir / (Path(shape["artifact"]).stem + ".xclbin")
    shutil.copyfile(record["xclbin"], xclbin_copy)
    record["xclbin_artifact"] = str(xclbin_copy)
    return record


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--kernel-dir", type=Path, default=Path("npu/kernels"))
    p.add_argument("--build-root", type=Path, default=Path("npu/aie/build"))
    p.add_argument(
        "--shapes",
        default="",
        help="restrict to these shapes, e.g. '256x4096x1024 32x4096x512'",
    )
    p.add_argument("--allow-partial", action="store_true")
    p.add_argument(
        "--shapes-tool",
        type=Path,
        default=VNNI_ROOT / "tools" / "npu_shapes_list",
    )
    opts = p.parse_args()

    shapes = select(shape_set(opts.shapes_tool), opts.shapes)
    opts.kernel_dir.mkdir(parents=True, exist_ok=True)
    opts.build_root.mkdir(parents=True, exist_ok=True)

    records = []
    for shape in shapes:
        tag = f"{shape['rows']}x{shape['inner']}x{shape['out']}"
        print(f"=== {tag} ({shape['role']})", flush=True)
        record = build_one(shape, opts.build_root, opts.kernel_dir)
        status = record.get("status", "failed")
        print(f"--- {tag}: {status} {record.get('reason', '')}".rstrip(), flush=True)
        records.append(record)

    manifest = {
        "kernel_dir": str(opts.kernel_dir),
        "shapes": records,
        "built": sum(r.get("status") == "built" for r in records),
        "failed": sum(r.get("status") == "failed" for r in records),
        "unsupported": sum(r.get("status") == "unsupported" for r in records),
        "total": len(records),
    }
    manifest_path = opts.kernel_dir / "manifest.json"
    manifest_path.write_text(json.dumps(manifest, indent=2) + "\n")

    print(
        f"\n{manifest['built']}/{manifest['total']} kernels built, "
        f"{manifest['failed']} failed, {manifest['unsupported']} unsupported "
        f"(manifest: {manifest_path})"
    )

    if manifest["failed"]:
        print("a shape the toolchain accepted failed to compile", file=sys.stderr)
        return 1
    if manifest["unsupported"] and not opts.allow_partial:
        for record in records:
            if record.get("status") == "unsupported":
                print(
                    f"unsupported: {record['rows']}x{record['inner']}x{record['out']}: "
                    f"{record.get('reason', '')}",
                    file=sys.stderr,
                )
        print(
            "re-run with --allow-partial to accept an incomplete kernel set; the "
            "runtime refuses the missing shapes with -ENOENT",
            file=sys.stderr,
        )
        return 2
    return 0


if __name__ == "__main__":
    sys.exit(main())
