# Windows 11 port plan — colibrì

Tracks the native Windows 11 x86-64 port (MinGW-w64 / MSYS2).
All platform differences live in `c/compat.h`; the engine source (`glm.c`,
`olmoe.c`) is unchanged across platforms.

---

## Phase 1 — basic compat shims (✅ complete)

Goal: the engine compiles, produces correct output, and links statically with
zero DLL dependencies.

| shim | mapping | notes |
|---|---|---|
| `pread` | `compat_pread` (ReadFile + OVERLAPPED) | thread-safe; 64-bit offsets; no CRT text-mode corruption |
| `posix_fadvise` | no-op | advisory only; safe to ignore |
| `posix_memalign` | `_aligned_malloc` + `compat_aligned_free` | `free()` on aligned memory corrupts the NTFS heap |
| `rename` | `MoveFileEx(MOVEFILE_REPLACE_EXISTING)` | CRT `rename` fails with EEXIST if dest exists |
| `meminfo` | `GlobalMemoryStatusEx` | `ullAvailPhys` ≈ Linux `MemAvailable` |
| `getpid` | `_getpid` | |
| `getrusage(RUSAGE_SELF)` | `GetProcessMemoryInfo` / `PROCESS_MEMORY_COUNTERS_EX` | peak working set → `ru_maxrss` in KB |
| `getline` | `compat_getline` (fgets + realloc) | POSIX `getline` absent in MinGW CRT |
| `setenv` | `SetEnvironmentVariableA` | POSIX `setenv` absent in MinGW CRT |
| `O_BINARY` | belt-and-braces on all `open()` calls | prevents CRT 0x0A → CR/LF corruption |
| `_FILE_OFFSET_BITS=64` | enforced at compile time (`#error` if missing) | 32-bit `off_t` wraps >4 GB; silent weight-tensor corruption |

**Validation:** `make test-c` (C unit tests) + `audit_win_shims.c` (pread >4 GB
offset via sparse NTFS file, rename replace-existing).

**Build:**
```powershell
# from c/
make glm.exe
make olmoe.exe
make iobench.exe
make test-c
```

---

## Phase 2 — O_DIRECT via FILE_FLAG_NO_BUFFERING (✅ complete)

Goal: `DIRECT=1` bypasses the Windows page cache, mirroring Linux `O_DIRECT`
and macOS `F_NOCACHE`.  On NVMe drives this lifts the effective read ceiling
from the page-cache speed to the raw device bandwidth.

**Implementation:** `compat_open_direct()` in `c/compat.h` opens a second
("twin") file descriptor using `CreateFile` with `FILE_FLAG_NO_BUFFERING` and
converts the Win32 `HANDLE` to a CRT fd via `_open_osfhandle`.  The existing
`compat_pread` (ReadFile + OVERLAPPED) works unchanged on the no-buffering
handle.

Alignment constraints enforced by `FILE_FLAG_NO_BUFFERING` (offset, buffer,
and length must be multiples of the physical sector size — 4 096 bytes on
modern NVMe):
- `st.h` allocates slab buffers with `posix_memalign(4096)` ✓
- `glm.c` rounds the read offset to `&~4095LL` and the length to `(need+4095)&~4095LL` ✓
- `iobench.c` uses `posix_memalign(4096)` and `offs[i]=o&~4095L` ✓

**Usage:**
```powershell
DIRECT=1 SNAP=D:\glm52_i4 ./glm.exe 64 4 16
./iobench.exe D:\glm52_i4\out-00069.safetensors 19 64 8 1   # direct I/O benchmark
```

**Files changed:** `c/compat.h`, `c/st.h`, `c/iobench.c`.

---

## Phase G0 — GPU stub (already in Makefile)

`CUDA=1` on Windows triggers a compile-time error that directs users to Phase
G1.  No GPU code runs; the CPU path is unaffected.

---

## Phase G1 — coli_cuda.dll (planned)

Goal: build the CUDA expert tier as a DLL (`coli_cuda.dll`) with MSVC + nvcc,
loaded at runtime via `LoadLibrary` so the MinGW CPU binary keeps its
zero-dependency status.

Scope:
- `backend_cuda.cu / .h` compiled by nvcc → `coli_cuda.dll`
- thin `LoadLibrary` loader in `compat.h` replaces the direct CUDA symbol
  references that the Linux binary uses
- CPU binary links no CUDA symbols; `coli_cuda.dll` is an optional sidecar

Toolchain requirement: MSVC (for the DLL) + nvcc (CUDA Toolkit ≥ 12).  The
MinGW build of `glm.exe` is unchanged.

---

## Phase G2 — DLL ABI and multi-GPU (planned)

Goal: stable versioned ABI for `coli_cuda.dll`; multi-GPU expert sharding via
the existing `COLI_GPUS` env var; P2P / UVA transfers between devices.

---

## Phase V — full-model validation (planned)

Byte-identical output validation against the Linux oracle on the real 370 GB
GLM-5.2 int4 checkpoint.  Blocked on access to a Windows machine with the
model downloaded.  The tiny-random fixture (`make_glm_oracle.py`) already
passes on Phase 1.

---

## Non-goals

- WSL2: not targeted by this plan (WSL2 runs the Linux binary unmodified).
- 32-bit Windows: not supported; `_FILE_OFFSET_BITS=64` is mandatory.
- MSVC as the primary compiler: MinGW-w64 / MSYS2 GCC is the supported
  toolchain for the CPU binary; MSVC is used only for `coli_cuda.dll`.
