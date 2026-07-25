/**
 * xdna2_driver.c — DRM ioctl wrapper for AMD XDNA 2 NPU
 *
 * Pure C implementation using direct DRM ioctls on /dev/dri/card1.
 * No XRT dependency. Works with any Linux kernel with amdxdna driver.
 *
 * Strix Halo: XDNA 2, 10×8 AIE array, ~30 TOPS INT8
 */

#include "xdna2_driver.h"

#include <drm/amdxdna_accel.h>
#include <linux/types.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <unistd.h>

/* ── Local ioctl helpers ── */

static int xdna2_ioctl(int fd, unsigned long cmd, void *arg) {
    int ret = ioctl(fd, cmd, arg);
    if (ret < 0) {
        fprintf(stderr, "xdna2: ioctl 0x%lx failed: %s\n",
                (unsigned long)cmd, strerror(errno));
    }
    return ret;
}

/* ── Device Management ── */

int xdna2_open_device(int *fd_ptr) {
    if (!fd_ptr) return -1;

    int fd = open("/dev/accel/accel0", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "xdna2: failed to open /dev/accel/accel0: %s\n",
                strerror(errno));
        return -1;
    }

    *fd_ptr = fd;
    return 0;
}

void xdna2_close_device(int fd) {
    if (fd >= 0) {
        close(fd);
    }
}

/* ── Hardware Context ── */

int xdna2_create_hwctx(int fd, xdna2_hwctx_t *ctx,
                       uint32_t num_tiles, uint32_t mem_size,
                       uint32_t max_opc, uint32_t qos) {
    if (!ctx) return -1;
    memset(ctx, 0, sizeof(*ctx));

    /* Create user mode queue BO (required for command submission) */
    xdna2_bo_t umq_bo;
    int ret = xdna2_create_bo(fd, &umq_bo, 4096, AMDXDNA_BO_CMD);
    if (ret < 0) {
        fprintf(stderr, "xdna2: failed to create UMQ BO\n");
        return -1;
    }

    /* Create log buffer BO */
    xdna2_bo_t log_bo;
    ret = xdna2_create_bo(fd, &log_bo, 4096, AMDXDNA_BO_CMD);
    if (ret < 0) {
        xdna2_destroy_bo(fd, &umq_bo);
        fprintf(stderr, "xdna2: failed to create log BO\n");
        return -1;
    }

    /* Create hardware context via DRM ioctl */
    struct {
        __u64 ext;
        __u64 ext_flags;
        __u64 qos_p;
        __u32 umq_bo;
        __u32 log_buf_bo;
        __u32 max_opc;
        __u32 num_tiles;
        __u32 mem_size;
        __u32 umq_doorbell;
        __u32 handle;
        __u32 syncobj_handle;
    } create_ctx = {0};

    create_ctx.umq_bo = umq_bo.handle;
    create_ctx.log_buf_bo = log_bo.handle;
    create_ctx.max_opc = max_opc;
    create_ctx.num_tiles = num_tiles;
    create_ctx.mem_size = mem_size;

    /* QoS pointer — use stack variable */
    struct {
        __u32 gops;
        __u32 fps;
        __u32 dma_bandwidth;
        __u32 latency;
        __u32 frame_exec_time;
        __u32 priority;
    } qos_info = {0};
    qos_info.priority = qos;
    create_ctx.qos_p = (uint64_t)(uintptr_t)&qos_info;

    ret = xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_CREATE_HWCTX, &create_ctx);
    if (ret < 0) {
        xdna2_destroy_bo(fd, &umq_bo);
        xdna2_destroy_bo(fd, &log_bo);
        fprintf(stderr, "xdna2: failed to create hardware context\n");
        return -1;
    }

    ctx->fd = fd;
    ctx->hwctx_handle = create_ctx.handle;
    ctx->umq_bo_handle = umq_bo.handle;
    ctx->log_buf_bo = log_bo.handle;
    ctx->umq_doorbell = create_ctx.umq_doorbell;
    ctx->num_tiles = create_ctx.num_tiles;
    ctx->mem_size = create_ctx.mem_size;
    ctx->max_opc = create_ctx.max_opc;
    ctx->syncobj_handle = create_ctx.syncobj_handle;
    ctx->initialized = true;

    /* Destroy temporary BOs (they're internal to the driver now) */
    xdna2_destroy_bo(fd, &umq_bo);
    xdna2_destroy_bo(fd, &log_bo);

    fprintf(stderr, "xdna2: hwctx created, handle=%u, tiles=%u, doorbell=0x%x\n",
            ctx->hwctx_handle, ctx->num_tiles, ctx->umq_doorbell);
    return 0;
}

int xdna2_destroy_hwctx(int fd, xdna2_hwctx_t *ctx) {
    if (!ctx || !ctx->initialized) return -1;

    struct {
        __u32 handle;
        __u32 pad;
    } destroy_ctx = {
        .handle = ctx->hwctx_handle,
    };

    xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_DESTROY_HWCTX, &destroy_ctx);

    ctx->initialized = false;
    ctx->hwctx_handle = 0;
    ctx->syncobj_handle = 0;
    return 0;
}

/* ── Buffer Objects ── */

int xdna2_create_bo(int fd, xdna2_bo_t *bo, uint64_t size, uint32_t type) {
    if (!bo) return -1;
    memset(bo, 0, sizeof(*bo));

    struct {
        __u64 flags;
        __u64 vaddr;
        __u64 size;
        __u32 type;
        __u32 handle;
    } create_bo = {
        .size = size,
        .type = type,
    };

    int ret = xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_CREATE_BO, &create_bo);
    if (ret < 0) {
        fprintf(stderr, "xdna2: failed to create BO (size=%lu, type=%u)\n",
                (unsigned long)size, type);
        return -1;
    }

    bo->handle = create_bo.handle;
    bo->size = size;
    bo->type = type;

    /* Get BO info for mapping */
    struct {
        __u64 ext;
        __u64 ext_flags;
        __u32 handle;
        __u32 pad;
        __u64 map_offset;
        __u64 vaddr;
        __u64 xdna_addr;
    } get_info = {
        .handle = create_bo.handle,
    };

    xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_GET_BO_INFO, &get_info);
    bo->map_offset = get_info.map_offset;
    bo->vaddr = get_info.vaddr;
    bo->xdna_addr = get_info.xdna_addr;

    return 0;
}

int xdna2_destroy_bo(int fd, xdna2_bo_t *bo) {
    if (!bo || bo->handle == 0) return -1;

    /* Note: DRM doesn't have a DESTROY_BO ioctl for user-created BOs.
     * The driver manages BO lifecycle via context destruction.
     * For SHARE-type BOs, munmap is sufficient. */
    if (bo->mapped) {
        xdna2_unmap_bo(bo);
    }

    bo->handle = 0;
    return 0;
}

int xdna2_map_bo(int fd, xdna2_bo_t *bo) {
    if (!bo || bo->mapped) return 0;

    void *addr = mmap(NULL, bo->size,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, bo->map_offset);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "xdna2: mmap failed for BO %u: %s\n",
                bo->handle, strerror(errno));
        return -1;
    }

    bo->mapped = addr;
    return 0;
}

int xdna2_unmap_bo(xdna2_bo_t *bo) {
    if (!bo || !bo->mapped) return 0;

    munmap(bo->mapped, bo->size);
    bo->mapped = NULL;
    return 0;
}

int xdna2_sync_bo(int fd, xdna2_bo_t *bo, uint32_t direction,
                  uint64_t offset, uint64_t size) {
    if (!bo) return -1;

    struct {
        __u32 handle;
        __u32 direction;
        __u64 offset;
        __u64 size;
    } sync = {
        .handle = bo->handle,
        .direction = direction,
        .offset = offset,
        .size = size,
    };

    return xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_SYNC_BO, &sync);
}

/* ── Command Submission ── */

int xdna2_submit_command(int fd, xdna2_hwctx_t *ctx,
                         uint64_t cmd_handle, uint64_t *args,
                         uint32_t arg_count) {
    if (!ctx || !ctx->initialized) return -1;

    struct {
        __u64 ext;
        __u64 ext_flags;
        __u32 hwctx;
        __u32 type;
        __u64 cmd_handles;
        __u64 args;
        __u32 cmd_count;
        __u32 arg_count;
        __u64 seq;
    } exec_cmd = {
        .hwctx = ctx->hwctx_handle,
        .type = 0, /* AMDXDNA_CMD_SUBMIT_EXEC_BUF */
        .cmd_handles = cmd_handle,
        .args = (uint64_t)(uintptr_t)args,
        .cmd_count = 1,
        .arg_count = arg_count,
    };

    int ret = xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_EXEC_CMD, &exec_cmd);
    if (ret < 0) {
        fprintf(stderr, "xdna2: command submission failed\n");
        return -1;
    }

    ctx->syncobj_handle = exec_cmd.seq;
    return 0;
}

int xdna2_wait_command(int fd, uint32_t syncobj_handle, uint64_t timeout_ns) {
    (void)fd;
    (void)syncobj_handle;
    (void)timeout_ns;
    /* DRM syncobj wait requires drm_syncobj_wait ioctl.
     * For now, return success (non-blocking).
     * Full implementation needs libdrm syncobj support. */
    return 0;
}

/* ── Device Query ── */

int xdna2_query_aie_metadata(int fd, xdna2_aie_metadata_t *meta) {
    if (!meta) return -1;
    memset(meta, 0, sizeof(*meta));

    struct {
        __u32 param;
        __u32 buffer_size;
        __u64 buffer;
    } get_info = {
        .param = 1, /* DRM_AMDXDNA_QUERY_AIE_METADATA */
        .buffer_size = sizeof(*meta),
        .buffer = (uint64_t)(uintptr_t)meta,
    };

    int ret = xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_GET_INFO, &get_info);
    if (ret < 0) {
        fprintf(stderr, "xdna2: query AIE metadata failed\n");
        return -1;
    }

    return 0;
}

int xdna2_query_resource_info(int fd, xdna2_resource_info_t *info) {
    if (!info) return -1;
    memset(info, 0, sizeof(*info));

    struct {
        __u32 param;
        __u32 buffer_size;
        __u64 buffer;
    } get_info = {
        .param = 15, /* DRM_AMDXDNA_QUERY_RESOURCE_INFO */
        .buffer_size = sizeof(*info),
        .buffer = (uint64_t)(uintptr_t)info,
    };

    int ret = xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_GET_INFO, &get_info);
    if (ret < 0) {
        fprintf(stderr, "xdna2: query resource info failed\n");
        return -1;
    }

    return 0;
}

int xdna2_query_firmware_version(int fd, uint32_t *major,
                                  uint32_t *minor, uint32_t *patch) {
    if (!major || !minor || !patch) return -1;

    struct {
        __u32 param;
        __u32 buffer_size;
        __u64 buffer;
    } get_info = {
        .param = 8, /* DRM_AMDXDNA_QUERY_FIRMWARE_VERSION */
        .buffer_size = sizeof(struct { __u32 major, minor, patch, build; }),
        .buffer = (uint64_t)(uintptr_t)major, /* major, minor, patch, build packed */
    };

    int ret = xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_GET_INFO, &get_info);
    if (ret < 0) {
        fprintf(stderr, "xdna2: query firmware version failed\n");
        return -1;
    }

    if (minor) *minor = ((uint32_t *)get_info.buffer)[1];
    if (patch) *patch = ((uint32_t *)get_info.buffer)[2];

    return 0;
}

void xdna2_print_device_info(int fd) {
    xdna2_aie_metadata_t meta = {0};
    xdna2_resource_info_t info = {0};

    printf("=== XDNA 2 NPU Device Info ===\n");

    if (xdna2_query_aie_metadata(fd, &meta) == 0) {
        printf("AIE Array: %u columns × %u rows = %u tiles\n",
               meta.cols, meta.rows, meta.cols * meta.rows);
        printf("AIE Version: %u.%u\n", meta.version_major, meta.version_minor);
        printf("Core tiles: %u rows (start=%u), %u locks, %u events\n",
               meta.core.row_count, meta.core.row_start,
               meta.core.lock_count, meta.core.event_reg_count);
        printf("Mem tiles:  %u rows (start=%u), %u DMA channels\n",
               meta.mem.row_count, meta.mem.row_start,
               meta.mem.dma_channel_count);
    }

    if (xdna2_query_resource_info(fd, &info) == 0) {
        printf("NPU Clock Max:     %lu MHz\n", (unsigned long)info.npu_clk_max);
        printf("NPU TOPS Max:      %lu\n", (unsigned long)info.npu_tops_max);
        printf("NPU Tasks Max:     %lu\n", (unsigned long)info.npu_task_max);
        printf("NPU TOPS Current:  %lu\n", (unsigned long)info.npu_tops_curr);
        printf("NPU Tasks Current: %lu\n", (unsigned long)info.npu_task_curr);
    }
}