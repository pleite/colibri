# Copilot NPU Runtime Fix — Complete Plan

**Date:** 2026-07-27
**Target:** Strix Halo (Ryzen AI Max+ 395) — XDNA 2 NPU
**Branch:** `copilot/validate-npu-plan` (this workspace)
**Purpose:** Validate the existing Strix Halo NPU plan against the current host and the repository implementation, then carry out hardware-side execution only on a real Strix Halo target.

---

## 1. Hardware & System State (validated on this host, 2026-07-27)

| Property | Value |
|---|---|
| Sandbox host | `runnervm3jd5f` / `Linux 6.17.0-1020-azure` |
| CPU / arch | `x86_64` (Azure VM); no visible Strix Halo device |
| NPU PCI device | None found by `lspci` or `/sys/bus/pci/devices` |
| Kernel driver | `amdxdna` is not loaded; `lsmod` and `modinfo amdxdna` return no module |
| Device node | `/dev/accel/accel0` is not present in this sandbox |
| Kernel headers | No `drm/amdxdna_accel.h` or `amdxdna` module files were found on this host |
| NPU artifacts | No `.npukernel` / `.xclbin` files were found under the repo checkout |
| Colibri repo | `/home/runner/work/colibri/colibri` |
| Current branch | `copilot/validate-npu-plan` |

### What this means

The current sandbox is not a Strix Halo host and cannot be used to drive real NPU execution. The remaining work is therefore split into two tracks:

1. Repository-side validation that can be done here (code review against the existing implementation and prior docs).
2. Hardware-side execution that must be run on an actual Strix Halo host or in a container that exposes `/dev/accel/accel0` and the `amdxdna` driver stack.

### How to verify the target host

```bash
uname -a
lsmod | grep amdxdna || true
lspci -nn | grep -iE '17f0|accel|npu|xdna' || true
ls -l /dev/accel/accel0 2>/dev/null || true
modinfo amdxdna 2>/dev/null | head
```

If the target is the previously referenced Strix Halo machine, use:

```bash
ssh leite@192.168.1.129 "uname -r && lsmod | grep amdxdna && lspci -nn | grep 17f0 && ls -la /dev/accel/accel0"
```

---

## 2. The amdxdna Kernel UAPI (what the kernel actually exposes)

All ioctls come from `<drm/amdxdna_accel.h>`. The driver registers an accel node at `/dev/accel/accel0`.

### 2.1 ioctl enumeration

```c
enum amdxdna_drm_ioctl_id {
    DRM_AMDXDNA_CREATE_HWCTX,       // Create hardware context
    DRM_AMDXDNA_DESTROY_HWCTX,      // Destroy hardware context
    DRM_AMDXDNA_CONFIG_HWCTX,       // Configure hardware context (CU registration)
    DRM_AMDXDNA_CREATE_BO,          // Create buffer object
    DRM_AMDXDNA_GET_BO_INFO,        // Get BO info (xdna_addr, map_offset, vaddr)
    DRM_AMDXDNA_SYNC_BO,            // Sync buffer to/from device
    DRM_AMDXDNA_EXEC_CMD,           // Execute command (ERT packet submission)
    DRM_AMDXDNA_GET_INFO,           // Query AIE metadata, firmware version, etc.
    DRM_AMDXDNA_SET_STATE,          // Set power mode, write AIE mem/reg
    DRM_AMDXDNA_GET_ARRAY,          // Get arrays (hw contexts, async errors, BO usage)
};
```

### 2.2 CREATE_HWCTX — Creating the hardware context

**Struct:** `struct amdxdna_drm_create_hwctx`
```c
struct amdxdna_drm_create_hwctx {
    __u64 ext;                    // MBZ
    __u64 ext_flags;              // MBZ
    __u64 qos_p;                  // Pointer to amdxdna_qos_info
    __u32 umq_bo;                 // User mode queue BO handle (required)
    __u32 log_buf_bo;             // Log buffer BO handle (required)
    __u32 max_opc;                // Max operations per cycle
    __u32 num_tiles;              // Number of AIE tiles (columns × core_rows)
    __u32 mem_size;               // AIE tile memory size
    __u32 umq_doorbell;           // OUT: doorbell offset
    __u32 handle;                 // OUT: hardware context handle
    __u32 syncobj_handle;         // OUT: timeline syncobj handle (for wait)
};
```

**Critical ordering:**
1. Create device heap BO (`AMDXDNA_BO_DEV_HEAP`, 64 MiB) — **must be done first**
2. mmap() the heap at 64 MiB-aligned address — **must be done before CREATE_HWCTX**
3. Create UMQ BO (`AMDXDNA_BO_CMD`, 4096 bytes)
4. Create log BO (`AMDXDNA_BO_CMD`, 4096 bytes)
5. Issue `DRM_IOCTL_AMDXDNA_CREATE_HWCTX`

Without the device heap, `CREATE_HWCTX` returns `-ENOENT`.
Without the heap mapped, `CREATE_HWCTX` returns `-EINVAL`.
The firmware also requires the heap's user address to be a multiple of 64 MiB chunks.

### 2.3 CONFIG_HWCTX — Registering CUs/PDI

**Struct:** `struct amdxdna_drm_config_hwctx`
```c
struct amdxdna_drm_config_hwctx {
    __u32 handle;                 // Hardware context handle
    __u32 param_type;             // DRM_AMDXDNA_HWCTX_CONFIG_CU
    __u64 param_val;              // Pointer to amdxdna_hwctx_param_config_cu
    __u32 param_val_size;         // Size of param_val buffer (max PAGE_SIZE=4KiB)
    __u32 pad;
};
```

**Param type:** `DRM_AMDXDNA_HWCTX_CONFIG_CU`

**Payload struct:** `struct amdxdna_hwctx_param_config_cu`
```c
struct amdxdna_hwctx_param_config_cu {
    __u16 num_cus;                              // Number of CUs
    __u16 pad[3];
    struct amdxdna_cu_config cu_configs[];      // Flexible array
};

struct amdxdna_cu_config {
    __u32 cu_bo;       // BO handle of the CU configuration buffer
    __u8  cu_func;     // CU function type
    __u8  pad[3];
};
```

**How PDI loading works:** The CU configuration buffer (a BO of type `AMDXDNA_BO_DEV`) contains the PDI data. The xclbin's IP_LAYOUT section is parsed to extract CU configurations. The BO is passed via `cu_bo` to `CONFIG_HWCTX`. This is the **PDI registration** step — the firmware learns about the partition from the CU config BO.

**CRITICAL PITFALL:** `DRM_AMDXDNA_HWCTX_CONFIG_CU` is an `enum amdxdna_drm_config_hwctx_param` value, NOT a `#define` macro. The C preprocessor `#if defined()` check will NOT work for it. Use it directly as an enum value.

### 2.4 CREATE_BO — Buffer objects

**Struct:** `struct amdxdna_drm_create_bo`
```c
struct amdxdna_drm_create_bo {
    __u64 flags;        // MBZ
    __u64 vaddr;        // User VA (MBZ if not userptr)
    __u64 size;         // Size in bytes
    __u32 type;         // enum amdxdna_bo_type
    __u32 handle;       // OUT: BO handle
};
```

**Buffer types:**
```c
enum amdxdna_bo_type {
    AMDXDNA_BO_INVALID = 0,
    AMDXDNA_BO_SHMEM = 1,    // Host-visible shared memory
    AMDXDNA_BO_SHARE = 1,    // Alias for SHMEM
    AMDXDNA_BO_DEV_HEAP = 2, // Device heap (must be created first)
    AMDXDNA_BO_DEV = 3,      // Device memory (sub-allocated from heap)
    AMDXDNA_BO_CMD = 4,      // Command buffer (UMQ, log, ERT packets)
};
```

**GET_BO_INFO returns:**
- `map_offset`: Fake mmap() offset (0 for AMDXDNA_BO_DEV, valid for SHMEM)
- `vaddr`: User VA (`AMDXDNA_INVALID_ADDR=~0` if not mapped)
- `xdna_addr`: XDNA device virtual address (always valid for DEV BOs)

### 2.5 EXEC_CMD — Submitting ERT commands

**Struct:** `struct amdxdna_drm_exec_cmd`
```c
struct amdxdna_drm_exec_cmd {
    __u64 ext;            // MBZ
    __u64 ext_flags;      // MBZ
    __u32 hwctx;          // Hardware context handle
    __u32 type;           // AMDXDNA_CMD_SUBMIT_EXEC_BUF
    __u64 cmd_handles;    // BO handle(s) of command buffer(s)
    __u64 args;           // BO handle(s) of argument buffers
    __u32 cmd_count;      // Number of cmd_handles
    __u32 arg_count;      // Number of args
    __u64 seq;            // OUT: command sequence number (timeline point)
};
```

### 2.6 GET_INFO — Querying device info

**Struct:** `struct amdxdna_drm_get_info`
```c
struct amdxdna_drm_get_info {
    __u32 param;          // enum amdxdna_drm_get_param
    __u32 buffer_size;    // IN/OUT: buffer size
    __u64 buffer;         // IN/OUT: buffer pointer
};
```

**Query params:**
```c
enum amdxdna_drm_get_param {
    DRM_AMDXDNA_QUERY_AIE_STATUS,           // 0
    DRM_AMDXDNA_QUERY_AIE_METADATA,         // 1
    DRM_AMDXDNA_QUERY_AIE_VERSION,          // 2
    DRM_AMDXDNA_QUERY_CLOCK_METADATA,       // 3
    DRM_AMDXDNA_QUERY_SENSORS,              // 4
    DRM_AMDXDNA_QUERY_HW_CONTEXTS,          // 5
    DRM_AMDXDNA_QUERY_FIRMWARE_VERSION,     // 8
    DRM_AMDXDNA_GET_POWER_MODE,             // 9
    DRM_AMDXDNA_QUERY_TELEMETRY,            // 10
    DRM_AMDXDNA_GET_FORCE_PREEMPT_STATE,    // 11
    DRM_AMDXDNA_QUERY_RESOURCE_INFO,        // 12 (post-6.18 only)
    DRM_AMDXDNA_GET_FRAME_BOUNDARY_PREEMPT_STATE, // 13
};
```

**Note:** `DRM_AMDXDNA_QUERY_RESOURCE_INFO` (param 12) requires kernel headers ≥ 6.18. Fedora 7.1.4 includes it.

### 2.7 WAIT — Command completion

There is **no** `WAIT_CMD` ioctl in the mainline UAPI. Completion is signaled through the **timeline syncobj** returned by `CREATE_HWCTX`. Use `DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT`:

```c
struct drm_syncobj_timeline_wait {
    __u64 handles;        // Pointer to syncobj handle array
    __u64 points;         // Pointer to timeline point array
    __s64 timeout_nsec;   // Absolute CLOCK_MONOTONIC deadline (INT64_MAX = forever)
    __u32 count_handles;
    __u32 flags;
};
// flags: DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL | DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT
```

---

## 3. ERT Packet Structure (for command submission)

### 3.1 Base packet header

```c
struct ert_packet {
    uint32_t state:4;     // [3:0]  ERT_CMD_STATE_NEW = 1
    uint32_t custom:8;    // [11:4] Custom per-command
    uint32_t count:11;    // [22:12] Number of payload words
    uint32_t opcode:5;    // [27:23] Command opcode
    uint32_t type:4;      // [31:28] Command type (0)
    uint32_t data[];      // Payload words
};
```

Header encoding:
```c
header = (state & 0xF) | ((count & 0x7FF) << 12) | ((opcode & 0x1F) << 23);
```

### 3.2 ERT opcodes relevant to NPU

```c
enum ert_cmd_opcode {
    ERT_START_CU              = 0,   // Start compute unit
    ERT_CONFIGURE             = 2,   // Configure command scheduler
    ERT_START_DPU             = 18,  // DPU instruction buffer
    ERT_CMD_CHAIN             = 19,  // Command chain
    ERT_START_NPU             = 20,  // NPU instruction buffer ← THIS ONE
    ERT_START_NPU_PREEMPT     = 21,  // NPU preemption
    ERT_START_NPU_PREEMPT_ELF = 22,  // NPU preemption with ELF
};
```

### 3.3 ERT_START_NPU payload structure

The data payload for `ERT_START_NPU` follows this layout:

```
[ert_packet header (4 bytes)]
[CU mask (4 bytes)]
[ert_npu_data struct (16 bytes)]
  - instruction_buffer: uint64_t  (device VA of instruction stream)
  - instruction_buffer_size: uint32_t (size in bytes)
  - instruction_prop_count: uint32_t (WORD count of following properties, usually 0)
[kernel args (variable, 32 bytes for matmul)]
  - x: uint64_t  (device VA of input activation)
  - w: uint64_t  (device VA of weights)
  - y: uint64_t  (device VA of output accumulator)
  - I: uint32_t  (input dimension)
  - O: uint32_t  (output dimension)
```

**Critical:** The `ert_npu_data` struct is 16 bytes (2 uint64 + 2 uint32). The `count` field in the packet header must include ALL payload words after the header: CU mask (1 word) + npu_data (4 words) + kernel_args (8 words) = 13 words minimum for matmul.

### 3.4 The .npukernel artifact format

Instead of hardcoding ERT opcodes, the runtime uses self-describing `.npukernel` artifacts:

```
offset  size  field
0       4     magic       'XDN2'  (0x324E4458)
4       4     version     1
8       4     ert_opcode  ERT packet opcode for this kernel
12      4     cu_mask     compute-unit mask
16      4     rows
20      4     inner_dim
24      4     out_cols
28      4     fmt         1 = INT8
32      4     instr_size  instruction stream length in bytes
36      4     instr_words words the DPU should execute
40      …     instruction stream (binary)
```

---

## 4. xclbin / PDI Loading

### 4.1 How PDI registration actually works

**The XRT approach (reference only):**
XRT loads xclbin via `xrt::xclbin(filename)`, parses `IP_LAYOUT` section, creates CUs via `xrt::hwctx::register_cu()`, and submits via `xrt::kernel::run()`.

**The direct DRM approach (what colibri uses):**
1. Parse the xclbin's `IP_LAYOUT` section to extract `ip_data` entries
2. For each CU, create a `AMDXDNA_BO_DEV` buffer containing the CU configuration
3. Call `DRM_IOCTL_AMDXDNA_CONFIG_HWCTX` with `DRM_AMDXDNA_HWCTX_CONFIG_CU` to register it

### 4.2 IP_LAYOUT section structure (from xrt/detail/xclbin.h)

```c
struct ip_data {
    uint32_t m_type;           // IP_TYPE enum (IP_KERNEL, IP_PS_KERNEL, etc.)
    union {
        uint32_t properties;   // Default: int_enable, interrupt_id, ip_control
        struct { /* ps_kernel */ } ps_kernel;
        struct { /* indices */ } indices;
    };
    uint64_t m_base_address;   // Base address of the IP
    uint8_t m_name[64];        // IP name (e.g., "matmul_int8_256x4096x1024")
};
// sizeof(ip_data) == 80 bytes

struct ip_layout {
    int32_t m_count;           // Number of IP entries
    struct ip_data m_ip_data[]; // Flexible array, sorted by m_base_address
};
```

**IP types:**
- `IP_KERNEL` — PL kernel compute unit
- `IP_PS_KERNEL` — PS (pre/post-processing) kernel
- `IP_MEM_*` — Memory interface IPs

**PS kernel properties:**
```c
struct {
    uint16_t m_subtype : 2;        // 0=ST_PS, 1=ST_DPU
    uint16_t : 2;
    uint16_t m_functional : 2;     // 0=FC_DPU, 1=FC_PREPOST
    uint16_t : 10;
    uint16_t m_kernel_id : 12;
    uint16_t : 4;
} ps_kernel;
```

### 4.3 axlf (xclbin) top-level structure

```c
struct axlf {
    char m_magic[8];          // "XCLXLOGI"
    uint32_t m_section_count;
    struct axlf_section_header m_sections[];
    // ... section data follows
};

struct axlf_section_header {
    char m_section_name[16];  // "IP_LAYOUT", "CONNECTIVITY", "MEM_TOPOLOGY", etc.
    uint32_t m_section_type;  // SectionType enum
    uint64_t m_section_offset;
    uint64_t m_section_size;
    uint32_t m_padding[3];
};
```

**Section types (relevant):**
- `IP_LAYOUT = 8` — CU definitions
- `DEBUG_IP_LAYOUT = 9` — Debug IPs
- `CONNECTIVITY = 11` — Argument-to-memory connections
- `MEM_TOPOLOGY = 12` — Memory bank definitions
- `DATA_SECTION = 15` — Binary/data sections

---

## 5. Colibri NPU Runtime Implementation (current state)

### 5.1 File layout

```
c/npu_kernels/
  xdna2_driver.h          — Public API header (device, hwctx, BO, submit, query)
  xdna2_driver.c          — DRM ioctl wrapper implementation (~500 lines)
  xdna2_matmul.h          — Matmul runtime public API
  xdna2_matmul.c          — Fixed-shape INT8 matmul with ERT packet building
  xdna2_xrt_driver.{c,h}  — XRT availability probe (no execution path)
  npu_runtime.h           — coli_npu_* integration surface for c/backend_npu.c
  tests/test_npu.c        — Device/runtime probe suite

vnni-int8-matmul/npu/
  xdna2_backend.{c,h}     — strix_xdna2_* backend for VNNI building blocks

vnni-int8-matmul/npu/aie/
  ert_opcode_probe.c      — Prints ERT_START_CU and ERT_START_NPU enum values
```

### 5.2 What's implemented in the current repo

1. **Device heap management** — Creates 64 MiB `AMDXDNA_BO_DEV_HEAP`, mmaps at a 64 MiB-aligned address before `CREATE_HWCTX`
2. **Hardware context creation** — `CREATE_HWCTX` with UMQ + log BOs, returns a timeline syncobj
3. **CU/PDI registration path** — `CONFIG_HWCTX` with `DRM_AMDXDNA_HWCTX_CONFIG_CU`, and the runtime loads a sidecar `.xclbin`
4. **ERT_START_NPU payload** — The repo already uses the 16-byte `ert_npu_data` + 32-byte kernel-args layout that matches the previous successful implementation notes
5. **Command submission** — `EXEC_CMD` with an `AMDXDNA_BO_CMD` buffer containing the ERT packet
6. **Command wait** — `DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT` on the timeline syncobj
7. **Device queries** — AIE metadata, firmware version, PCI IDs, and resource-info probes are present
8. **Kernel artifact loading** — The runtime reads `.npukernel` files and extracts opcode / mask / instructions

### 5.3 What's NOT yet proven on this host (the remaining gaps)

1. **Hardware validation is blocked here** — This sandbox does not expose `amdxdna`, `/dev/accel/accel0`, or an NPU PCI device, so the runtime cannot be exercised end-to-end on this host
2. **Real-device confirmation is still required** — The repo code already reflects the successful implementation approach from the earlier Strix Halo notes, but that path still needs to be validated on an actual Strix Halo machine
3. **Instruction stream format** — The `.npukernel` contains a DPU instruction stream, but the exact format expected by the XDNA 2 firmware still needs hardware confirmation
4. **xclbin validation** — The runtime should still be tested against the target host to verify that the xclbin's IP_LAYOUT and CU configuration are accepted by the firmware
5. **Decode shapes (rows=1)** — These remain hardware-limited and should be treated as out-of-scope for the current validation pass unless a real Strix Halo target is available

---

## 6. Build System

### 6.1 Kernel headers requirement

- `<drm/amdxdna_accel.h>` — kernel headers ≥ 6.14 (in-tree since 6.14)
- `DRM_AMDXDNA_QUERY_RESOURCE_INFO` — kernel headers ≥ 6.18 (Fedora 7.1.4 has it)
- `DRM_SYNCOBJ_TIMELINE_WAIT` — standard DRM ioctl, available in all modern headers

### 6.2 XRT headers (inside podman container only)

```
ghcr.io/pleite/colibri-npu:main-npu
  /usr/xrt/include/
    xrt/detail/ert.h              — ERT packet structures (1213 lines)
    xrt/detail/xclbin.h           — xclbin/axlf/IP_LAYOUT structures (705 lines)
    xrt/experimental/xrt_xclbin.h — xrt::xclbin C++ API
    xrt/experimental/xrt_aie.h    — xrt::aie::program API
    xrt/experimental/xrt_hw_context.h — xrt::hwctx API
    xrt/experimental/xrt_bo.h     — xrt::bo API
    xrt/experimental/xrt_elf.h    — xrt::elf API
    xrt/experimental/xrt_kernel.h — xrt::kernel API
    xrt.h                         — Top-level XRT C API
```

### 6.3 AIE toolchain container

```
ghcr.io/pleite/colibri-aie-toolchain:latest  (2.45 GB)
  — IRON/mlir-aie + Peano toolchain
  — Used to compile .npukernel artifacts from AIE IR
  — NOT the proprietary AMD aiecompiler (no public XDNA 2 release)
```

---

## 7. Key Pitfalls (learned from debugging)

1. **`#if defined(DRM_AMDXDNA_HWCTX_CONFIG_CU)` does NOT work** — It's an enum value, not a `#define` macro. Just use the enum directly.
2. **Device heap MUST be mapped before CREATE_HWCTX** — Not just created. The driver records the user address in `mem.userptr` only during mmap.
3. **Heap mapping must be 64 MiB-aligned** — Plain `mmap()` is page-aligned only. Use the reserve+MAP_FIXED pattern.
4. **`AMDXDNA_INVALID_ADDR` is `~0`, not `0`** — GET_BO_INFO returns this sentinel for unmapped BOs. Don't treat it as a valid address.
5. **`seq` from EXEC_CMD is NOT the syncobj handle** — It's a timeline point. Store separately in `ctx->last_seq`.
6. **No WAIT_CMD ioctl in mainline** — Use `DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT` on the syncobj returned by CREATE_HWCTX.
7. **`DRM_AMDXDNA_QUERY_RESOURCE_INFO` is param 12, not 15** — Magic numbers are wrong.
8. **PCI vendor for NPU is 0x1022, NOT 0x1002** — 0x1002 is Radeon/ATI. The accel function reports AMD's CPU vendor ID.
9. **AIE metadata version 1.1 does NOT mean AIE-1** — Strix Halo (XDNA 2) reports 1.1. Use PCI IDs for generation detection.
10. **ERT opcode must come from .npukernel artifact** — Never hardcode. The kernel's calling convention determines it.

---

## 8. Implementation Plan

These tasks are the next steps for a real Strix Halo host. The current sandbox is only suitable for repo-side validation; it cannot execute the hardware steps below.

### Task 1: Validate xclbin IP_LAYOUT parsing

**Goal:** Parse the sidecar `.xclbin` file that ships alongside each `.npukernel`, extract the `IP_LAYOUT` section, and create a `AMDXDNA_BO_DEV` buffer with the CU configuration data.

**Files to modify:**
- `c/npu_kernels/xdna2_driver.c` — Add `xdna2_parse_xclbin_ip_layout()` function
- `c/npu_kernels/xdna2_driver.h` — Add new function declaration

**What to implement:**
1. Read the axlf header, find the `IP_LAYOUT` section by name
2. Parse each `ip_data` entry (80 bytes each)
3. Filter for `IP_PS_KERNEL` type (DPU CUs), extract the CU configuration
4. Create a `AMDXDNA_BO_DEV` buffer containing the parsed CU config
5. Register it via `CONFIG_HWCTX` with `DRM_AMDXDNA_HWCTX_CONFIG_CU`

**Validation:**
```bash
ssh leite@192.168.1.129 "cd /home/leite/colibri/vnni-int8-matmul && make && ./tests/npu_device_test"
```

### Task 2: Validate ERT_START_NPU payload against XRT's ert.h

**Goal:** Ensure the payload struct layout matches XRT's `struct ert_npu_data` exactly.

**Files to check:**
- `c/npu_kernels/xdna2_matmul.c` — Current payload struct definitions
- `xrt/detail/ert.h` (inside `ghcr.io/pleite/colibri-npu:main-npu`) — Reference layout

**What to verify:**
1. `sizeof(xdna2_ert_npu_data_t) == 16` (already asserted)
2. Field order: `instruction_buffer` (u64), `instruction_buffer_size` (u32), `instruction_prop_count` (u32)
3. Kernel args: `x` (u64), `w` (u64), `y` (u64), `I` (u32), `O` (u32)
4. Total payload words: 13 (CU mask=1 + npu_data=4 + args=8)
5. `count` field in header = 13

**Validation:**
```bash
ssh leite@192.168.1.129 "podman run --rm ghcr.io/pleite/colibri-npu:main-npu cat /usr/xrt/include/xrt/detail/ert.h | grep -A 10 'struct ert_npu_data'"
```

### Task 3: End-to-end matmul test on hardware

**Goal:** Run a real matmul on the NPU with a compiled kernel artifact and verify the output.

**Prerequisites:**
- At least one `.npukernel` artifact exists (rows=256, e.g., `matmul_int8_256x4096x4096.npukernel`)
- Its sidecar `.xclbin` exists in the same directory
- The amdxdna module is loaded and `/dev/accel/accel0` is accessible

**Test command:**
```bash
ssh leite@192.168.1.129 "cd /home/leite/colibri/vnni-int8-matmul && podman run --rm --device /dev/accel/accel0 localhost/colibri-strix-halo-test ./tests/test_npu"
```

**Expected output:**
- Device discovery: PASS
- Hardware context: PASS
- Buffer objects: PASS
- Runtime lifecycle: PASS
- Matmul execution: PASS (if kernel artifact exists)

### Task 4: Check dmesg for firmware rejection messages

**Goal:** If the NPU rejects the PDI or command, dmesg will show why.

**Command:**
```bash
ssh leite@192.168.1.129 "sudo dmesg | grep -iE 'xdna|amdnpu|accel|aie2' | tail -30"
```

**Common messages to look for:**
- "Invalid dev heap userptr" — heap not mapped before CREATE_HWCTX
- "Map host buffer failed" — firmware rejected heap address alignment
- "PDI registration failed" — xclbin parsing produced invalid CU config
- "ERT command rejected" — malformed ERT packet
- "AIE partition conflict" — another process owns the partition

---

## 9. Restart Instructions

To resume NPU runtime development from this point:

1. **Check current state:**
   ```bash
   ssh leite@192.168.1.129 "cd /home/leite/colibri && git log --oneline -5 && git status"
   ```

2. **Verify hardware:**
   ```bash
   ssh leite@192.168.1.129 "lsmod | grep amdxdna && lspci | grep 17f0"
   ```

3. **Build and test:**
   ```bash
   ssh leite@192.168.1.129 "cd /home/leite/colibri/vnni-int8-matmul && make && ./scripts/strix-halo-podman-test.sh"
   ```

4. **Key files to read first:**
   - `c/npu_kernels/xdna2_driver.c` — Full DRM ioctl wrapper
   - `c/npu_kernels/xdna2_matmul.c` — ERT packet building and kernel dispatch
   - `docs/strix-halo-npu.md` — Architecture docs and guardrails
   - `docs/plans/2026-07-26_npu-execution-path.md` — Implementation plan

5. **Next steps from current state:**
   - Validate xclbin IP_LAYOUT parsing produces correct CU configs
   - Test end-to-end matmul on hardware with rows=256 kernel
   - Fix any firmware rejection messages (check dmesg on host)

---

## 10. Reference: Current Source Code Summary

### xdna2_driver.c (879 lines)
- `xdna2_open_device()` — Opens `/dev/accel/accel0`
- `xdna2_create_hwctx()` — Creates device heap (64 MiB), maps it (64 MiB-aligned), creates UMQ+log BOs, issues CREATE_HWCTX
- `xdna2_config_hwctx_single_cu()` — Registers one CU via CONFIG_HWCTX (uses `#if defined(DRM_IOCTL_AMDXDNA_CONFIG_HWCTX)` guard)
- `xdna2_create_bo()` — Creates BO via CREATE_BO + GET_BO_INFO
- `xdna2_map_bo()` / `xdna2_unmap_bo()` — mmap/munmap with 64 MiB alignment support
- `xdna2_submit_command()` — EXEC_CMD submission
- `xdna2_wait_command()` — DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT
- `xdna2_query_aie_metadata()` — GET_INFO with DRM_AMDXDNA_QUERY_AIE_METADATA
- `xdna2_query_resource_info()` — GET_INFO with DRM_AMDXDNA_QUERY_RESOURCE_INFO (param 12)
- `xdna2_query_firmware_version()` — GET_INFO with DRM_AMDXDNA_QUERY_FIRMWARE_VERSION

### xdna2_matmul.c (564 lines)
- `xdna2_runtime_init()` — Opens device, creates hwctx with 16 tiles, 32 KiB mem_size, max_opc=4
- `xdna2_load_kernel()` — Reads `.npukernel` artifact, uploads instruction stream to DEV BO, loads sidecar `.xclbin` for CU registration
- `xdna2_matmul_int8_timed()` — Full dispatch: alloc SHMEM BOs for x/w/y, sync to device, build ERT packet, submit, wait, sync back, dequantize
- `xdna2_dequant_i32()` — Converts int32 accumulators to f32 with per-column scales
- `xdna2_dequant_int4()` — Expands packed int4 to int8

### xdna2_backend.c (174 lines)
- `strix_xdna2_matmul()` — Backend entry point for VNNI building blocks
- `strix_xdna2_is_supported()` — Checks if NPU is available
- `strix_xdna2_kernel_exists()` — Checks if kernel artifact exists for shape
- Lazy kernel loading from `COLI_NPU_KERNEL_DIR` (default: `npu/kernels`)

### Test harness (test_npu.c, ~300 lines)
- Device discovery test
- Hardware context creation test
- Buffer object management test
- Runtime lifecycle test
- No-CPU-fallback assertion test
- int4 expansion test
- Kernel lookup test
- Control plane parsing test

---

## 11. Podman Images Available

```
ghcr.io/pleite/colibri-npu:main-npu          (612 MB) — XRT headers at /usr/xrt/include/
ghcr.io/pleite/colibri-aie-toolchain:latest  (2.45 GB) — AIE compiler toolchain
ghcr.io/pleite/colibri-strix-halo-test:latest (958 MB) — Test container with NPU passthrough
ghcr.io/pleite/colibri-rocm:main-rocm        (10.4 GB) — ROCm build environment
ghcr.io/pleite/colibri-cpu:main-cpu          (547 MB) — CPU-only build
```

---

## 12. SSH Access

```
Host: 192.168.1.129
User: leite
Key:  /home/leite/.ssh/id_ed25519_github (if available locally)
```

If the GitHub key isn't available on this host, use the standard SSH key:
```bash
ssh -i ~/.ssh/id_ed25519 leite@192.168.1.129
```

---

## 13. Local self-hosted agent runbook (Strix Halo)

Use this when the execution agent is running on the Strix Halo machine itself, via the self-hosted GitHub Actions runner, rather than on a cloud runner.

### 13.1 What to do first

1. Treat the self-hosted runner as the target. The GitHub cloud runners are not a valid target for this work because they do not expose the hardware.
2. Verify the host before doing anything else:
   ```bash
   uname -a
   ls -l /dev/dri /dev/kfd /dev/accel/accel0 2>/dev/null || true
   lsmod | grep amdxdna || true
   sudo dmesg | grep -iE 'amdxdna|accel|aie2|xclbin' | tail -n 40 || true
   ```
3. If the host kernel / `amdxdna` module rejects the xclbin or reports a loader/version mismatch, stop and do not try a fresh hand-built xclbin with a guessed format. The correct response is to use the artifact set that the self-hosted NPU-kernel build workflow produced, or to run that workflow again on the same Strix Halo runner and use its output.

### 13.2 Preferred execution path

Use the self-hosted workflows that already exist in this repo:

1. Build the kernel artifacts on the self-hosted runner:
   - Workflow: `.github/workflows/npu-kernels.yml`
   - Equivalent local command on the Strix Halo host:
     ```bash
     cd /home/leite/colibri/vnni-int8-matmul
     ./scripts/build-npu-kernels.sh --allow-partial
     python3 npu/aie/pack_npukernel.py --verify npu/kernels/*.npukernel
     ```
2. Run the Strix Halo harness against the resulting artifacts:
   - Workflow: `.github/workflows/vnni-test.yml`
   - Equivalent local command on the Strix Halo host:
     ```bash
     cd /home/leite/colibri/vnni-int8-matmul
     REQUIRE_NPU=1 ./scripts/strix-halo-podman-test.sh
     ```

### 13.3 If the host rejects the xclbin version

If the host logs a failure such as an xclbin/loader version mismatch, unsupported xclbin load, or a similar `amdxdna`/firmware rejection, then:

1. Do not change the runtime to a different guessed xclbin format.
2. Reuse the artifact bundle produced by the self-hosted `npu-kernels` workflow (or rerun that workflow on the same host) and test that artifact set.
3. Keep the test run focused on the host/kernel/module that is actually present. The answer is not "use a cloud runner"; it is "use the known-good artifact set from the self-hosted Strix Halo workflow".

### 13.4 What the agent must capture

After every run, the agent should save and report:

- `vnni-int8-matmul/test_output.log`
- `vnni-int8-matmul/amdxdna.log` (if produced)
- the first 40 lines of `dmesg` that mention `amdxdna`, `accel`, `aie2`, or `xclbin`
- whether the run used a locally built artifact set or the workflow artifact set

### 13.5 Acceptance criteria

The run is successful only when:

- the Strix Halo host exposes `/dev/accel/accel0` and the `amdxdna` stack is available,
- the harness actually runs the NPU path rather than silently falling back to CPU,
- the run log shows the `.npukernel` artifact being used, and
- any rejection is reported as a real kernel/module/firmware issue rather than a silent skip.

---

## 14. Questions and follow-up queries for the execution agent

Use these prompts when more information is needed. They are intentionally narrow so the agent can answer them without ambiguity and then return to the plan with the missing data.

- `[ASK-1]` What exact target host are you using for the NPU run? Please paste the output of `uname -a`, `lspci -nn | grep -iE '17f0|accel|npu|xdna'`, and `ls -l /dev/accel/accel0`.
- `[ASK-2]` Is `amdxdna` loaded on that host? Please paste `lsmod | grep amdxdna`, `modinfo amdxdna`, and the last 30 lines of `dmesg | grep -iE 'amdxdna|accel|npu'`.
- `[ASK-3]` Are there any `.npukernel` and sidecar `.xclbin` artifacts available for the target build? Please list them with `find <repo> -name '*.npukernel' -o -name '*.xclbin'`.
- `[ASK-4]` When you run `./tests/npu_device_test`, which step fails first: device open, hwctx creation, BO create/map, or command execution? Please paste the exact stderr/stdout.
- `[ASK-5]` If the failure occurs during `CREATE_HWCTX` or `EXEC_CMD`, please paste the relevant `dmesg` lines and the return value from the failing call.
- `[ASK-6]` If the execution must move to a different host, please provide the new host name/IP and the access method before running the next step.
- `[ASK-7]` After each run, paste the relevant stdout/stderr plus the last 30 lines of `dmesg` and I will update this plan with the next action.
