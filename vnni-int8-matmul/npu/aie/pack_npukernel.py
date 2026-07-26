#!/usr/bin/env python3
"""pack_npukernel.py — pack a compiled AIE kernel into a `.npukernel` artifact.

The XDNA 2 runtime (``c/npu_kernels/xdna2_matmul.c``) loads a small
self-describing container so that the ERT opcode and the CU mask travel with the
DPU instruction stream they belong to. The layout is defined in
``c/npu_kernels/xdna2_matmul.h`` and documented in ``docs/strix-halo-npu.md`` §3::

    offset  size  field
    0       4     magic       'XDN2'
    4       4     version     1
    8       4     ert_opcode  ERT packet opcode for this kernel
    12      4     cu_mask     compute-unit mask
    16      4     rows
    20      4     inner_dim
    24      4     out_cols
    28      4     fmt         1 = INT8
    32      4     instr_size  instruction stream length in bytes
    36      4     instr_words words the DPU should execute
    40      ...   instruction stream

Neither of the two ABI values is invented here:

* ``ert_opcode`` comes from XRT's own ``ert.h`` through ``ert-opcode-probe``
  (npu/aie/ert_opcode_probe.c), which the toolchain image compiles and runs at
  image build time;
* ``cu_mask`` comes from the compiled xclbin's IP_LAYOUT section as reported by
  ``xclbinutil``.

Guessing either would produce artifacts that look valid and submit malformed
packets — guardrails 2 and 4 in ``docs/strix-halo-npu.md`` §6.
"""

from __future__ import annotations

import argparse
import json
import struct
import subprocess
import sys
import tempfile
from pathlib import Path

MAGIC = 0x324E4458  # 'XDN2', little-endian
VERSION = 1
HEADER_BYTES = 40
FMT_INT8 = 1

# The packet the amdxdna path uses. Named, not numbered: the value is read from
# XRT's header by the probe below.
ERT_OPCODE_NAME = "ERT_START_NPU"

# IP_LAYOUT entry types that denote a compute unit the ERT CU mask can address.
CU_IP_TYPES = frozenset({"IP_KERNEL", "IP_PS_KERNEL"})


def probe_ert_opcode(probe: str, name: str = ERT_OPCODE_NAME) -> int:
    """Read an ERT opcode out of XRT's ert.h via the compiled probe."""
    try:
        out = subprocess.run(
            [probe], check=True, capture_output=True, text=True
        ).stdout
    except FileNotFoundError as exc:
        raise SystemExit(
            f"{probe} not found: the ERT opcode can only come from XRT's ert.h. "
            "Run this inside the AIE toolchain image, which compiles the probe."
        ) from exc
    except subprocess.CalledProcessError as exc:
        raise SystemExit(f"{probe} failed: {exc.stderr.strip()}") from exc

    values = {}
    for line in out.splitlines():
        if "=" in line:
            key, _, value = line.partition("=")
            values[key.strip()] = int(value.strip())
    if name not in values:
        raise SystemExit(f"{probe} did not report {name}; got {sorted(values)}")
    return values[name]


def cu_mask_from_xclbin(xclbin: Path, xclbinutil: str = "xclbinutil") -> int:
    """Build the CU mask from the xclbin's IP_LAYOUT, one bit per kernel CU."""
    with tempfile.TemporaryDirectory() as tmp:
        dump = Path(tmp) / "ip_layout.json"
        try:
            subprocess.run(
                [
                    xclbinutil,
                    "--force",
                    "--input",
                    str(xclbin),
                    "--dump-section",
                    f"IP_LAYOUT:JSON:{dump}",
                ],
                check=True,
                capture_output=True,
                text=True,
            )
        except FileNotFoundError as exc:
            raise SystemExit(
                f"{xclbinutil} not found: the CU mask can only be read from the "
                "compiled xclbin. Run this inside the AIE toolchain image."
            ) from exc
        except subprocess.CalledProcessError as exc:
            raise SystemExit(
                f"{xclbinutil} could not dump IP_LAYOUT of {xclbin}: "
                f"{exc.stderr.strip()}"
            ) from exc
        layout = json.loads(dump.read_text())

    entries = layout.get("ip_layout", {}).get("m_ip_data", [])
    # aiecc registers the design through xclbinutil's --add-kernel with a
    # "ps-kernels" JSON, which XRT records as IP_PS_KERNEL (m_subtype DPU), not
    # IP_KERNEL. Both are compute units addressable by the ERT CU mask.
    cus = [
        e
        for e in entries
        if str(e.get("m_type", "")).upper() in CU_IP_TYPES
    ]
    if not cus:
        seen = sorted({str(e.get("m_type", "")) for e in entries})
        raise SystemExit(
            f"{xclbin} declares no compute unit of type "
            f"{'/'.join(sorted(CU_IP_TYPES))}; IP_LAYOUT holds "
            f"{seen or 'no entries'}"
        )
    if len(cus) > 32:
        raise SystemExit(
            f"{xclbin} declares {len(cus)} compute units; the ERT CU mask word "
            "addresses at most 32"
        )
    mask = 0
    for index in range(len(cus)):
        mask |= 1 << index
    return mask


def pack(
    rows: int,
    inner: int,
    out: int,
    instructions: bytes,
    ert_opcode: int,
    cu_mask: int,
    fmt: int = FMT_INT8,
) -> bytes:
    if not instructions:
        raise ValueError("empty instruction stream")
    if len(instructions) % 4 != 0:
        raise ValueError(
            f"instruction stream is {len(instructions)} bytes, not a whole "
            "number of 32-bit DPU words"
        )
    if rows <= 0 or inner <= 0 or out <= 0:
        raise ValueError("shape must be positive")

    header = struct.pack(
        "<10I",
        MAGIC,
        VERSION,
        ert_opcode,
        cu_mask,
        rows,
        inner,
        out,
        fmt,
        len(instructions),
        len(instructions) // 4,
    )
    assert len(header) == HEADER_BYTES
    return header + instructions


def unpack_header(blob: bytes) -> dict:
    """Parse a container header the way the C loader does, for verification."""
    if len(blob) < HEADER_BYTES:
        raise ValueError("artifact is truncated")
    fields = struct.unpack("<10I", blob[:HEADER_BYTES])
    keys = (
        "magic",
        "version",
        "ert_opcode",
        "cu_mask",
        "rows",
        "inner_dim",
        "out_cols",
        "fmt",
        "instr_size",
        "instr_words",
    )
    parsed = dict(zip(keys, fields))
    if parsed["magic"] != MAGIC:
        raise ValueError("bad magic")
    if parsed["version"] != VERSION:
        raise ValueError(f"unsupported version {parsed['version']}")
    if len(blob) != HEADER_BYTES + parsed["instr_size"]:
        raise ValueError("instruction stream length does not match the header")
    return parsed


def self_test() -> int:
    """Round-trip the container without any toolchain present."""
    instructions = bytes(range(0, 64))
    blob = pack(256, 4096, 1024, instructions, ert_opcode=17, cu_mask=0b11)
    parsed = unpack_header(blob)
    expected = {
        "magic": MAGIC,
        "version": VERSION,
        "ert_opcode": 17,
        "cu_mask": 0b11,
        "rows": 256,
        "inner_dim": 4096,
        "out_cols": 1024,
        "fmt": FMT_INT8,
        "instr_size": 64,
        "instr_words": 16,
    }
    if parsed != expected:
        print(f"self-test: header mismatch {parsed} != {expected}", file=sys.stderr)
        return 1
    if blob[HEADER_BYTES:] != instructions:
        print("self-test: instruction stream was not preserved", file=sys.stderr)
        return 1

    for bad, why in (
        (b"", "empty instruction stream"),
        (b"\x01\x02\x03", "instruction stream that is not a whole word count"),
    ):
        try:
            pack(1, 1, 1, bad, 0, 0)
        except ValueError:
            pass
        else:
            print(f"self-test: {why} was accepted", file=sys.stderr)
            return 1

    print("pack_npukernel self-test OK")
    return 0


def verify(paths: list[Path]) -> int:
    """Re-read artifacts the way the C loader does and check the file names."""
    failures = 0
    for path in paths:
        try:
            parsed = unpack_header(path.read_bytes())
        except (OSError, ValueError) as exc:
            print(f"{path}: INVALID ({exc})", file=sys.stderr)
            failures += 1
            continue
        expected_name = (
            f"matmul_int8_{parsed['rows']}x{parsed['inner_dim']}x"
            f"{parsed['out_cols']}.npukernel"
        )
        if path.name != expected_name:
            print(
                f"{path}: header declares {parsed['rows']}x{parsed['inner_dim']}x"
                f"{parsed['out_cols']}, which the loader would look up as "
                f"{expected_name}",
                file=sys.stderr,
            )
            failures += 1
            continue
        if parsed["fmt"] != FMT_INT8:
            print(f"{path}: fmt {parsed['fmt']} is not INT8", file=sys.stderr)
            failures += 1
            continue
        print(
            f"{path}: OK shape=({parsed['rows']},{parsed['inner_dim']},"
            f"{parsed['out_cols']}) opcode={parsed['ert_opcode']} "
            f"cu_mask=0x{parsed['cu_mask']:x} instr_words={parsed['instr_words']}"
        )
    if failures:
        print(f"{failures} artifact(s) rejected", file=sys.stderr)
        return 1
    if not paths:
        print("no artifacts given to verify", file=sys.stderr)
        return 1
    return 0


def main() -> int:
    p = argparse.ArgumentParser(description=__doc__)
    p.add_argument("--self-test", action="store_true")
    p.add_argument(
        "--verify",
        nargs="+",
        type=Path,
        metavar="ARTIFACT",
        help="parse existing artifacts instead of writing one",
    )
    p.add_argument("--rows", type=int)
    p.add_argument("--inner", type=int)
    p.add_argument("--out", type=int)
    p.add_argument("--insts", type=Path, help="insts.bin from the AIE toolchain")
    p.add_argument("--xclbin", type=Path, help="final.xclbin from the AIE toolchain")
    p.add_argument("--output", type=Path, help="artifact to write")
    p.add_argument(
        "--ert-opcode-probe",
        default="ert-opcode-probe",
        help="probe that prints the ERT opcodes from XRT's ert.h",
    )
    p.add_argument("--xclbinutil", default="xclbinutil")
    opts = p.parse_args()

    if opts.self_test:
        return self_test()
    if opts.verify:
        return verify(opts.verify)

    missing = [
        name
        for name, value in (
            ("--rows", opts.rows),
            ("--inner", opts.inner),
            ("--out", opts.out),
            ("--insts", opts.insts),
            ("--xclbin", opts.xclbin),
            ("--output", opts.output),
        )
        if value is None
    ]
    if missing:
        p.error("missing required arguments: " + ", ".join(missing))

    instructions = opts.insts.read_bytes()
    ert_opcode = probe_ert_opcode(opts.ert_opcode_probe)
    cu_mask = cu_mask_from_xclbin(opts.xclbin, opts.xclbinutil)

    blob = pack(opts.rows, opts.inner, opts.out, instructions, ert_opcode, cu_mask)
    unpack_header(blob)  # refuse to write something the C loader would reject
    opts.output.parent.mkdir(parents=True, exist_ok=True)
    opts.output.write_bytes(blob)

    print(
        json.dumps(
            {
                "artifact": str(opts.output),
                "rows": opts.rows,
                "inner": opts.inner,
                "out": opts.out,
                "fmt": FMT_INT8,
                "ert_opcode": ert_opcode,
                "ert_opcode_name": ERT_OPCODE_NAME,
                "cu_mask": cu_mask,
                "instr_size": len(instructions),
                "instr_words": len(instructions) // 4,
                "bytes": len(blob),
            }
        )
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
