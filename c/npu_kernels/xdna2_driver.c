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
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <time.h>
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

/*
 * The user address the driver associates with a BO, exactly as GET_BO_INFO
 * reports it: XDNA2_INVALID_ADDR when the driver holds no address, which is a
 * meaningful state and must not be folded into 0 here.
 */
static int xdna2_bo_userptr(int fd, uint32_t handle, uint64_t *uva) {
    struct amdxdna_drm_get_bo_info info;
    memset(&info, 0, sizeof(info));
    info.handle = handle;
    if (xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_GET_BO_INFO, &info) < 0) {
        return -1;
    }
    if (uva) *uva = info.vaddr;
    return 0;
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

/* Defined with the other BO helpers below. `force_mmap` ignores any user
 * address the driver already reports and insists on a real mmap(); `align`
 * constrains the address that mmap() hands back (0 or 1 for "page alignment is
 * enough"). */
static int xdna2_map_bo_ex(int fd, xdna2_bo_t *bo, bool force_mmap,
                           uint64_t align);

/*
 * The amdxdna client owns exactly one device heap. `aie2_hwctx_init()` fails
 * with -ENOENT ("The client dev heap object not exist") when DRM_IOCTL_
 * AMDXDNA_CREATE_HWCTX is issued before that heap exists, and every
 * AMDXDNA_BO_DEV allocation (the instruction streams) is carved out of it.
 * The heap must therefore be created first and outlive the context.
 *
 * The kernel caps the heap at dev_info->dev_mem_size, which is 64 MiB
 * (AIE2_DEVM_SIZE) on every AIE2 part including Strix Halo, and rounds device
 * allocations to dev_mem_buf_shift = 32 KiB.
 *
 * The size itself must be a whole number of dev_mem_size chunks: that is what
 * the driver checks when it creates the heap BO and what `aie2_map_host_buf()`
 * hands to the firmware one chunk at a time. A smaller heap is refused rather
 * than silently rounded, because a rounded-down size would fail later, inside
 * CREATE_HWCTX, as a bare -EINVAL.
 */
static uint64_t xdna2_dev_heap_bytes(void) {
    const char *env = getenv("XDNA2_HEAP_BYTES");
    if (env && env[0]) {
        char *end = NULL;
        unsigned long long value = strtoull(env, &end, 0);
        if (end && *end == '\0' && value >= XDNA2_DEV_HEAP_CHUNK_BYTES &&
            value <= XDNA2_DEV_HEAP_MAX_BYTES &&
            (value % XDNA2_DEV_HEAP_CHUNK_BYTES) == 0) {
            return (uint64_t)value;
        }
        fprintf(stderr,
                "xdna2: ignoring XDNA2_HEAP_BYTES='%s' (expected a multiple of "
                "%llu bytes, at most %llu)\n",
                env, (unsigned long long)XDNA2_DEV_HEAP_CHUNK_BYTES,
                (unsigned long long)XDNA2_DEV_HEAP_MAX_BYTES);
    }
    return XDNA2_DEV_HEAP_MAX_BYTES;
}

int xdna2_create_hwctx(int fd, xdna2_hwctx_t *ctx,
                       uint32_t num_tiles, uint32_t mem_size,
                       uint32_t max_opc, uint32_t qos) {
    if (!ctx) return -1;
    memset(ctx, 0, sizeof(*ctx));
    ctx->fd = -1;

    /* Device heap first: without it CREATE_HWCTX returns -ENOENT. */
    xdna2_bo_t heap_bo;
    if (xdna2_create_bo(fd, &heap_bo, xdna2_dev_heap_bytes(),
                        AMDXDNA_BO_DEV_HEAP) < 0) {
        fprintf(stderr,
                "xdna2: failed to create the %llu-byte device heap; the driver "
                "allows one heap per open file descriptor\n",
                (unsigned long long)xdna2_dev_heap_bytes());
        return -1;
    }

    /*
     * The heap must also be mapped into this process before the context is
     * created. The driver only records the heap's user address in its mmap
     * path (amdxdna_gem_obj_mmap -> amdxdna_hmm_register sets mem.userptr),
     * and amdxdna_drm_alloc_dev_bo() rejects every AMDXDNA_BO_DEV allocation
     * with -EINVAL ("Invalid dev heap userptr") while that address is still
     * AMDXDNA_INVALID_ADDR. aie2_hwctx_init() allocates its command buffers
     * exactly that way, so an unmapped heap turns CREATE_HWCTX into -EINVAL.
     *
     * The mapping is forced: a heap's user address exists *because* this
     * process mapped it, so an address reported before that has to be treated
     * as stale rather than as permission to skip the mmap.
     *
     * It is also aligned to the firmware's chunk size. `aie2_map_host_buf()`
     * hands the heap's user address to the firmware in `dev_mem_size`-sized
     * chunks (MSG_OP_MAP_HOST_BUFFER), and the firmware rejects a base that is
     * not a multiple of that chunk: the mgmt message fails, `aie2_hwctx_init()`
     * logs "Map host buffer failed" and CREATE_HWCTX returns -EINVAL. The
     * kernel applies the same alignment when it allocates the heap itself
     * (`align = dev_info->dev_mem_size` for AMDXDNA_BO_DEV_HEAP), and both XRT
     * and ROCr align their heap mapping the same way. A plain mmap() is only
     * page aligned, so it satisfies this by luck: that is why the context came
     * up in some processes and not in others on the same machine.
     */
    if (xdna2_map_bo_ex(fd, &heap_bo, /* force_mmap */ true,
                        /* align */ XDNA2_DEV_HEAP_CHUNK_BYTES) < 0) {
        xdna2_destroy_bo(fd, &heap_bo);
        fprintf(stderr,
                "xdna2: failed to map the device heap; the driver needs the "
                "heap's user address before it can carve device BOs out of it\n");
        return -1;
    }

    /*
     * Confirm the driver actually recorded that address. Without this the only
     * evidence of a heap the kernel considers unmapped is a bare -EINVAL from
     * CREATE_HWCTX several calls later, with the real cause visible only in
     * dmesg on the host.
     */
    uint64_t heap_uva = XDNA2_INVALID_ADDR;
    if (xdna2_bo_userptr(fd, heap_bo.handle, &heap_uva) < 0 ||
        heap_uva == XDNA2_INVALID_ADDR) {
        fprintf(stderr,
                "xdna2: the device heap is mapped at %p but the driver still "
                "reports no user address for it; every AMDXDNA_BO_DEV "
                "allocation, and so CREATE_HWCTX, would fail with -EINVAL "
                "(\"Invalid dev heap userptr\")\n",
                heap_bo.mapped);
        xdna2_destroy_bo(fd, &heap_bo);
        return -1;
    }

    /* The user mode queue and log buffer BOs must stay alive for as long as
     * the hardware context references them, so they are owned by the context
     * and released in xdna2_destroy_hwctx(). */
    xdna2_bo_t umq_bo;
    if (xdna2_create_bo(fd, &umq_bo, 4096, AMDXDNA_BO_CMD) < 0) {
        xdna2_destroy_bo(fd, &heap_bo);
        fprintf(stderr, "xdna2: failed to create UMQ BO\n");
        return -1;
    }

    xdna2_bo_t log_bo;
    if (xdna2_create_bo(fd, &log_bo, 4096, AMDXDNA_BO_CMD) < 0) {
        xdna2_destroy_bo(fd, &umq_bo);
        xdna2_destroy_bo(fd, &heap_bo);
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
        int err = errno;
        /* Read the heap state the driver held at the moment it refused, before
         * the BOs are released: whether the heap was still mapped decides
         * between a client-side bookkeeping fault and a partitioning refusal. */
        uint64_t heap_uva_now = XDNA2_INVALID_ADDR;
        int heap_known = (xdna2_bo_userptr(fd, heap_bo.handle, &heap_uva_now) == 0);
        xdna2_destroy_bo(fd, &log_bo);
        xdna2_destroy_bo(fd, &umq_bo);
        xdna2_destroy_bo(fd, &heap_bo);
        fprintf(stderr, "xdna2: failed to create hardware context: %s\n",
                strerror(err));
        fprintf(stderr,
                "xdna2: requested num_tiles=%u mem_size=%u max_opc=%u qos=%u\n",
                num_tiles, mem_size, max_opc, qos);
        if (err == ENOENT) {
            fprintf(stderr,
                    "xdna2: -ENOENT from CREATE_HWCTX means the driver could not "
                    "find this client's device heap, not that the ioctl is "
                    "unsupported (that would be ENOTTY)\n");
        } else if (err == EINVAL) {
            if (!heap_known || heap_uva_now == XDNA2_INVALID_ADDR) {
                fprintf(stderr,
                        "xdna2: the driver reports no user address for the "
                        "device heap now, though it reported 0x%" PRIx64 " right "
                        "after the mapping; the heap registration was dropped "
                        "between the two calls\n",
                        heap_uva);
            } else {
                fprintf(stderr,
                        "xdna2: the device heap is still registered at "
                        "0x%" PRIx64 ", so the rejection is not the "
                        "\"Invalid dev heap userptr\" case\n",
                        heap_uva_now);
            }
            /* Report the geometry the driver derives the partition from rather
             * than asserting a cause: the column count is only one of the ways
             * context init can return -EINVAL, and guessing at the reason is
             * how the previous revisions of this file went wrong. */
            xdna2_aie_metadata_t meta;
            if (xdna2_query_aie_metadata(fd, &meta) == 0 &&
                meta.core.row_count != 0) {
                unsigned core_rows = (unsigned)meta.core.row_count;
                unsigned num_col = num_tiles / core_rows;
                fprintf(stderr,
                        "xdna2: AIE array is %u columns x %u rows, %u core rows; "
                        "num_tiles=%u implies %u column(s)%s\n",
                        (unsigned)meta.cols, (unsigned)meta.rows, core_rows,
                        num_tiles, num_col,
                        (num_tiles % core_rows) ? " (not a whole multiple of "
                        "the core row count)" : "");
                if (num_col != 0 && num_col <= (unsigned)meta.cols &&
                    (num_tiles % core_rows) == 0) {
                    fprintf(stderr,
                            "xdna2: that partition is within range, so -EINVAL "
                            "came from something other than the column count\n");
                }
            }
            fprintf(stderr,
                    "xdna2: the driver logs the exact rejection; check "
                    "`dmesg | grep -i amdxdna` on the host. "
                    "\"Invalid dev heap userptr\" there means the device heap "
                    "was not mapped before the context was created; "
                    "\"Map host buffer failed\" means the firmware refused the "
                    "heap's user address, which must be a multiple of %llu "
                    "bytes\n",
                    (unsigned long long)XDNA2_DEV_HEAP_CHUNK_BYTES);
        }
        return -1;
    }

    ctx->fd = fd;
    ctx->hwctx_handle = create_ctx.handle;
    ctx->dev_heap_bo = heap_bo;
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

    /* Release the BOs the context was holding. The device heap is released
     * last: device BOs are sub-allocations of it. */
    xdna2_destroy_bo(fd, &ctx->log_bo);
    xdna2_destroy_bo(fd, &ctx->umq_bo);
    xdna2_destroy_bo(fd, &ctx->dev_heap_bo);

    ctx->initialized = false;
    ctx->hwctx_handle = 0;
    ctx->syncobj_handle = 0;
    ctx->last_seq = 0;
    return ret < 0 ? -1 : 0;
}

int xdna2_config_hwctx_single_cu(int fd, xdna2_hwctx_t *ctx,
                                 uint32_t cu_bo_handle, uint8_t cu_func) {
    if (!ctx || !ctx->initialized || cu_bo_handle == 0) return -1;
#if defined(DRM_IOCTL_AMDXDNA_CONFIG_HWCTX) && \
    defined(DRM_AMDXDNA_HWCTX_CONFIG_CU)
    struct {
        struct amdxdna_hwctx_param_config_cu hdr;
        struct amdxdna_cu_config cu;
    } cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.hdr.num_cus = 1;
    cfg.cu.cu_bo = cu_bo_handle;
    cfg.cu.cu_func = cu_func;

    struct amdxdna_drm_config_hwctx req;
    memset(&req, 0, sizeof(req));
    req.handle = ctx->hwctx_handle;
    req.param_type = DRM_AMDXDNA_HWCTX_CONFIG_CU;
    req.param_val = (__u64)(uintptr_t)&cfg;
    req.param_val_size = (uint32_t)sizeof(cfg);

    if (xdna2_ioctl(fd, DRM_IOCTL_AMDXDNA_CONFIG_HWCTX, &req) < 0) {
        fprintf(stderr,
                "xdna2: failed to configure CU on hwctx=%u with BO=%u\n",
                ctx->hwctx_handle, cu_bo_handle);
        return -1;
    }
    return 0;
#else
    (void)fd;
    (void)ctx;
    (void)cu_bo_handle;
    (void)cu_func;
    fprintf(stderr,
            "xdna2: this kernel UAPI lacks DRM_IOCTL_AMDXDNA_CONFIG_HWCTX; "
            "cannot register CU/PDI on this build\n");
    errno = ENOTSUP;
    return -1;
#endif
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
    /*
     * The driver reports "no address" as AMDXDNA_INVALID_ADDR (~0), not 0:
     * `vaddr` is that sentinel until the BO is mmap()ed, and `map_offset` is
     * that sentinel for AMDXDNA_BO_DEV, which is a sub-allocation of the heap
     * and has no mmap offset of its own. Storing the sentinel verbatim made
     * xdna2_map_bo() hand out (void *)-1 as a host pointer.
     */
    bo->map_offset = (get_info.map_offset == XDNA2_INVALID_ADDR)
                         ? 0 : get_info.map_offset;
    bo->vaddr = (get_info.vaddr == XDNA2_INVALID_ADDR) ? 0 : get_info.vaddr;
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

/*
 * mmap() a BO at an address that is a multiple of `align`.
 *
 * mmap() only guarantees page alignment, and there is no portable way to ask
 * for more, so a slack region of `size + align - 1` bytes is reserved first and
 * the real mapping is placed over its aligned interior with MAP_FIXED. The
 * unused head and tail are released afterwards, which keeps munmap(mapped,
 * size) correct at teardown. MAP_FIXED lands inside a reservation this call
 * owns, so it can never replace an unrelated mapping.
 */
static void *xdna2_mmap_aligned(int fd, uint64_t size, uint64_t align,
                                uint64_t offset) {
    if (align <= 1) {
        return mmap(NULL, (size_t)size, PROT_READ | PROT_WRITE,
                    MAP_SHARED, fd, (off_t)offset);
    }

    size_t reserve = (size_t)size + (size_t)align - 1;
    char *base = mmap(NULL, reserve, PROT_NONE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (base == MAP_FAILED) {
        return MAP_FAILED;
    }

    char *aligned = (char *)(((uintptr_t)base + (uintptr_t)align - 1) &
                             ~((uintptr_t)align - 1));
    void *addr = mmap(aligned, (size_t)size, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_FIXED, fd, (off_t)offset);
    if (addr == MAP_FAILED) {
        munmap(base, reserve);
        return MAP_FAILED;
    }

    if (aligned > base) {
        munmap(base, (size_t)(aligned - base));
    }
    char *tail = aligned + size;
    if (tail < base + reserve) {
        munmap(tail, (size_t)((base + reserve) - tail));
    }
    return addr;
}

static int xdna2_map_bo_ex(int fd, xdna2_bo_t *bo, bool force_mmap,
                           uint64_t align) {
    if (!bo) return -1;
    if (bo->mapped) return 0;

    /* Some BO types are already mapped into the process by the driver and
     * report the address directly. A caller that owns the registration itself
     * (the device heap) passes force_mmap and never takes this shortcut. */
    if (bo->vaddr && !force_mmap) {
        bo->mapped = (void *)(uintptr_t)bo->vaddr;
        return 0;
    }

    if (bo->map_offset == 0) {
        fprintf(stderr,
                "xdna2: BO %u (type %u) has neither a user address nor an mmap "
                "offset; a device BO is only reachable through its heap\n",
                bo->handle, bo->type);
        return -1;
    }

    void *addr = xdna2_mmap_aligned(fd, bo->size, align, bo->map_offset);
    if (addr == MAP_FAILED) {
        fprintf(stderr, "xdna2: mmap failed for BO %u: %s\n",
                bo->handle, strerror(errno));
        return -1;
    }

    bo->mapped = addr;
    bo->owns_mapping = true;
    return 0;
}

int xdna2_map_bo(int fd, xdna2_bo_t *bo) {
    return xdna2_map_bo_ex(fd, bo, /* force_mmap */ false, /* align */ 0);
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

    /*
     * Completion is reported through the per-context *timeline* syncobj whose
     * handle CREATE_HWCTX returned: the kernel calls
     * drm_syncobj_add_point(syncobj, chain, out_fence, seq) for every job, so
     * the sequence number returned by EXEC_CMD is the timeline point to wait
     * on. This is the only wait mechanism the in-tree driver exposes —
     * DRM_IOCTL_AMDXDNA_WAIT_CMD does not exist in the mainline UAPI (it is an
     * out-of-tree amd/xdna-driver addition), which is why building against
     * distribution kernel headers fails to find it.
     *
     * Returning success without waiting is never an option: the caller reads
     * the output BO immediately afterwards and would silently consume whatever
     * happened to be in memory.
     */
    if (ctx->syncobj_handle == 0 || ctx->syncobj_handle == UINT32_MAX) {
        fprintf(stderr,
                "xdna2: hardware context has no completion syncobj; cannot wait "
                "for command seq %" PRIu64 "\n", (uint64_t)ctx->last_seq);
        return -1;
    }

    /* drm_syncobj_timeline_wait takes an absolute CLOCK_MONOTONIC deadline. */
    int64_t deadline_ns = INT64_MAX;
    if (timeout_ms != 0) {
        struct timespec now;
        if (clock_gettime(CLOCK_MONOTONIC, &now) != 0) {
            fprintf(stderr, "xdna2: clock_gettime failed: %s\n", strerror(errno));
            return -1;
        }
        deadline_ns = (int64_t)now.tv_sec * 1000000000ll + (int64_t)now.tv_nsec +
                      (int64_t)timeout_ms * 1000000ll;
    }

    uint32_t handle = ctx->syncobj_handle;
    uint64_t point = ctx->last_seq;

    struct drm_syncobj_timeline_wait wait;
    memset(&wait, 0, sizeof(wait));
    wait.handles = (__u64)(uintptr_t)&handle;
    wait.points = (__u64)(uintptr_t)&point;
    wait.timeout_nsec = deadline_ns;
    wait.count_handles = 1;
    wait.flags = DRM_SYNCOBJ_WAIT_FLAGS_WAIT_ALL |
                 DRM_SYNCOBJ_WAIT_FLAGS_WAIT_FOR_SUBMIT;

    if (xdna2_ioctl(fd, DRM_IOCTL_SYNCOBJ_TIMELINE_WAIT, &wait) < 0) {
        fprintf(stderr,
                "xdna2: wait for command seq %" PRIu64 " failed after %u ms\n",
                point, timeout_ms);
        return -1;
    }
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

/*
 * The NPU generation is a property of the part, not of the firmware protocol.
 * `struct amdxdna_drm_query_aie_metadata.version` is the AIE tile-info version
 * the firmware answers with — Strix Halo, an XDNA 2 part, reports 1.1 there —
 * so it cannot be used to tell AIE-2 silicon from AIE-1 silicon. The PCI
 * identity can: it is exactly what amdxdna's own device table matches on.
 * There is no DRM query for it, so it is read from sysfs for the character
 * device behind the fd; a host without /sys leaves the generation unknown
 * rather than assumed.
 */
static int xdna2_read_sysfs_u32(const char *path, uint32_t *out) {
    FILE *f = fopen(path, "re");
    if (!f) return -1;
    unsigned long value = 0;
    int fields = fscanf(f, "%lx", &value); /* sysfs prints "0x1022" */
    fclose(f);
    if (fields != 1) return -1;
    *out = (uint32_t)value;
    return 0;
}

int xdna2_query_pci_ids(int fd, xdna2_pci_ids_t *ids) {
    if (!ids || fd < 0) return -1;

    struct stat st;
    if (fstat(fd, &st) != 0 || !S_ISCHR(st.st_mode)) {
        return -1;
    }

    char base[128];
    int n = snprintf(base, sizeof(base), "/sys/dev/char/%u:%u/device",
                     (unsigned)major(st.st_rdev), (unsigned)minor(st.st_rdev));
    if (n <= 0 || (size_t)n >= sizeof(base)) return -1;

    xdna2_pci_ids_t out;
    memset(&out, 0, sizeof(out));

    static const char *const attrs[3] = {"vendor", "device", "revision"};
    uint32_t *slots[3] = {&out.vendor, &out.device, &out.revision};
    for (int i = 0; i < 3; ++i) {
        char path[192];
        n = snprintf(path, sizeof(path), "%s/%s", base, attrs[i]);
        if (n <= 0 || (size_t)n >= sizeof(path)) return -1;
        if (xdna2_read_sysfs_u32(path, slots[i]) != 0) return -1;
    }

    *ids = out;
    return 0;
}

int xdna2_is_xdna2_hardware(int fd) {
    xdna2_pci_ids_t ids;
    if (xdna2_query_pci_ids(fd, &ids) != 0) {
        return -1;
    }
    if (ids.vendor != XDNA2_PCI_VENDOR_AMD) {
        return 0;
    }
    /* Every XDNA 2 part shipped so far shares one device id and differs only
     * in the revision; the XDNA 1 parts use different device ids entirely. */
    return ids.device == XDNA2_PCI_DEVICE_NPU4 ? 1 : 0;
}

/*
 * DRM_AMDXDNA_QUERY_RESOURCE_INFO and struct amdxdna_drm_get_resource_info
 * were added to the amdxdna UAPI after Linux 6.18, so they are absent from
 * every currently shipping kernel-headers package. The build system probes for
 * them and defines COLI_HAVE_XDNA2_RESOURCE_INFO when present; nothing is
 * re-declared locally, and when the query is unavailable the function reports
 * an explicit failure instead of inventing values.
 */
#ifdef COLI_HAVE_XDNA2_RESOURCE_INFO
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
#else
int xdna2_query_resource_info(int fd, xdna2_resource_info_t *info) {
    (void)fd;
    if (!info) return -1;
    memset(info, 0, sizeof(*info));
    fprintf(stderr,
            "xdna2: DRM_AMDXDNA_QUERY_RESOURCE_INFO is not present in these "
            "kernel headers; resource info is unavailable\n");
    return -1;
}
#endif

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

    xdna2_pci_ids_t ids;
    if (xdna2_query_pci_ids(fd, &ids) == 0) {
        printf("PCI: %04x:%04x rev %02x (%s)\n", ids.vendor, ids.device,
               ids.revision,
               xdna2_is_xdna2_hardware(fd) == 1 ? "XDNA 2" : "not XDNA 2");
    }

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
