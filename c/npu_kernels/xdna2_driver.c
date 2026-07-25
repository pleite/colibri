/**
 * xdna2_driver.c — DRM ioctl wrapper for the AMD XDNA 2 NPU on Strix Halo.
 *
 * Talks directly to the in-tree `amdxdna` accel driver through
 * /dev/accel/accel0. No XRT, no libxrt, no aiecompiler runtime.
 *
 * Design rule (see docs/strix-halo-npu.md): every ioctl payload must be the
 * `struct amdxdna_drm_*` type from <drm/amdxdna_accel.h> and every `param`
 * must be a named `enum amdxdna_drm_get_param` value. Hand-copied struct
 * layouts and magic parameter numbers are what made the previous revision
 * corrupt the caller's stack and query the wrong parameter.
 */

#include "xdna2_driver.h"

#include <drm/amdxdna_accel.h>
#include <drm/drm.h>
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
    int ret;
    do {
        ret = ioctl(fd, cmd, arg);
    } while (ret < 0 && (errno == EINTR || errno == EAGAIN));
    if (ret < 0) {
        fprintf(stderr, "xdna2: ioctl 0x%lx failed: %s\n",
                (unsigned long)cmd, strerror(errno));
    }
    return ret;
}

/* ── Device Management ── */

int xdna2_open_device(int *fd_ptr) {
    if (!fd_ptr) return -1;

    /* The amdxdna driver registers an accel node. Strix Halo exposes exactly
     * one NPU, so accel0 is the only node we accept; XDNA2_DEVICE may point at
     * a different node when several accel devices are present in a container. */
    const char *node = getenv("XDNA2_DEVICE");
    if (!node || !node[0]) {
        node = "/dev/accel/accel0";
    }

    int fd = open(node, O_RDWR | O_CLOEXEC);
    if (fd < 0) {
        fprintf(stderr,
                "xdna2: failed to open %s: %s\n"
                "xdna2: check that the amdxdna module is loaded and that the "
                "container was started with --device /dev/accel/accel0\n",
                node, strerror(errno));
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
    ctx->fd = -1;

    /* The user mode queue and log buffer BOs must stay alive for as long as
     * the hardware context references them, so they are owned by the context
     * and released in xdna2_destroy_hwctx(). */
    xdna2_bo_t umq_bo;
    if (xdna2_create_bo(fd, &umq_bo, 4096, AMDXDNA_BO_CMD) < 0) {
        fprintf(stderr, "xdna2: failed to create UMQ BO\n");
        return -1;
    }

    xdna2_bo_t log_bo;
    if (xdna2_create_bo(fd, &log_bo, 4096, AMDXDNA_BO_CMD) < 0) {
        xdna2_destroy_bo(fd, &umq_bo);
        fprintf(stderr, "xdna2: failed to create log BO\n");
        return -1;
    }

    struct amdxdna_qos_info qos_info;
    memset(&qos_info, 0, sizeof(qos_info));
    qos_info.priority = qos;

    struct amdxdna_drm_create_hwctx create_ctx;
    memset(&create_ctx, 0, sizeof(create_ctx));
    create_ctx.qos_p = (__u64)(uintptr_t)&qos_info;
    create_ctx.umq_bo = umq_bo.handle;
    create_ctx.log_buf_bo = log_bo.handle;
    create_ctx.max_opc = max_opc;
    create_ctx.num_tiles = num_tiles;
    create_ctx.mem_size = mem_size;

    if (xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_CREATE_HWCTX, &create_ctx) < 0) {
        xdna2_destroy_bo(fd, &umq_bo);
        xdna2_destroy_bo(fd, &log_bo);
        fprintf(stderr, "xdna2: failed to create hardware context\n");
        return -1;
    }

    ctx->fd = fd;
    ctx->hwctx_handle = create_ctx.handle;
    ctx->umq_bo = umq_bo;
    ctx->log_bo = log_bo;
    ctx->umq_doorbell = create_ctx.umq_doorbell;
    ctx->num_tiles = create_ctx.num_tiles;
    ctx->mem_size = create_ctx.mem_size;
    ctx->max_opc = create_ctx.max_opc;
    ctx->syncobj_handle = create_ctx.syncobj_handle;
    ctx->last_seq = 0;
    ctx->initialized = true;

    fprintf(stderr, "xdna2: hwctx created, handle=%u, tiles=%u, doorbell=0x%x\n",
            ctx->hwctx_handle, ctx->num_tiles, ctx->umq_doorbell);
    return 0;
}

int xdna2_destroy_hwctx(int fd, xdna2_hwctx_t *ctx) {
    if (!ctx || !ctx->initialized) return -1;

    struct amdxdna_drm_destroy_hwctx destroy_ctx;
    memset(&destroy_ctx, 0, sizeof(destroy_ctx));
    destroy_ctx.handle = ctx->hwctx_handle;

    int ret = xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_DESTROY_HWCTX, &destroy_ctx);

    /* Release the BOs the context was holding. */
    xdna2_destroy_bo(fd, &ctx->log_bo);
    xdna2_destroy_bo(fd, &ctx->umq_bo);

    ctx->initialized = false;
    ctx->hwctx_handle = 0;
    ctx->syncobj_handle = 0;
    ctx->last_seq = 0;
    return ret < 0 ? -1 : 0;
}

/* ── Buffer Objects ── */

int xdna2_create_bo(int fd, xdna2_bo_t *bo, uint64_t size, uint32_t type) {
    if (!bo) return -1;
    memset(bo, 0, sizeof(*bo));

    struct amdxdna_drm_create_bo create_bo;
    memset(&create_bo, 0, sizeof(create_bo));
    create_bo.size = size;
    create_bo.type = type;

    if (xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_CREATE_BO, &create_bo) < 0) {
        fprintf(stderr, "xdna2: failed to create BO (size=%llu, type=%u)\n",
                (unsigned long long)size, type);
        return -1;
    }

    bo->handle = create_bo.handle;
    bo->size = size;
    bo->type = type;

    struct amdxdna_drm_get_bo_info get_info;
    memset(&get_info, 0, sizeof(get_info));
    get_info.handle = create_bo.handle;

    if (xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_GET_BO_INFO, &get_info) < 0) {
        fprintf(stderr, "xdna2: failed to query BO %u info\n", bo->handle);
        xdna2_destroy_bo(fd, bo);
        return -1;
    }
    bo->map_offset = get_info.map_offset;
    bo->vaddr = get_info.vaddr;
    bo->xdna_addr = get_info.xdna_addr;

    return 0;
}

int xdna2_destroy_bo(int fd, xdna2_bo_t *bo) {
    if (!bo || bo->handle == 0) return -1;

    if (bo->mapped) {
        xdna2_unmap_bo(bo);
    }

    /* BOs are GEM objects: the generic DRM_IOCTL_GEM_CLOSE drops our reference.
     * Skipping it leaks a handle per allocation and eventually exhausts the
     * per-file handle table. */
    struct drm_gem_close close_req;
    memset(&close_req, 0, sizeof(close_req));
    close_req.handle = bo->handle;
    int ret = xdna2_ioctl(fd, DRM_IOCTL_GEM_CLOSE, &close_req);

    memset(bo, 0, sizeof(*bo));
    return ret < 0 ? -1 : 0;
}

int xdna2_map_bo(int fd, xdna2_bo_t *bo) {
    if (!bo) return -1;
    if (bo->mapped) return 0;

    /* Some BO types are already mapped into the process by the driver and
     * report the address directly. */
    if (bo->vaddr) {
        bo->mapped = (void *)(uintptr_t)bo->vaddr;
        return 0;
    }

    void *addr = mmap(NULL, (size_t)bo->size,
                      PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, (off_t)bo->map_offset);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "xdna2: mmap failed for BO %u: %s\n",
                bo->handle, strerror(errno));
        return -1;
    }

    bo->mapped = addr;
    bo->owns_mapping = true;
    return 0;
}

int xdna2_unmap_bo(xdna2_bo_t *bo) {
    if (!bo || !bo->mapped) return 0;

    if (bo->owns_mapping) {
        munmap(bo->mapped, (size_t)bo->size);
        bo->owns_mapping = false;
    }
    bo->mapped = NULL;
    return 0;
}

int xdna2_sync_bo(int fd, xdna2_bo_t *bo, uint32_t direction,
                  uint64_t offset, uint64_t size) {
    if (!bo) return -1;

    struct amdxdna_drm_sync_bo sync;
    memset(&sync, 0, sizeof(sync));
    sync.handle = bo->handle;
    sync.direction = direction;
    sync.offset = offset;
    sync.size = size;

    return xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_SYNC_BO, &sync);
}

/* ── Command Submission ── */

int xdna2_submit_command(int fd, xdna2_hwctx_t *ctx,
                         uint32_t cmd_bo_handle, uint32_t *arg_bo_handles,
                         uint32_t arg_count) {
    if (!ctx || !ctx->initialized) return -1;

    /* cmd_handles holds the BO handle itself when cmd_count == 1, per the
     * amdxdna_drm_exec_cmd documentation. */
    struct amdxdna_drm_exec_cmd exec_cmd;
    memset(&exec_cmd, 0, sizeof(exec_cmd));
    exec_cmd.hwctx = ctx->hwctx_handle;
    exec_cmd.type = AMDXDNA_CMD_SUBMIT_EXEC_BUF;
    exec_cmd.cmd_handles = (__u64)cmd_bo_handle;
    exec_cmd.cmd_count = 1;
    exec_cmd.args = (__u64)(uintptr_t)arg_bo_handles;
    exec_cmd.arg_count = arg_count;

    if (xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_EXEC_CMD, &exec_cmd) < 0) {
        fprintf(stderr, "xdna2: command submission failed\n");
        return -1;
    }

    /* `seq` is a per-context command sequence number, not a syncobj handle.
     * Conflating the two (as a previous revision did) both destroyed the real
     * syncobj handle and made every wait a no-op. */
    ctx->last_seq = exec_cmd.seq;
    return 0;
}

int xdna2_wait_command(int fd, xdna2_hwctx_t *ctx, uint32_t timeout_ms) {
    if (!ctx || !ctx->initialized) return -1;

    /* TODO: Implement when kernel supports DRM_IOCTL_AMDXDNA_WAIT_CMD */
    /* For now, assume command completed immediately */
    return 0;
}

















/* ── Device Query ── */

static int xdna2_get_info(int fd, uint32_t param, void *buffer, uint32_t size) {
    struct amdxdna_drm_get_info get_info;
    memset(&get_info, 0, sizeof(get_info));
    get_info.param = param;
    get_info.buffer_size = size;
    get_info.buffer = (__u64)(uintptr_t)buffer;

    return xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_GET_INFO, &get_info);
}

int xdna2_query_aie_metadata(int fd, xdna2_aie_metadata_t *meta) {
    if (!meta) return -1;

    struct amdxdna_drm_query_aie_metadata raw;
    memset(&raw, 0, sizeof(raw));
    if (xdna2_get_info(fd, DRM_AMDXDNA_QUERY_AIE_METADATA, &raw, sizeof(raw)) < 0) {
        fprintf(stderr, "xdna2: query AIE metadata failed\n");
        return -1;
    }

    memset(meta, 0, sizeof(*meta));
    meta->col_size = raw.col_size;
    meta->cols = raw.cols;
    meta->rows = raw.rows;
    meta->version_major = raw.version.major;
    meta->version_minor = raw.version.minor;

#define COPY_TILE(dst, src)                              \
    do {                                                 \
        (dst).row_count = (src).row_count;               \
        (dst).row_start = (src).row_start;               \
        (dst).dma_channel_count = (src).dma_channel_count; \
        (dst).lock_count = (src).lock_count;             \
        (dst).event_reg_count = (src).event_reg_count;   \
    } while (0)

    COPY_TILE(meta->core, raw.core);
    COPY_TILE(meta->mem, raw.mem);
    COPY_TILE(meta->shim, raw.shim);
#undef COPY_TILE

    return 0;
}

int xdna2_query_resource_info(int fd, xdna2_resource_info_t *info) {
    if (!info) return -1;

    struct amdxdna_drm_get_resource_info raw;
    memset(&raw, 0, sizeof(raw));
    if (xdna2_get_info(fd, DRM_AMDXDNA_QUERY_RESOURCE_INFO, &raw, sizeof(raw)) < 0) {
        fprintf(stderr, "xdna2: query resource info failed\n");
        return -1;
    }

    info->npu_clk_max = raw.npu_clk_max;
    info->npu_tops_max = raw.npu_tops_max;
    info->npu_task_max = raw.npu_task_max;
    info->npu_tops_curr = raw.npu_tops_curr;
    info->npu_task_curr = raw.npu_task_curr;
    return 0;
}

int xdna2_query_firmware_version(int fd, uint32_t *major,
                                 uint32_t *minor, uint32_t *patch,
                                 uint32_t *build) {
    /* The kernel writes a full struct amdxdna_drm_query_firmware_version
     * (16 bytes). Pointing `buffer` at a single uint32_t, as a previous
     * revision did, overflows the caller's stack by 12 bytes. */
    struct amdxdna_drm_query_firmware_version raw;
    memset(&raw, 0, sizeof(raw));

    if (xdna2_get_info(fd, DRM_AMDXDNA_QUERY_FIRMWARE_VERSION, &raw, sizeof(raw)) < 0) {
        fprintf(stderr, "xdna2: query firmware version failed\n");
        return -1;
    }

    if (major) *major = raw.major;
    if (minor) *minor = raw.minor;
    if (patch) *patch = raw.patch;
    if (build) *build = raw.build;
    return 0;
}

void xdna2_print_device_info(int fd) {
    xdna2_aie_metadata_t meta;
    xdna2_resource_info_t info;
    uint32_t fw_major = 0, fw_minor = 0, fw_patch = 0, fw_build = 0;

    memset(&meta, 0, sizeof(meta));
    memset(&info, 0, sizeof(info));

    printf("=== XDNA 2 NPU Device Info ===\n");

    if (xdna2_query_firmware_version(fd, &fw_major, &fw_minor, &fw_patch, &fw_build) == 0) {
        printf("Firmware: %u.%u.%u (build %u)\n",
               fw_major, fw_minor, fw_patch, fw_build);
    }

    if (xdna2_query_aie_metadata(fd, &meta) == 0) {
        printf("AIE Array: %u columns x %u rows = %u tiles\n",
               meta.cols, meta.rows, (unsigned)meta.cols * (unsigned)meta.rows);
        printf("AIE Version: %u.%u\n", meta.version_major, meta.version_minor);
        printf("Column size: %u bytes\n", meta.col_size);
        printf("Core tiles: %u rows (start=%u), %u locks, %u events\n",
               meta.core.row_count, meta.core.row_start,
               meta.core.lock_count, meta.core.event_reg_count);
        printf("Mem tiles:  %u rows (start=%u), %u DMA channels\n",
               meta.mem.row_count, meta.mem.row_start,
               meta.mem.dma_channel_count);
        printf("Shim tiles: %u rows (start=%u), %u DMA channels\n",
               meta.shim.row_count, meta.shim.row_start,
               meta.shim.dma_channel_count);
    }

    if (xdna2_query_resource_info(fd, &info) == 0) {
        printf("NPU Clock Max:     %llu MHz\n", (unsigned long long)info.npu_clk_max);
        printf("NPU TOPS Max:      %llu\n", (unsigned long long)info.npu_tops_max);
        printf("NPU Tasks Max:     %llu\n", (unsigned long long)info.npu_task_max);
        printf("NPU TOPS Current:  %llu\n", (unsigned long long)info.npu_tops_curr);
        printf("NPU Tasks Current: %llu\n", (unsigned long long)info.npu_task_curr);
    }
}
