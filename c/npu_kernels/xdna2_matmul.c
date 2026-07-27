/**
 * xdna2_matmul.c — fixed-shape INT8 matmul on the AMD XDNA 2 NPU (Strix Halo).
 *
 * There is no CPU fallback anywhere in this file. If the NPU is absent, if the
 * hardware context cannot be created, or if no kernel is loaded for the exact
 * requested shape, the call fails and says why. See xdna2_matmul.h for the
 * rationale.
 */

#include "xdna2_matmul.h"
#include "xdna2_driver.h"

#include <drm/amdxdna_accel.h>

#include <errno.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ── ERT command packet ──────────────────────────────────────────────────────
 *
 * The AMDXDNA_BO_CMD buffer submitted through DRM_IOCTL_AMDXDNA_EXEC_CMD holds
 * an ERT packet: a 32-bit header followed by a CU mask and payload words. The
 * header packs, from bit 0: state(4) | custom(8) | count(11) | opcode(5) |
 * type(4). `count` is the number of payload words that follow the header.
 *
 * The opcode is *not* hardcoded here: it is carried by the .npukernel artifact
 * produced by the AIE toolchain, because it depends on the kernel's calling
 * convention. Guessing it in C would produce a binary that appears to work and
 * silently submits malformed packets.
 */

#define ERT_STATE_NEW 1u

typedef struct {
    uint32_t header;
    uint32_t cu_mask;
    uint32_t data[1]; /* variable length */
} ert_packet_t;

/* Compatible with XRT's ert.h layout for ERT_START_NPU (struct ert_npu_data). */
typedef struct {
    uint64_t instruction_buffer;
    uint32_t instruction_buffer_size;
    uint32_t instruction_prop_count;
} xdna2_ert_npu_data_t;

/* Kernel argument register payload after ert_npu_data, matching firmware ABI:
 * x, w, y device pointers as 64-bit values, then I and O. */
typedef struct {
    uint64_t x;
    uint64_t w;
    uint64_t y;
    uint32_t I;
    uint32_t O;
} xdna2_ert_kernel_args_t;

/* Full payload area written into ert_start_kernel_cmd::data[] for START_NPU. */
typedef struct {
    xdna2_ert_npu_data_t npu;
    xdna2_ert_kernel_args_t args;
} xdna2_ert_start_npu_payload_t;

_Static_assert(sizeof(xdna2_ert_npu_data_t) == 16, "ert_npu_data size mismatch");
_Static_assert(sizeof(xdna2_ert_kernel_args_t) == 32, "kernel arg payload size mismatch");
_Static_assert(offsetof(xdna2_ert_start_npu_payload_t, args) == 16,
               "START_NPU payload order mismatch");
_Static_assert((sizeof(xdna2_ert_start_npu_payload_t) % sizeof(uint32_t)) == 0,
               "START_NPU payload must be 32-bit word aligned");

static uint32_t ert_make_header(uint32_t opcode, uint32_t count) {
    return (ERT_STATE_NEW & 0xFu) |
           ((count & 0x7FFu) << 12) |
           ((opcode & 0x1Fu) << 23);
}

/* ── Kernel artifact loading ─────────────────────────────────────────────── */

static uint32_t read_le32(const uint8_t *p) {
    return (uint32_t)p[0] | ((uint32_t)p[1] << 8) |
           ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}

static char *sidecar_xclbin_path(const char *kernel_path) {
    if (!kernel_path) return NULL;
    size_t path_len = strlen(kernel_path);
    const char *dot = strrchr(kernel_path, '.');
    size_t stem_len = path_len;
    if (dot && strcmp(dot, ".npukernel") == 0) {
        stem_len = (size_t)(dot - kernel_path);
    }
    char *path = (char *)malloc(stem_len + sizeof(".xclbin"));
    if (!path) return NULL;
    memcpy(path, kernel_path, stem_len);
    memcpy(path + stem_len, ".xclbin", sizeof(".xclbin"));
    return path;
}

static void destroy_cu_config_bos(int fd, xdna2_kernel_t *kernel) {
    if (!kernel) return;
    for (uint32_t i = 0; i < kernel->cu_config_count; ++i) {
        xdna2_destroy_bo(fd, &kernel->cu_config_bos[i]);
    }
    kernel->cu_config_count = 0;
}

int xdna2_load_kernel(xdna2_runtime_t *runtime, const char *kernel_path) {
    if (!runtime || !runtime->initialized || !kernel_path) return -EINVAL;
    if (runtime->kernel_count >= XDNA2_MAX_KERNELS) {
        fprintf(stderr, "xdna2: kernel table full (%d entries)\n", XDNA2_MAX_KERNELS);
        return -ENOSPC;
    }

    FILE *fp = fopen(kernel_path, "rb");
    if (!fp) {
        fprintf(stderr, "xdna2: cannot open kernel '%s': %s\n",
                kernel_path, strerror(errno));
        return -ENOENT;
    }

    uint8_t header[XDNA2_KERNEL_HEADER_BYTES];
    if (fread(header, 1, sizeof(header), fp) != sizeof(header)) {
        fprintf(stderr, "xdna2: kernel '%s' is truncated\n", kernel_path);
        fclose(fp);
        return -EINVAL;
    }

    if (read_le32(header + 0) != XDNA2_KERNEL_MAGIC) {
        fprintf(stderr, "xdna2: kernel '%s' has bad magic\n", kernel_path);
        fclose(fp);
        return -EINVAL;
    }
    uint32_t version = read_le32(header + 4);
    if (version != XDNA2_KERNEL_VERSION) {
        fprintf(stderr, "xdna2: kernel '%s' has unsupported version %u (expected %u)\n",
                kernel_path, version, XDNA2_KERNEL_VERSION);
        fclose(fp);
        return -EINVAL;
    }

    xdna2_kernel_t entry;
    memset(&entry, 0, sizeof(entry));
    entry.ert_opcode      = read_le32(header + 8);
    entry.cu_mask         = read_le32(header + 12);
    entry.shape.rows      = (int32_t)read_le32(header + 16);
    entry.shape.inner_dim = (int32_t)read_le32(header + 20);
    entry.shape.out_cols  = (int32_t)read_le32(header + 24);
    entry.shape.fmt       = (int32_t)read_le32(header + 28);
    uint32_t instr_size   = read_le32(header + 32);
    entry.instr_words     = read_le32(header + 36);

    if (instr_size == 0 || (instr_size % 4u) != 0u) {
        fprintf(stderr, "xdna2: kernel '%s' has invalid instruction size %u\n",
                kernel_path, instr_size);
        fclose(fp);
        return -EINVAL;
    }
    if (entry.shape.rows <= 0 || entry.shape.inner_dim <= 0 || entry.shape.out_cols <= 0) {
        fprintf(stderr, "xdna2: kernel '%s' declares a non-positive shape\n", kernel_path);
        fclose(fp);
        return -EINVAL;
    }

    uint8_t *instr = (uint8_t *)malloc(instr_size);
    if (!instr) {
        fclose(fp);
        return -ENOMEM;
    }
    if (fread(instr, 1, instr_size, fp) != instr_size) {
        fprintf(stderr, "xdna2: kernel '%s' instruction stream is truncated\n", kernel_path);
        free(instr);
        fclose(fp);
        return -EINVAL;
    }
    fclose(fp);

    /* Upload the instruction stream into a device BO. */
    if (xdna2_create_bo(runtime->device_fd, &entry.instr_bo, instr_size, AMDXDNA_BO_DEV) < 0) {
        free(instr);
        return -EIO;
    }
    if (xdna2_map_bo(runtime->device_fd, &entry.instr_bo) < 0) {
        xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
        free(instr);
        return -EIO;
    }
    memcpy(entry.instr_bo.mapped, instr, instr_size);
    free(instr);

    if (xdna2_sync_bo(runtime->device_fd, &entry.instr_bo,
                      SYNC_DIRECT_TO_DEVICE, 0, instr_size) < 0) {
        xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
        return -EIO;
    }

    if (!runtime->cu_configured) {
        char *xclbin_path = sidecar_xclbin_path(kernel_path);
        if (!xclbin_path) {
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -ENOMEM;
        }
        FILE *xclfp = fopen(xclbin_path, "rb");
        if (!xclfp) {
            fprintf(stderr, "xdna2: sidecar xclbin '%s' not found for '%s'\n",
                    xclbin_path, kernel_path);
            free(xclbin_path);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -ENOENT;
        }
        if (fseek(xclfp, 0, SEEK_END) != 0) {
            fclose(xclfp);
            free(xclbin_path);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EIO;
        }
        long xcl_size_long = ftell(xclfp);
        if (xcl_size_long <= 0) {
            fclose(xclfp);
            free(xclbin_path);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EINVAL;
        }
        if (fseek(xclfp, 0, SEEK_SET) != 0) {
            fclose(xclfp);
            free(xclbin_path);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EIO;
        }
        size_t xcl_size = (size_t)xcl_size_long;
        uint8_t *xclbin = (uint8_t *)malloc(xcl_size);
        if (!xclbin) {
            fclose(xclfp);
            free(xclbin_path);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -ENOMEM;
        }
        if (fread(xclbin, 1, xcl_size, xclfp) != xcl_size) {
            fclose(xclfp);
            free(xclbin_path);
            free(xclbin);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EINVAL;
        }
        fclose(xclfp);

        xdna2_xclbin_ip_data_t *ip_entries = NULL;
        size_t ip_count = 0;
        if (xdna2_parse_xclbin_ip_layout(xclbin, xcl_size, &ip_entries, &ip_count) < 0) {
            fprintf(stderr, "xdna2: '%s' did not expose any IP_PS_KERNEL entries\n",
                    xclbin_path);
            free(xclbin_path);
            free(xclbin);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EINVAL;
        }
        if (ip_count > 32u) {
            fprintf(stderr, "xdna2: '%s' exposes too many (%zu) DPU CUs\n",
                    xclbin_path, ip_count);
            free(ip_entries);
            free(xclbin_path);
            free(xclbin);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EINVAL;
        }
        uint32_t expected_mask = 0;
        for (size_t i = 0; i < ip_count; ++i) {
            expected_mask |= (1u << i);
        }
        if (entry.cu_mask != expected_mask) {
            fprintf(stderr,
                    "xdna2: xclbin '%s' exposes %zu DPU CU(s) but kernel mask=0x%x\n",
                    xclbin_path, ip_count, entry.cu_mask);
            free(ip_entries);
            free(xclbin_path);
            free(xclbin);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EINVAL;
        }

        xdna2_cu_config_t *cu_configs = calloc(ip_count, sizeof(*cu_configs));
        if (!cu_configs) {
            free(ip_entries);
            free(xclbin_path);
            free(xclbin);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -ENOMEM;
        }
        for (size_t i = 0; i < ip_count; ++i) {
            if (entry.cu_config_count >= XDNA2_MAX_CU_CONFIG_BOS) {
                fprintf(stderr, "xdna2: too many CU config BOs for kernel '%s'\n",
                        kernel_path);
                free(cu_configs);
                free(ip_entries);
                free(xclbin_path);
                free(xclbin);
                destroy_cu_config_bos(runtime->device_fd, &entry);
                xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
                return -ENOSPC;
            }
            if (xdna2_create_bo(runtime->device_fd, &entry.cu_config_bos[entry.cu_config_count],
                                sizeof(ip_entries[i]), AMDXDNA_BO_DEV) < 0) {
                free(cu_configs);
                free(ip_entries);
                free(xclbin_path);
                free(xclbin);
                destroy_cu_config_bos(runtime->device_fd, &entry);
                xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
                return -EIO;
            }
            if (xdna2_map_bo(runtime->device_fd, &entry.cu_config_bos[entry.cu_config_count]) < 0) {
                xdna2_destroy_bo(runtime->device_fd, &entry.cu_config_bos[entry.cu_config_count]);
                free(cu_configs);
                free(ip_entries);
                free(xclbin_path);
                free(xclbin);
                destroy_cu_config_bos(runtime->device_fd, &entry);
                xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
                return -EIO;
            }
            memcpy(entry.cu_config_bos[entry.cu_config_count].mapped, &ip_entries[i],
                   sizeof(ip_entries[i]));
            if (xdna2_sync_bo(runtime->device_fd, &entry.cu_config_bos[entry.cu_config_count],
                              SYNC_DIRECT_TO_DEVICE, 0, sizeof(ip_entries[i])) < 0) {
                xdna2_destroy_bo(runtime->device_fd, &entry.cu_config_bos[entry.cu_config_count]);
                free(cu_configs);
                free(ip_entries);
                free(xclbin_path);
                free(xclbin);
                destroy_cu_config_bos(runtime->device_fd, &entry);
                xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
                return -EIO;
            }
            cu_configs[i].cu_bo_handle = entry.cu_config_bos[entry.cu_config_count].handle;
            cu_configs[i].cu_func = 0;
            entry.cu_config_count++;
        }
        if (xdna2_config_hwctx_cus(runtime->device_fd, &runtime->hwctx, cu_configs,
                                   (uint16_t)ip_count) < 0) {
            free(cu_configs);
            free(ip_entries);
            free(xclbin_path);
            free(xclbin);
            destroy_cu_config_bos(runtime->device_fd, &entry);
            xdna2_destroy_bo(runtime->device_fd, &entry.instr_bo);
            return -EIO;
        }
        free(cu_configs);
        free(ip_entries);
        free(xclbin_path);
        free(xclbin);
        runtime->cu_configured = true;
    }

    entry.loaded = true;
    runtime->kernels[runtime->kernel_count++] = entry;

    fprintf(stderr, "xdna2: loaded kernel '%s' shape=(%d,%d,%d) fmt=%d opcode=%u instr=%u bytes\n",
            kernel_path, entry.shape.rows, entry.shape.inner_dim,
            entry.shape.out_cols, entry.shape.fmt, entry.ert_opcode, instr_size);
    return 0;
}

const xdna2_kernel_t *xdna2_find_kernel(const xdna2_runtime_t *runtime,
                                        int rows, int inner_dim, int out_cols,
                                        int fmt) {
    if (!runtime) return NULL;
    for (int i = 0; i < runtime->kernel_count; ++i) {
        const xdna2_kernel_t *k = &runtime->kernels[i];
        if (k->loaded &&
            k->shape.rows == rows &&
            k->shape.inner_dim == inner_dim &&
            k->shape.out_cols == out_cols &&
            k->shape.fmt == fmt) {
            return k;
        }
    }
    return NULL;
}

/* ── Runtime management ──────────────────────────────────────────────────── */

int xdna2_runtime_init(xdna2_runtime_t *runtime) {
    if (!runtime) return -EINVAL;
    memset(runtime, 0, sizeof(*runtime));
    runtime->device_fd = -1;
    runtime->timeout_ms = 5000;

    /*
     * Pick the control plane before touching the device. This decides how the
     * NPU is validated (DRM ioctls only, or the official XRT + XDNA shim
     * stack); fixed-shape dispatch below is DRM in both cases. It never
     * selects a CPU path.
     */
    xdna2_control_plane_t requested = XDNA2_CONTROL_PLANE_AUTO;
    if (xdna2_control_plane_from_env(&requested) < 0) {
        return -EINVAL;
    }
    int ret = xdna2_control_plane_resolve(requested, &runtime->control_plane);
    if (ret < 0) {
        return ret;
    }

    if (xdna2_open_device(&runtime->device_fd) < 0) {
        return -ENODEV;
    }

    if (getenv("XDNA2_VERBOSE")) {
        printf("  Control plane: %s (%s)\n",
               xdna2_control_plane_name(runtime->control_plane),
               xdna2_xrt_status());
        xdna2_print_device_info(runtime->device_fd);
    }

    /*
     * Partition sizing for a matmul kernel. Strix Halo exposes a 10x8 AIE-2
     * array; 16 cores with 32 KiB of tile memory each is the smallest
     * partition that keeps a 4096-wide reduction resident.
     */
#ifdef AMDXDNA_QOS_HIGH_PRIORITY
    const uint32_t qos_priority = AMDXDNA_QOS_HIGH_PRIORITY;
#else
    /* The AMDXDNA_QOS_* priority hints post-date Linux 6.18 and are absent
     * from shipping kernel headers. Zero leaves the driver default in place;
     * the value is deliberately not hard-coded here. */
    const uint32_t qos_priority = 0;
#endif
    if (xdna2_create_hwctx(runtime->device_fd, &runtime->hwctx,
                           /* num_tiles */ 16,
                           /* mem_size  */ 32768,
                           /* max_opc   */ 4,
                           /* qos       */ qos_priority) < 0) {
        xdna2_close_device(runtime->device_fd);
        runtime->device_fd = -1;
        return -EIO;
    }

    runtime->initialized = true;
    return 0;
}

void xdna2_runtime_shutdown(xdna2_runtime_t *runtime) {
    if (!runtime || !runtime->initialized) return;

    for (int i = 0; i < runtime->kernel_count; ++i) {
        if (runtime->kernels[i].loaded) {
            destroy_cu_config_bos(runtime->device_fd, &runtime->kernels[i]);
            xdna2_destroy_bo(runtime->device_fd, &runtime->kernels[i].instr_bo);
            runtime->kernels[i].loaded = false;
        }
    }
    runtime->kernel_count = 0;

    xdna2_destroy_hwctx(runtime->device_fd, &runtime->hwctx);
    xdna2_close_device(runtime->device_fd);
    runtime->device_fd = -1;
    runtime->initialized = false;
}

/* ── Execution ───────────────────────────────────────────────────────────── */

typedef struct {
    xdna2_bo_t x;
    xdna2_bo_t w;
    xdna2_bo_t y;
    xdna2_bo_t cmd;
} matmul_bos_t;

static void free_bos(int fd, matmul_bos_t *bos) {
    xdna2_destroy_bo(fd, &bos->cmd);
    xdna2_destroy_bo(fd, &bos->y);
    xdna2_destroy_bo(fd, &bos->w);
    xdna2_destroy_bo(fd, &bos->x);
}

static int alloc_and_map(int fd, xdna2_bo_t *bo, size_t bytes, uint32_t type) {
    if (xdna2_create_bo(fd, bo, (uint64_t)bytes, type) < 0) return -EIO;
    if (xdna2_map_bo(fd, bo) < 0) {
        xdna2_destroy_bo(fd, bo);
        return -EIO;
    }
    return 0;
}

/* CLOCK_MONOTONIC, so the stage costs are wall clock and comparable with the
 * CPU and Vulkan backends measured by the same harness. */
static uint64_t now_ns(void) {
    struct timespec ts;
    if (clock_gettime(CLOCK_MONOTONIC, &ts) != 0) return 0;
    return (uint64_t)ts.tv_sec * 1000000000ull + (uint64_t)ts.tv_nsec;
}

uint64_t xdna2_timing_fixed_ns(const xdna2_matmul_timing_t *timing) {
    if (!timing) return 0;
    return timing->alloc_ns + timing->upload_ns + timing->submit_ns +
           timing->readback_ns + timing->teardown_ns;
}

int xdna2_matmul_int8(xdna2_runtime_t *runtime,
                      const int8_t *x, const int8_t *weights,
                      const float *scales, float *y,
                      int S, int I, int O) {
    return xdna2_matmul_int8_timed(runtime, x, weights, scales, y, S, I, O, NULL);
}

int xdna2_matmul_int8_timed(xdna2_runtime_t *runtime,
                            const int8_t *x, const int8_t *weights,
                            const float *scales, float *y,
                            int S, int I, int O,
                            xdna2_matmul_timing_t *timing) {
    if (timing) memset(timing, 0, sizeof(*timing));
    if (!runtime || !x || !weights || !y || S <= 0 || I <= 0 || O <= 0) {
        return -EINVAL;
    }
    if (!runtime->initialized) {
        fprintf(stderr, "xdna2: runtime is not initialised\n");
        return -ENODEV;
    }

    const xdna2_kernel_t *kernel = xdna2_find_kernel(runtime, S, I, O, XDNA2_FMT_INT8);
    if (!kernel) {
        /* Deliberate hard failure: AIE-2 is fixed-shape and this backend has no
         * fallback. The caller must load a kernel for this shape first. */
        fprintf(stderr,
                "xdna2: no NPU kernel loaded for shape (%d, %d, %d) fmt=int8; "
                "build one with the AIE toolchain and load it with "
                "xdna2_load_kernel()\n", S, I, O);
        return -ENOENT;
    }

    const size_t x_bytes = (size_t)S * (size_t)I;
    const size_t w_bytes = (size_t)O * (size_t)I;
    const size_t y_bytes = (size_t)S * (size_t)O * sizeof(float);

    matmul_bos_t bos;
    memset(&bos, 0, sizeof(bos));
    int fd = runtime->device_fd;
    int ret;
    const uint64_t t_start = now_ns();
    uint64_t t_mark = t_start;

    /* AMDXDNA_BO_SHMEM is the canonical name in enum amdxdna_bo_type; the
     * AMDXDNA_BO_SHARE alias (same value) only exists in newer headers. */
    if ((ret = alloc_and_map(fd, &bos.x, x_bytes, AMDXDNA_BO_SHMEM)) < 0) goto out;
    if ((ret = alloc_and_map(fd, &bos.w, w_bytes, AMDXDNA_BO_SHMEM)) < 0) goto out;
    if ((ret = alloc_and_map(fd, &bos.y, y_bytes, AMDXDNA_BO_SHMEM)) < 0) goto out;
    if (timing) { uint64_t t = now_ns(); timing->alloc_ns = t - t_mark; t_mark = t; }

    memcpy(bos.x.mapped, x, x_bytes);
    memcpy(bos.w.mapped, weights, w_bytes);
    memset(bos.y.mapped, 0, y_bytes);

    if (xdna2_sync_bo(fd, &bos.x, SYNC_DIRECT_TO_DEVICE, 0, x_bytes) < 0 ||
        xdna2_sync_bo(fd, &bos.w, SYNC_DIRECT_TO_DEVICE, 0, w_bytes) < 0 ||
        xdna2_sync_bo(fd, &bos.y, SYNC_DIRECT_TO_DEVICE, 0, y_bytes) < 0) {
        ret = -EIO;
        goto out;
    }
    if (timing) { uint64_t t = now_ns(); timing->upload_ns = t - t_mark; t_mark = t; }

    const uint32_t payload_words =
        (uint32_t)(sizeof(xdna2_ert_start_npu_payload_t) / sizeof(uint32_t));
    const size_t cmd_bytes = sizeof(uint32_t) * (2u + payload_words);
    if ((ret = alloc_and_map(fd, &bos.cmd, cmd_bytes, AMDXDNA_BO_CMD)) < 0) goto out;

    ert_packet_t *pkt = (ert_packet_t *)bos.cmd.mapped;
    memset(pkt, 0, cmd_bytes);
    pkt->header = ert_make_header(kernel->ert_opcode, payload_words + 1u /* cu_mask */);
    pkt->cu_mask = kernel->cu_mask;

    xdna2_ert_start_npu_payload_t *payload = (xdna2_ert_start_npu_payload_t *)pkt->data;
    payload->npu.instruction_buffer = kernel->instr_bo.xdna_addr;
    payload->npu.instruction_buffer_size = (uint32_t)kernel->instr_bo.size;
    payload->npu.instruction_prop_count = 0;
    payload->args.x = bos.x.xdna_addr;
    payload->args.w = bos.w.xdna_addr;
    payload->args.y = bos.y.xdna_addr;
    payload->args.I = (uint32_t)I;
    payload->args.O = (uint32_t)O;

    uint32_t arg_handles[3] = { bos.x.handle, bos.w.handle, bos.y.handle };
    if (xdna2_submit_command(fd, &runtime->hwctx, bos.cmd.handle, arg_handles, 3) < 0) {
        ret = -EIO;
        goto out;
    }
    if (timing) { uint64_t t = now_ns(); timing->submit_ns = t - t_mark; t_mark = t; }

    if (xdna2_wait_command(fd, &runtime->hwctx, runtime->timeout_ms) < 0) {
        ret = -ETIMEDOUT;
        goto out;
    }
    if (timing) { uint64_t t = now_ns(); timing->wait_ns = t - t_mark; t_mark = t; }

    if (xdna2_sync_bo(fd, &bos.y, SYNC_DIRECT_FROM_DEVICE, 0, y_bytes) < 0) {
        ret = -EIO;
        goto out;
    }
    /* The AIE int8 datapath accumulates into int32 — every artifact is compiled
     * with dtype_in=i8, dtype_out=i32; see
     * vnni-int8-matmul/npu/aie/build_shape.py — so the output BO holds
     * accumulators, not floats. Reinterpreting those bytes as f32 would be
     * silently wrong in exactly the way the NPU guardrails exist to prevent. The
     * per-column scale that turns them back into activations is applied here, on
     * the host, because the kernels carry no dequantisation epilogue. */
    xdna2_dequant_i32((const int32_t *)bos.y.mapped, y, S, O, scales);
    if (timing) { uint64_t t = now_ns(); timing->readback_ns = t - t_mark; t_mark = t; }
    ret = 0;

out:
    free_bos(fd, &bos);
    if (timing) {
        uint64_t t = now_ns();
        timing->teardown_ns = t - t_mark;
        timing->total_ns = t - t_start;
    }
    return ret;
}

void xdna2_dequant_i32(const int32_t *acc, float *out, int S, int O,
                       const float *scales) {
    if (!acc || !out || S <= 0 || O <= 0) return;

    for (int s = 0; s < S; ++s) {
        const int32_t *row = acc + (size_t)s * (size_t)O;
        float *dst = out + (size_t)s * (size_t)O;
        for (int o = 0; o < O; ++o) {
            const float scale = scales ? scales[o] : 1.0f;
            dst[o] = (float)row[o] * scale;
        }
    }
}

void xdna2_dequant_int4(const uint8_t *packed, int8_t *out, int O, int I) {
    if (!packed || !out || O <= 0 || I <= 0) return;

    const size_t stride = (size_t)((I + 1) / 2);
    for (int o = 0; o < O; ++o) {
        const uint8_t *row = packed + (size_t)o * stride;
        for (int i = 0; i < I; ++i) {
            unsigned byte = row[(size_t)i >> 1];
            unsigned nibble = (i & 1) ? (byte >> 4) : (byte & 0x0Fu);
            out[(size_t)o * (size_t)I + (size_t)i] = (int8_t)((int)nibble - 8);
        }
    }
}
