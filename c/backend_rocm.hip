/*
 * colibrì ROCm/HIP backend — direct HIP port of backend_cuda.cu.
 *
 * Compile:  hipcc -O3 -std=c++17 -c backend_rocm.hip -o backend_rocm.o
 *           (Makefile does this automatically with ROCM=1.)
 *
 * Unified-memory path (AMD APU / Strix Halo):
 *   On integrated GPUs (hipDeviceAttributeIntegrated == 1) the CPU and GPU
 *   share physical DRAM.  Weight tensors are allocated with
 *   hipHostMalloc(hipHostMallocMapped) so the GPU accesses them via a mapped
 *   device pointer without any DMA copy.  Input/output scratch buffers also
 *   use mapped host memory to avoid the VRAM carve-out size limit.
 *
 *   Override via environment variable COLI_ROCM_UNIFIED:
 *     1  — force unified path even on discrete GPUs (rarely useful)
 *     0  — force discrete path even on integrated GPUs (for testing)
 *   Absent: auto-detected.
 */

#include "backend_rocm.h"

#include <hip/hip_runtime.h>

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

/* ---- struct ------------------------------------------------------------- */

struct ColiCudaTensor {
    void  *weights;       /* GPU-accessible device (or mapped host) pointer  */
    float *scales;        /* GPU-accessible scale pointer                     */
    void  *weights_host;  /* non-NULL only on unified memory (hipHostMalloc)  */
    float *scales_host;   /* non-NULL only on unified memory                  */
    size_t weight_bytes;
    int fmt, I, O, device;
    int tracked;
    int unified;          /* 1 = APU unified memory path                      */
};

typedef struct {
    int    device;
    float *x,  *y;        /* GPU-accessible scratch (device or mapped host)   */
    float *xh, *yh;       /* host-side pointers when unified                  */
    size_t x_cap, y_cap;
    size_t tensor_count, tensor_bytes;
    int unified;
} DeviceContext;

static DeviceContext g_ctx[COLI_CUDA_MAX_DEVICES];
static int g_nctx;

/* ---- helpers ------------------------------------------------------------ */

static int hip_ok(hipError_t err, const char *what) {
    if (err == hipSuccess) return 1;
    std::fprintf(stderr, "[ROCm] %s: %s\n", what, hipGetErrorString(err));
    return 0;
}

static DeviceContext *find_ctx(int device) {
    for (int i = 0; i < g_nctx; i++)
        if (g_ctx[i].device == device) return &g_ctx[i];
    return nullptr;
}

static int select_ctx(DeviceContext *ctx) {
    return ctx && hip_ok(hipSetDevice(ctx->device), "select device");
}

static size_t row_bytes(int fmt, int I) {
    if (fmt == 0) return (size_t)I * sizeof(float);
    if (fmt == 1) return (size_t)I;
    if (fmt == 2) return (size_t)(I + 1) / 2;
    if (fmt == 3) return (size_t)(I + 3) / 4;
    if (fmt == 4) return (size_t)I;
    if (fmt == 5) return (size_t)((I * 6 + 7) / 8);
    if (fmt == 6) return (size_t)(I + 1) / 2;
    return 0;
}

/*
 * Allocate `bytes` of GPU-accessible memory.
 * On unified devices, uses hipHostMalloc(hipHostMallocMapped) so the host
 * pointer is also valid for CPU access.  *host_ptr receives the host pointer
 * (identical to *dev_ptr on APU); on discrete devices *host_ptr is set NULL.
 */
static int alloc_gpu(void **dev_ptr, void **host_ptr, size_t bytes, int unified) {
    if (unified) {
        void *h = nullptr;
        if (!hip_ok(hipHostMalloc(&h, bytes, hipHostMallocMapped), "host alloc mapped"))
            return 0;
        void *d = nullptr;
        if (!hip_ok(hipHostGetDevicePointer(&d, h, 0), "host get dev ptr")) {
            hipHostFree(h);
            return 0;
        }
        *host_ptr = h;
        *dev_ptr  = d;
    } else {
        if (!hip_ok(hipMalloc(dev_ptr, bytes), "device alloc")) return 0;
        *host_ptr = nullptr;
    }
    return 1;
}

static void free_gpu(void *dev_ptr, void *host_ptr, int unified) {
    if (!dev_ptr && !host_ptr) return;
    if (unified) {
        if (host_ptr) hipHostFree(host_ptr);
    } else {
        if (dev_ptr) hipFree(dev_ptr);
    }
}

/*
 * Ensure scratch float* has at least `bytes`.  On unified devices, scratch
 * lives in mapped host memory so we avoid the VRAM carve-out limit.
 */
static int reserve_scratch(float **dev, float **host, size_t *cap,
                            size_t bytes, int unified) {
    if (*cap >= bytes) return 1;
    free_gpu(*dev, *host, unified);
    *dev = nullptr; *host = nullptr; *cap = 0;
    void *d = nullptr, *h = nullptr;
    if (!alloc_gpu(&d, &h, bytes, unified)) return 0;
    *dev  = static_cast<float *>(d);
    *host = static_cast<float *>(h);
    *cap  = bytes;
    return 1;
}

/* ---- GPU kernel --------------------------------------------------------- */

__device__ static float decode_fp8_e4m3fnuz(uint8_t raw) {
    constexpr float subnormal_scale = 0.015625f;  /* 2^-6 */
    constexpr int exponent_bias = 7;
    const int sign_multiplier = (raw & 0x80) ? -1 : 1;
    const int exp = (raw >> 3) & 0x0f;
    const int mant = raw & 0x07;
    if (exp == 0) {
        if (mant == 0) return sign_multiplier < 0 ? -0.0f : 0.0f;
        return sign_multiplier * (mant / 8.0f) * subnormal_scale;
    }
    if (exp == 0x0f) return sign_multiplier * INFINITY;
    return sign_multiplier * (1.0f + mant / 8.0f) * exp2f(exp - exponent_bias);
}

__device__ static float decode_packed4(const uint8_t *q, int i) {
    uint8_t v = q[i >> 1];
    return static_cast<float>(((i & 1) ? (v >> 4) : (v & 15)) - 8);
}

__device__ static float weight_at(const void *weights, int fmt,
                                   size_t row, int i, int I, size_t row_bytes) {
    const uint8_t *base = static_cast<const uint8_t *>(weights) + row;
    if (fmt == 0) return reinterpret_cast<const float *>(base)[i];
    if (fmt == 1) return static_cast<float>(reinterpret_cast<const int8_t *>(base)[i]);
    const uint8_t *q = base;
    if (fmt == 2) return decode_packed4(q, i);
    if (fmt == 3) {
        uint8_t v = q[i >> 2];
        return static_cast<float>(((v >> ((i & 3) * 2)) & 3) - 2);
    }
    if (fmt == 4) return decode_fp8_e4m3fnuz(q[i]);
    if (fmt == 5) {
        size_t bit = (size_t)i * 6u;
        size_t byte = bit / 8u;
        size_t shift = bit % 8u;
        uint16_t word = static_cast<uint16_t>(q[byte]);
        /* Only read the next byte when it still exists in the packed row. */
        if (byte < row_bytes - 1) word |= (static_cast<uint16_t>(q[byte + 1]) << 8);
        int value = static_cast<int>((word >> shift) & 0x3f);
        if (value & 0x20) value -= 0x40;
        return static_cast<float>(value);
    }
    if (fmt == 6) return decode_packed4(q, i);
    return 0.0f;
}

__global__ static void quant_matmul(float *y, const float *x,
                                    const void *weights, const float *scales,
                                    int fmt, int S, int I, int O, size_t rb) {
    int o = blockIdx.x;
    int s = blockIdx.y;
    float sum = 0.0f;
    size_t row = (size_t)o * rb;
    const float *xs = x + (size_t)s * I;
    for (int i = threadIdx.x; i < I; i += blockDim.x)
        sum += xs[i] * weight_at(weights, fmt, row, i, I, rb);

    __shared__ float partial[256];
    partial[threadIdx.x] = sum;
    __syncthreads();
    for (int n = blockDim.x >> 1; n; n >>= 1) {
        if (threadIdx.x < n) partial[threadIdx.x] += partial[threadIdx.x + n];
        __syncthreads();
    }
    if (!threadIdx.x)
        y[(size_t)s * O + o] = partial[0] * (fmt ? scales[o] : 1.0f);
}

/* ---- public API --------------------------------------------------------- */

extern "C" int coli_cuda_init(const int *devices, int count) {
    int available = 0;
    if (!devices || count < 1 || count > COLI_CUDA_MAX_DEVICES) return 0;
    if (!hip_ok(hipGetDeviceCount(&available), "device discovery")) return 0;

    /* honour COLI_ROCM_UNIFIED override */
    const char *env_uni = std::getenv("COLI_ROCM_UNIFIED");
    int force_unified = env_uni ? std::atoi(env_uni) : -1; /* -1 = auto */

    g_nctx = 0;
    for (int i = 0; i < count; i++) {
        int device = devices[i];
        if (device < 0 || device >= available) {
            std::fprintf(stderr, "[ROCm] invalid device %d (available: 0..%d)\n",
                         device, available - 1);
            g_nctx = 0;
            return 0;
        }
        if (find_ctx(device)) {
            std::fprintf(stderr, "[ROCm] duplicate device %d\n", device);
            g_nctx = 0;
            return 0;
        }

        DeviceContext *ctx = &g_ctx[g_nctx];
        std::memset(ctx, 0, sizeof(*ctx));
        ctx->device = device;
        if (!select_ctx(ctx)) { g_nctx = 0; return 0; }

        hipDeviceProp_t prop{};
        if (!hip_ok(hipGetDeviceProperties(&prop, device), "device properties")) {
            g_nctx = 0; return 0;
        }

        /* detect integrated / unified-memory GPU */
        int integrated = 0;
        hipDeviceGetAttribute(&integrated, hipDeviceAttributeIntegrated, device);
        if (force_unified >= 0)
            ctx->unified = force_unified;
        else
            ctx->unified = integrated;

        g_nctx++;
        std::fprintf(stderr,
                     "[ROCm] device %d: %s, %.1f GB%s\n",
                     device, prop.name, prop.totalGlobalMem / 1e9,
                     ctx->unified ? " (unified/APU memory)" : "");
    }
    return 1;
}

extern "C" void coli_cuda_shutdown(void) {
    for (int i = 0; i < g_nctx; i++) {
        DeviceContext *ctx = &g_ctx[i];
        if (!select_ctx(ctx)) continue;
        free_gpu(ctx->x, ctx->xh, ctx->unified);
        free_gpu(ctx->y, ctx->yh, ctx->unified);
        ctx->x = ctx->y = nullptr;
        ctx->xh = ctx->yh = nullptr;
        ctx->x_cap = ctx->y_cap = 0;
    }
    g_nctx = 0;
}

extern "C" int coli_cuda_device_count(void) { return g_nctx; }

extern "C" int coli_cuda_device_at(int index) {
    return index >= 0 && index < g_nctx ? g_ctx[index].device : -1;
}

extern "C" int coli_cuda_mem_info(int device,
                                   size_t *free_bytes, size_t *total_bytes) {
    DeviceContext *ctx = find_ctx(device);
    if (!free_bytes || !total_bytes || !select_ctx(ctx)) return 0;
    return hip_ok(hipMemGetInfo(free_bytes, total_bytes), "memory info");
}

extern "C" void coli_cuda_stats(int device,
                                 size_t *tensor_count, size_t *tensor_bytes) {
    size_t count = 0, bytes = 0;
    for (int i = 0; i < g_nctx; i++)
        if (device < 0 || g_ctx[i].device == device) {
            count += g_ctx[i].tensor_count;
            bytes += g_ctx[i].tensor_bytes;
        }
    if (tensor_count) *tensor_count = count;
    if (tensor_bytes)  *tensor_bytes  = bytes;
}

extern "C" int coli_cuda_tensor_upload(ColiCudaTensor **tensor,
                                        const void *weights, const float *scales,
                                        int fmt, int I, int O, int device) {
    DeviceContext *ctx = find_ctx(device);
    if (!tensor || !weights || I < 1 || O < 1 || !select_ctx(ctx)) return 0;
    size_t rb = row_bytes(fmt, I);
    if (!rb || (fmt && !scales)) return 0;
    if (*tensor) {
        ColiCudaTensor *t = *tensor;
        return t->fmt == fmt && t->I == I && t->O == O && t->device == device;
    }

    ColiCudaTensor *t =
        static_cast<ColiCudaTensor *>(std::calloc(1, sizeof(*t)));
    if (!t) return 0;
    t->fmt = fmt; t->I = I; t->O = O; t->device = device;
    t->weight_bytes = rb * (size_t)O;
    t->unified = ctx->unified;

    if (!alloc_gpu(&t->weights, &t->weights_host, t->weight_bytes, t->unified)) {
        coli_cuda_tensor_free(t);
        return 0;
    }
    if (t->unified) {
        std::memcpy(t->weights_host, weights, t->weight_bytes);
    } else {
        if (!hip_ok(hipMemcpy(t->weights, weights, t->weight_bytes,
                              hipMemcpyHostToDevice), "tensor upload")) {
            coli_cuda_tensor_free(t);
            return 0;
        }
    }

    if (fmt) {
        size_t sb = (size_t)O * sizeof(float);
        void *sd = nullptr, *sh = nullptr;
        if (!alloc_gpu(&sd, &sh, sb, t->unified)) {
            coli_cuda_tensor_free(t);
            return 0;
        }
        t->scales = static_cast<float *>(sd);
        t->scales_host = static_cast<float *>(sh);
        if (t->unified) {
            std::memcpy(t->scales_host, scales, sb);
        } else {
            if (!hip_ok(hipMemcpy(t->scales, scales, sb,
                                  hipMemcpyHostToDevice), "scale upload")) {
                coli_cuda_tensor_free(t);
                return 0;
            }
        }
    }

    t->tracked = 1;
    ctx->tensor_count++;
    ctx->tensor_bytes += t->weight_bytes +
                         (fmt ? (size_t)O * sizeof(float) : 0);
    *tensor = t;
    return 1;
}

extern "C" int coli_cuda_matmul(ColiCudaTensor **tensor,
                                 float *y, const float *x,
                                 const void *weights, const float *scales,
                                 int fmt, int S, int I, int O, int device) {
    if (S < 1 ||
        !coli_cuda_tensor_upload(tensor, weights, scales, fmt, I, O, device))
        return 0;
    ColiCudaTensor *t = *tensor;
    DeviceContext *ctx = find_ctx(t->device);
    if (!select_ctx(ctx)) return 0;

    size_t xb = (size_t)S * I * sizeof(float);
    size_t yb = (size_t)S * O * sizeof(float);
    if (!reserve_scratch(&ctx->x, &ctx->xh, &ctx->x_cap, xb, ctx->unified) ||
        !reserve_scratch(&ctx->y, &ctx->yh, &ctx->y_cap, yb, ctx->unified))
        return 0;

    if (ctx->unified) {
        /* Zero-copy: copy input into the mapped host buffer; GPU reads it
         * directly without any DMA transfer.  Output is written in-place. */
        std::memcpy(ctx->xh, x, xb);
    } else {
        if (!hip_ok(hipMemcpy(ctx->x, x, xb, hipMemcpyHostToDevice),
                    "input upload"))
            return 0;
    }

    dim3 grid((unsigned)O, (unsigned)S);
    hipLaunchKernelGGL(quant_matmul, grid, 256, 0, 0,
                       ctx->y, ctx->x, t->weights, t->scales,
                       fmt, S, I, O, row_bytes(fmt, I));

    if (!hip_ok(hipGetLastError(), "matmul launch")) return 0;

    if (ctx->unified) {
        /* GPU and CPU share the output buffer; sync and read directly. */
        if (!hip_ok(hipDeviceSynchronize(), "sync")) return 0;
        std::memcpy(y, ctx->yh, yb);
    } else {
        if (!hip_ok(hipMemcpy(y, ctx->y, yb, hipMemcpyDeviceToHost),
                    "output download"))
            return 0;
    }
    return 1;
}

extern "C" void coli_cuda_tensor_free(ColiCudaTensor *tensor) {
    if (!tensor) return;
    DeviceContext *ctx = find_ctx(tensor->device);
    if (ctx) select_ctx(ctx);
    if (tensor->tracked && ctx) {
        size_t bytes = tensor->weight_bytes +
                       (tensor->fmt ? (size_t)tensor->O * sizeof(float) : 0);
        if (ctx->tensor_count) ctx->tensor_count--;
        if (ctx->tensor_bytes >= bytes) ctx->tensor_bytes -= bytes;
    }
    free_gpu(tensor->weights, tensor->weights_host, tensor->unified);
    free_gpu(tensor->scales,  tensor->scales_host,  tensor->unified);
    std::free(tensor);
}

extern "C" size_t coli_cuda_tensor_bytes(const ColiCudaTensor *tensor) {
    return tensor ? tensor->weight_bytes +
                    (tensor->fmt ? (size_t)tensor->O * sizeof(float) : 0) : 0;
}

extern "C" int coli_cuda_tensor_device(const ColiCudaTensor *tensor) {
    return tensor ? tensor->device : -1;
}
