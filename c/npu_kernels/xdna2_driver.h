#ifndef XDNA2_DRIVER_H
#define XDNA2_DRIVER_H

/**
 * xdna2_driver.h — DRM ioctl wrapper for AMD XDNA 2 NPU
 *
 * Pure C interface to the in-tree `amdxdna` accel driver via /dev/accel/accel0
 * (override with the XDNA2_DEVICE environment variable). No XRT dependency.
 *
 * Strix Halo: XDNA 2, 10 columns x 8 rows AIE array, ~30 TOPS INT8.
 * This wrapper is Strix Halo exclusive and has no software-emulation path.
 */

#include <stdint.h>
#include <stdbool.h>

/**
 * DRM constants are defined in <drm/amdxdna_accel.h>.
 * This header does NOT redefine them — include amdxdna_accel.h
 * in the .c file for the actual definitions.
 */

/* ── Device heap ──
 *
 * The amdxdna driver gives each open file descriptor a single device heap and
 * carves every AMDXDNA_BO_DEV allocation out of it. It must exist before
 * DRM_IOCTL_AMDXDNA_CREATE_HWCTX, which otherwise fails with -ENOENT
 * ("The client dev heap object not exist"). The kernel caps it at
 * AIE2_DEVM_SIZE (64 MiB) and aligns device allocations to dev_mem_buf_shift
 * (32 KiB). Override the size with the XDNA2_HEAP_BYTES environment variable.
 */
#define XDNA2_DEV_HEAP_ALIGN      (32u * 1024u)
#define XDNA2_DEV_HEAP_MAX_BYTES  (64ull * 1024ull * 1024ull)

/* ── Opaque handles ── */

typedef struct {
    uint32_t handle;        /* DRM BO handle */
    uint64_t size;          /* Buffer size in bytes */
    uint32_t type;          /* Buffer type (enum amdxdna_bo_type) */
    uint64_t map_offset;    /* Fake mmap() offset returned by the driver */
    uint64_t vaddr;         /* Returned user VA (0 if the BO needs mmap) */
    uint64_t xdna_addr;     /* XDNA device virtual address */
    void *mapped;           /* Host pointer, valid after xdna2_map_bo() */
    bool owns_mapping;      /* True when `mapped` came from mmap() */
} xdna2_bo_t;

typedef struct {
    int fd;                 /* /dev/accel/accel0 file descriptor */
    uint32_t hwctx_handle;  /* Hardware context handle from driver */
    xdna2_bo_t dev_heap_bo; /* Client device heap, required before CREATE_HWCTX */
    xdna2_bo_t umq_bo;      /* User mode queue BO, owned for the ctx lifetime */
    xdna2_bo_t log_bo;      /* Log buffer BO, owned for the ctx lifetime */
    uint32_t umq_doorbell;  /* Doorbell offset */
    uint32_t num_tiles;     /* Number of AIE tiles allocated */
    uint32_t mem_size;      /* AIE tile memory size */
    uint32_t max_opc;       /* Max operations per cycle */
    uint32_t syncobj_handle;/* Timeline syncobj returned at hwctx creation */
    uint64_t last_seq;      /* Timeline point of the last submitted command */
    bool initialized;
} xdna2_hwctx_t;

/* ── AIE metadata ── */

typedef struct {
    uint16_t row_count;
    uint16_t row_start;
    uint16_t dma_channel_count;
    uint16_t lock_count;
    uint16_t event_reg_count;
    uint16_t pad[3];
} xdna2_tile_metadata_t;

typedef struct {
    uint32_t col_size;
    uint16_t cols;
    uint16_t rows;
    uint32_t version_major;
    uint32_t version_minor;
    xdna2_tile_metadata_t core;
    xdna2_tile_metadata_t mem;
    xdna2_tile_metadata_t shim;
} xdna2_aie_metadata_t;

/* ── Resource info ── */

typedef struct {
    uint64_t npu_clk_max;    /* Max H-clock frequency */
    uint64_t npu_tops_max;   /* Max TOPS */
    uint64_t npu_task_max;   /* Max concurrent tasks */
    uint64_t npu_tops_curr;  /* Current TOPS allocation */
    uint64_t npu_task_curr;  /* Current task count */
} xdna2_resource_info_t;

/* ═══════════════════════════════════════════════════════════
 * Device / Context Management
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_open_device — Open the NPU accel node (/dev/accel/accel0)
 *
 * Returns 0 on success, -1 on failure.
 * Sets *fd_ptr to the file descriptor.
 */
int xdna2_open_device(int *fd_ptr);

/**
 * xdna2_close_device — Close NPU device
 */
void xdna2_close_device(int fd);

/**
 * xdna2_create_hwctx — Create hardware context via DRM ioctl
 *
 * Allocates the client device heap (see XDNA2_DEV_HEAP_MAX_BYTES), then the
 * AIE tiles and the user mode queue. The heap is owned by the context and
 * released by xdna2_destroy_hwctx().
 * Returns 0 on success, negative on failure.
 */
int xdna2_create_hwctx(int fd, xdna2_hwctx_t *ctx,
                       uint32_t num_tiles, uint32_t mem_size,
                       uint32_t max_opc, uint32_t qos);

/**
 * xdna2_destroy_hwctx — Destroy hardware context
 */
int xdna2_destroy_hwctx(int fd, xdna2_hwctx_t *ctx);

/* ═══════════════════════════════════════════════════════════
 * Buffer Object Management
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_create_bo — Create a buffer object
 *
 * type: one of enum amdxdna_bo_type — AMDXDNA_BO_SHMEM (host-visible; the
 * AMDXDNA_BO_SHARE alias only exists in newer headers), AMDXDNA_BO_DEV_HEAP,
 * AMDXDNA_BO_DEV (carved out of the heap) or AMDXDNA_BO_CMD.
 */
int xdna2_create_bo(int fd, xdna2_bo_t *bo, uint64_t size, uint32_t type);

/**
 * xdna2_destroy_bo — Unmap and release a buffer object (DRM_IOCTL_GEM_CLOSE)
 */
int xdna2_destroy_bo(int fd, xdna2_bo_t *bo);

/**
 * xdna2_map_bo — mmap a BO for host access
 */
int xdna2_map_bo(int fd, xdna2_bo_t *bo);

/**
 * xdna2_unmap_bo — munmap a BO
 */
int xdna2_unmap_bo(xdna2_bo_t *bo);

/**
 * xdna2_sync_bo — Sync buffer to/from device
 *
 * direction: SYNC_DIRECT_TO_DEVICE or SYNC_DIRECT_FROM_DEVICE
 */
int xdna2_sync_bo(int fd, xdna2_bo_t *bo, uint32_t direction,
                  uint64_t offset, uint64_t size);

/* ═══════════════════════════════════════════════════════════
 * Command Submission
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_submit_command — Submit an exec buffer command
 *
 * cmd_bo_handle:   handle of the AMDXDNA_BO_CMD buffer holding the packet.
 * arg_bo_handles:  array of BO handles referenced by the command (may be NULL).
 * arg_count:       number of entries in arg_bo_handles.
 *
 * On success the returned command sequence number is stored in ctx->last_seq.
 * Returns 0 on success, negative on failure.
 */
int xdna2_submit_command(int fd, xdna2_hwctx_t *ctx,
                         uint32_t cmd_bo_handle, uint32_t *arg_bo_handles,
                         uint32_t arg_count);

/**
 * xdna2_wait_command — Block until ctx->last_seq has completed
 *
 * Waits on the hardware context's timeline syncobj at point ctx->last_seq;
 * the mainline amdxdna UAPI exposes no WAIT_CMD ioctl.
 * timeout_ms: milliseconds to wait; 0 means wait forever.
 * Returns 0 on success, negative on failure or timeout. It never reports
 * success without the command having completed.
 */
int xdna2_wait_command(int fd, xdna2_hwctx_t *ctx, uint32_t timeout_ms);

/* ═══════════════════════════════════════════════════════════
 * Device Query
 * ═══════════════════════════════════════════════════════════ */

/**
 * xdna2_query_aie_metadata — Get AIE tile layout
 */
int xdna2_query_aie_metadata(int fd, xdna2_aie_metadata_t *meta);

/**
 * xdna2_query_resource_info — Get NPU resource limits
 */
int xdna2_query_resource_info(int fd, xdna2_resource_info_t *info);

/**
 * xdna2_query_firmware_version — Get firmware version
 */
int xdna2_query_firmware_version(int fd, uint32_t *major,
                                 uint32_t *minor, uint32_t *patch,
                                 uint32_t *build);

/**
 * xdna2_print_device_info — Print NPU device summary
 */
void xdna2_print_device_info(int fd);

#endif /* XDNA2_DRIVER_H */