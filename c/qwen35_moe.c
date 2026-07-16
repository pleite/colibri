/* Qwen3.5 MoE engine skeleton for colibri.
 *
 * This is intentionally a lightweight CPU-only forward pass that supports
 * tiny synthetic Qwen3.5-style models stored as safetensors shards. It is
 * designed to be a correctness scaffold for later full-precision/quantized
 * work, while also giving the repository a real engine binary to build and
 * wire into the CLI/doctor/resource-planning flows.
 */
#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#if defined(__GLIBC__)
#include <malloc.h>
#endif
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "parallelism.h"

#ifdef _OPENMP
#include <omp.h>
#endif
#if defined(COLI_VULKAN)
#include "backend_vulkan.h"
#elif defined(COLI_ROCM)
#include "backend_rocm.h"
#elif defined(COLI_NPU)
#include "backend_npu.h"
#elif defined(COLI_CUDA)
#include "backend_cuda.h"
#endif

#if defined(COLI_ROCM)
int coli_rocm_init(const int *devices, int count);
void coli_rocm_shutdown(void);
int coli_rocm_device_count(void);
int coli_rocm_device_at(int index);
int coli_rocm_mem_info(int device, size_t *free_bytes, size_t *total_bytes);
void coli_rocm_stats(int device, size_t *tensor_count, size_t *tensor_bytes);
int coli_rocm_tensor_upload(ColiCudaTensor **tensor, const void *weights, const float *scales, int fmt, int I, int O, int device);
int coli_rocm_matmul(ColiCudaTensor **tensor, float *y, const float *x, const void *weights, const float *scales, int fmt, int S, int I, int O, int device);
void coli_rocm_tensor_free(ColiCudaTensor *tensor);
size_t coli_rocm_tensor_bytes(const ColiCudaTensor *tensor);
int coli_rocm_tensor_device(const ColiCudaTensor *tensor);
#endif

#if defined(COLI_ENABLE_NPU)
int coli_npu_compat_init(const int *devices, int count);
void coli_npu_compat_shutdown(void);
int coli_npu_compat_device_count(void);
int coli_npu_compat_device_at(int index);
int coli_npu_compat_mem_info(int device, size_t *free_bytes, size_t *total_bytes);
void coli_npu_compat_stats(int device, size_t *tensor_count, size_t *tensor_bytes);
int coli_npu_compat_tensor_upload(ColiCudaTensor **tensor, const void *weights, const float *scales, int fmt, int I, int O, int device);
int coli_npu_compat_matmul(ColiCudaTensor **tensor, float *y, const float *x, const void *weights, const float *scales, int fmt, int S, int I, int O, int device);
void coli_npu_compat_tensor_free(ColiCudaTensor *tensor);
size_t coli_npu_compat_tensor_bytes(const ColiCudaTensor *tensor);
int coli_npu_compat_tensor_device(const ColiCudaTensor *tensor);
#endif

#include "json.h"
#include "st.h"
#include "backend_runtime.h"

#if !defined(COLI_VULKAN) && !defined(COLI_ROCM) && !defined(COLI_NPU) && !defined(COLI_CUDA)
typedef struct ColiCudaTensor ColiCudaTensor;
#endif

#if !defined(COLI_ROCM) && !defined(COLI_ENABLE_NPU)
void coli_cuda_tensor_free(ColiCudaTensor *tensor);
#endif

/* Keep the backend-selection condition centralized and use the same ordering as
 * the existing include/dispatch logic: Vulkan, ROCm, NPU, then CUDA. */
#if defined(COLI_VULKAN) || defined(COLI_ROCM) || defined(COLI_NPU) || defined(COLI_CUDA)
#define COLI_HAS_BACKEND 1
#else
#define COLI_HAS_BACKEND 0
#endif

/* Keep accelerator dispatch opt-in for larger matmuls while preserving a CPU fallback. */
#define COLI_MATMUL_BACKEND_THRESHOLD 64

#define MAX_KV_SLOTS 16
#define LA_STATE_DECAY 0.6f
#define LA_STATE_UPDATE 0.4f
#define LA_CONV1D_WIDTH 4

typedef enum {
    LAYER_TYPE_LINEAR = 0,
    LAYER_TYPE_FULL = 1,
} layer_type_t;

typedef enum {
    QWEN_EXPERT_STATE_UNLOADED = 0,
    QWEN_EXPERT_STATE_RESIDENT = 1,
    QWEN_EXPERT_STATE_PINNED = 2,
} qwen_expert_state_t;

/* Quantized tensor container (dequant-on-use). Mirrors GLM's QT layout so
 * Qwen3.5 stores weights in the same INT8/INT4/F32 formats instead of expanding
 * everything to F32 in memory. fmt: 0=F32, 1=INT8, 2=INT4. For fmt!=0 `data`
 * holds the packed payload ([O,I] int8 or [O,ceil(I/2)] int4) and `scales` the
 * per-output-row F32 scale. The matmul dequantizes on the fly, so a weight that
 * used to occupy 4 bytes/param now costs 1 byte (int8) or 0.5 byte (int4). */
typedef struct {
    int fmt;
    int O;
    int I;
    void *data;
    float *scales;
    ColiCudaTensor *handle; /* backend-runtime cache handle (resident tensors) */
} QTensor;

typedef struct {
    float *in_ln;
    float *post_ln;
    bool is_full_attn;
    float *q_norm;
    float *k_norm;
    QTensor q_proj;
    QTensor k_proj;
    QTensor v_proj;
    QTensor o_proj;
    QTensor router;
    QTensor sh_gate;
    QTensor sh_up;
    QTensor sh_down;
    float *shared_expert_gate;
    QTensor mlp_gate_proj;
    QTensor mlp_up_proj;
    QTensor mlp_down_proj;
    QTensor *expert_gate_proj;
    QTensor *expert_up_proj;
    QTensor *expert_down_proj;
    int *expert_state;
    float *la_in_proj_a;
    float *la_in_proj_b;
    float *la_in_proj_qkv;
    float *la_in_proj_z;
    float *la_out_proj;
    float *la_norm;
    float *la_A_log;
    float *la_dt_bias;
    float *la_conv1d;
    float *la_state;
    float *kv_cache_k;
    float *kv_cache_v;
    int kv_cache_len;
    int kv_cache_cap;
} QLayer;

typedef struct {
    int vocab_size;
    int hidden_size;
    int num_layers;
    int num_experts;
    int experts_per_tok;
    int moe_intermediate_size;
    int shared_expert_intermediate_size;
    int num_attention_heads;
    int num_kv_heads;
    int head_dim;
    int linear_num_key_heads;
    int linear_key_head_dim;
    int linear_num_value_heads;
    int linear_value_head_dim;
    bool has_router;
    char *snap_dir;
    shards shards;
    float *embed;
    float *final_norm;
    float *lm_head;
    QLayer *layers;
    int *layer_types;
    int kv_slots;
    float ***kv_cache_k_slots;
    float ***kv_cache_v_slots;
    int **kv_cache_lens;
    int **kv_cache_caps;
    float rope_theta;
    float partial_rotary_factor;
    bool use_rope;
} qwen35_model;

static void fail(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

static void *qwen_malloc_impl(size_t size, const char *what);
static void *qwen_calloc_impl(size_t nmemb, size_t size, const char *what);
static void *qwen_realloc_impl(void *ptr, size_t size, const char *what);
#define qwen_malloc(size, ...) qwen_malloc_impl((size), "allocation")
#define qwen_calloc(nmemb, size, ...) qwen_calloc_impl((nmemb), (size), "allocation")
#define qwen_realloc(ptr, size, ...) qwen_realloc_impl((ptr), (size), "allocation")
static char *qwen_strdup(const char *s);
static void model_debug(const char *fmt, ...);

static char *read_text_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) fail("cannot open %s", path);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char *buf = (char *)qwen_malloc((size_t)size + 1, "read_text_file");
    size_t n = fread(buf, 1, (size_t)size, fp);
    fclose(fp);
    buf[n] = '\0';
    return buf;
}

static bool parse_bool_env(const char *name) {
    const char *env = getenv(name);
    if (!env || !*env) return false;
    if (strcmp(env, "0") == 0 || strcmp(env, "false") == 0 || strcmp(env, "FALSE") == 0 ||
        strcmp(env, "no") == 0 || strcmp(env, "NO") == 0 || strcmp(env, "off") == 0 ||
        strcmp(env, "OFF") == 0) {
        return false;
    }
    if (strcmp(env, "1") == 0 || strcmp(env, "true") == 0 || strcmp(env, "TRUE") == 0 ||
        strcmp(env, "yes") == 0 || strcmp(env, "YES") == 0 || strcmp(env, "on") == 0 ||
        strcmp(env, "ON") == 0) {
        return true;
    }
    return false;
}

static bool g_model_debug_enabled = false;
static bool g_model_debug_initialized = false;
static size_t g_ram_limit_bytes = 0;
static size_t g_ram_peak_bytes = 0;

static void set_model_debug_enabled(bool enabled) {
    g_model_debug_enabled = enabled;
    g_model_debug_initialized = true;
}

static bool model_debug_enabled(void) {
    if (!g_model_debug_initialized) {
        set_model_debug_enabled(parse_bool_env("COLI_QWEN_DEBUG"));
    }
    return g_model_debug_enabled;
}

static bool parse_ram_limit_mb(const char *value, size_t *out_bytes) {
    if (!value || !*value) return false;
    char *end = NULL;
    errno = 0;
    unsigned long long parsed = strtoull(value, &end, 10);
    if (!end || *end != '\0' || errno == ERANGE) return false;
    if (parsed > (unsigned long long)(SIZE_MAX / (1024ULL * 1024ULL))) return false;
    *out_bytes = (size_t)(parsed * 1024ULL * 1024ULL);
    return true;
}

static void configure_ram_limit(const char *value) {
    if (!value || !*value) {
        const char *env_value = getenv("COLI_QWEN_RAM_LIMIT_MB");
        value = env_value;
    }
    if (!value || !*value) return;
    size_t limit_bytes = 0;
    if (!parse_ram_limit_mb(value, &limit_bytes)) fail("invalid RAM limit: %s", value);
    g_ram_limit_bytes = limit_bytes;
    model_debug("RAM limit enabled: %zu bytes", g_ram_limit_bytes);
}

static bool reserve_ram(size_t bytes, const char *what) {
    if (g_ram_limit_bytes == 0) return true;
    if (g_ram_peak_bytes + bytes > g_ram_limit_bytes) {
        fprintf(stderr, "[qwen35_moe] RAM limit exceeded while allocating %s (%zu bytes requested, %zu bytes used, %zu bytes limit)\n",
                what, bytes, g_ram_peak_bytes, g_ram_limit_bytes);
        return false;
    }
    g_ram_peak_bytes += bytes;
    return true;
}

static void *qwen_malloc_impl(size_t size, const char *what) {
    if (!reserve_ram(size, what)) fail("memory limit exceeded while allocating %s", what);
    void *ptr = malloc(size);
    if (!ptr) fail("out of memory");
    return ptr;
}

static void *qwen_calloc_impl(size_t nmemb, size_t size, const char *what) {
    if (nmemb != 0 && size > SIZE_MAX / nmemb) fail("allocation size overflow");
    size_t total = nmemb * size;
    if (!reserve_ram(total, what)) fail("memory limit exceeded while allocating %s", what);
    void *ptr = calloc(nmemb, size);
    if (!ptr) fail("out of memory");
    return ptr;
}

static size_t qwen_malloc_usable_size(void *ptr) {
#if defined(__GLIBC__)
    return malloc_usable_size(ptr);
#else
    (void)ptr;
    return 0;
#endif
}

static void *qwen_realloc_impl(void *ptr, size_t size, const char *what) {
    size_t old_size = ptr ? qwen_malloc_usable_size(ptr) : 0;
    size_t delta = size > old_size ? size - old_size : 0;
    if (delta > 0 && !reserve_ram(delta, what)) fail("memory limit exceeded while reallocating %s", what);
    void *new_ptr = realloc(ptr, size);
    if (!new_ptr && size != 0) fail("out of memory");
    return new_ptr;
}

static char *qwen_strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *copy = (char *)qwen_malloc(len, "string copy");
    memcpy(copy, s, len);
    return copy;
}

/* Scale tensors are float32 and are expected to match exactly for duplicated rows;
 * this small epsilon only absorbs float parsing noise. */
static const float SCALE_LAYOUT_EPSILON = 1e-6f;

static void model_debug(const char *fmt, ...) {
    if (!model_debug_enabled()) return;
    va_list ap;
    va_start(ap, fmt);
    fprintf(stderr, "[qwen35_moe] ");
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
}

static int parse_int_field(jval *obj, const char *key, int fallback) {
    jval *value = json_get(obj, key);
    if (!value || value->t != J_NUM) return fallback;
    return (int)value->num;
}

static int parse_int_field_with_fallback(jval *primary, jval *secondary, const char *key, int fallback) {
    jval *value = json_get(primary, key);
    if (!value || value->t != J_NUM) value = json_get(secondary, key);
    if (!value || value->t != J_NUM) return fallback;
    return (int)value->num;
}

static float parse_float_field(jval *obj, const char *key, float fallback) {
    jval *value = json_get(obj, key);
    if (!value || value->t != J_NUM) return fallback;
    return (float)value->num;
}

static bool parse_bool_field(jval *obj, const char *key) {
    jval *value = json_get(obj, key);
    return value && value->t == J_BOOL && value->boolean;
}

static char *join_path(const char *dir, const char *name) {
    size_t n = strlen(dir) + 1 + strlen(name) + 1;
    char *out = (char *)qwen_malloc(n, "join_path");
    snprintf(out, n, "%s/%s", dir, name);
    return out;
}

static st_tensor *find_tensor(qwen35_model *m, const char *name) {
    st_tensor *t = st_find(&m->shards, name);
    if (t) return t;
    static const char *const aliases[][2] = {
        {"model.layers.", "model.language_model.layers."},
        {"model.language_model.layers.", "model.layers."},
        {"model.embed_tokens.", "model.language_model.embed_tokens."},
        {"model.language_model.embed_tokens.", "model.embed_tokens."},
        {"model.norm.", "model.language_model.norm."},
        {"model.language_model.norm.", "model.norm."},
        {"lm_head.", "model.language_model.lm_head."},
        {"model.language_model.lm_head.", "lm_head."},
    };
    for (size_t i = 0; i < sizeof(aliases) / sizeof(aliases[0]); i++) {
        const char *from = aliases[i][0];
        const char *to = aliases[i][1];
        size_t from_len = strlen(from);
        if (strncmp(name, from, from_len) == 0) {
            size_t tail_len = strlen(name + from_len);
            size_t total = strlen(to) + tail_len + 1;
            char *candidate = (char *)qwen_malloc(total, "tensor alias");
            snprintf(candidate, total, "%s%s", to, name + from_len);
            t = st_find(&m->shards, candidate);
            free(candidate);
            if (t) return t;
        }
    }
    return NULL;
}

static st_tensor *find_scale_tensor(qwen35_model *m, const char *name) {
    static const char *const suffixes[] = {".weight_scale", ".scale", ".scales", ".qs"};
    for (size_t i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); i++) {
        size_t total = strlen(name) + strlen(suffixes[i]) + 1;
        char *candidate = (char *)qwen_malloc(total, "scale tensor alias");
        snprintf(candidate, total, "%s%s", name, suffixes[i]);
        st_tensor *t = find_tensor(m, candidate);
        free(candidate);
        if (t) return t;
    }
    return NULL;
}

static int tensor_exists(qwen35_model *m, const char *name) {
    return find_tensor(m, name) != NULL;
}

/* ---- Quantized tensor helpers (dequant-on-use) --------------------------- */

static inline float *qt_f32(QTensor *q) { return (float *)q->data; }

/* Allocate an F32 (fmt 0) QTensor of shape [O, I]. Used for missing/synthetic
 * tensors and for weights that are not stored in a standard quantized layout. */
static QTensor qt_alloc_f32(int O, int I, const char *what) {
    QTensor q;
    q.fmt = 0;
    q.O = O;
    q.I = I;
    q.scales = NULL;
    q.handle = NULL;
    q.data = qwen_calloc((size_t)O * (size_t)I, sizeof(float), what);
    return q;
}

static void qt_free(QTensor *q) {
    if (!q) return;
    if (q->handle) {
        coli_runtime_tensor_free(q->handle);
        q->handle = NULL;
    }
    free(q->data);
    q->data = NULL;
    free(q->scales);
    q->scales = NULL;
    q->fmt = q->O = q->I = 0;
}

/* CPU dequant-on-use matmul: y[o] = scale[o] * sum_i x[i] * dequant(w[o,i]).
 * The integer accumulation with a single trailing scale multiply matches the
 * backend runtime's host_matmul exactly, and for fmt 0 it is bit-identical to
 * matmul_vec (so F32 models remain token-exact). No F32 expansion or weight copy
 * is performed, which is where the memory savings come from. */
static void qwen_cpu_qmatmul(float *out, const float *x, const void *weights,
                             const float *scales, int fmt, int I, int O) {
    const float *wf = (const float *)weights;
    const int8_t *wi8 = (const int8_t *)weights;
    const uint8_t *wp = (const uint8_t *)weights;
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (int o = 0; o < O; o++) {
        const float row_scale = (scales && fmt != 0) ? scales[o] : 1.0f;
        float acc = 0.0f;
        if (fmt == 0) {
            const float *row = wf + (size_t)o * (size_t)I;
            for (int i = 0; i < I; i++) acc += x[i] * row[i];
        } else if (fmt == 1) {
            const int8_t *row = wi8 + (size_t)o * (size_t)I;
            for (int i = 0; i < I; i++) acc += x[i] * (float)row[i];
        } else { /* fmt == 2: INT4 nibble-packed */
            const size_t stride = (size_t)((I + 1) / 2);
            const uint8_t *row = wp + (size_t)o * stride;
            for (int i = 0; i < I; i++) {
                const int nib = (i & 1) ? (row[i >> 1] >> 4) & 0x0f : row[i >> 1] & 0x0f;
                acc += x[i] * (float)(nib - 8);
            }
        }
        out[o] = acc * row_scale;
    }
}

/* Returns non-zero when a real accelerator lane (NPU/Vulkan/GPU) is active, so
 * the role-aware runtime scheduler should be used. On a CPU-only build we stay
 * on the in-place dequant path to avoid the runtime's weight-copy overhead. */
static int qwen_accelerator_active(void) {
#if COLI_HAS_BACKEND
    int mask = coli_runtime_backend_mask();
    return (mask & ~COLI_RUNTIME_BACKEND_CPU_BIT) != 0;
#else
    return 0;
#endif
}

/* Role-aware quantized matmul. When an accelerator is present the work is routed
 * through the parallel role-aware runtime (which offloads and can split the
 * output across CPU/NPU/GPU lanes); otherwise it runs the in-place CPU dequant
 * kernel. `x` is a single token (S=1). */
static void matmul_qt(QTensor *w, const float *x, float *out, coli_op_role role) {
    if (w->I <= 0 || w->O <= 0) return;
#if COLI_HAS_BACKEND
    if (qwen_accelerator_active()) {
        if (coli_runtime_matmul_ex(&w->handle, out, x, w->data, w->scales,
                                   w->fmt, 1, w->I, w->O, 0, role)) {
            return;
        }
    }
#else
    (void)role;
#endif
    qwen_cpu_qmatmul(out, x, w->data, w->scales, w->fmt, w->I, w->O);
}

static float *load_tensor_f32(qwen35_model *m, const char *name, size_t nelems) {
    model_debug("load_tensor_f32: tensor=%s expected_nelems=%zu", name, nelems);
    float *buf = (float *)qwen_calloc(nelems, sizeof(float), "tensor buffer");
    st_tensor *t = find_tensor(m, name);
    if (!t) {
        model_debug("load_tensor_f32: tensor=%s not found; returning zeroed buffer", name);
        return buf;
    }
    model_debug("load_tensor_f32: tensor=%s found dtype=%d numel=%" PRId64 " nbytes=%" PRId64, name, t->dtype, t->numel, t->nbytes);
    if (t->dtype == 3) {
        st_tensor *scale = find_scale_tensor(m, name);
        if (scale) {
            model_debug("load_tensor_f32: tensor=%s scale=%s scale_elems=%" PRId64, name, scale->name, scale->numel);
        } else {
            model_debug("load_tensor_f32: tensor=%s missing scale tensor", name);
        }
        if (!scale) {
            fprintf(stderr, "warning: missing scale for quantized tensor %s; assuming unit scale\n", name);
        }
        size_t out_dim = scale ? (size_t)scale->numel : 1;
        if (out_dim == 0 || nelems % out_dim != 0) {
            fprintf(stderr, "warning: incompatible scale shape for quantized tensor %s (scale elems=%" PRId64 "); assuming unit scale\n", name, scale ? scale->numel : 0);
            out_dim = 1;
        }
        size_t in_dim = nelems / out_dim;
        size_t bytes_per_row = (in_dim + 1) / 2;
        size_t packed_bytes = (size_t)t->nbytes;
        if (packed_bytes == nelems) {
            uint8_t *raw = (uint8_t *)qwen_malloc(packed_bytes, "quantized payload");
            st_read_raw(&m->shards, t->name, raw, 0);
            float *scale_vals = (float *)qwen_calloc(out_dim, sizeof(float), "quantized scales");
            if (scale) {
                st_read_f32(&m->shards, scale->name, scale_vals, 0);
            } else {
                for (size_t row = 0; row < out_dim; row++) scale_vals[row] = 1.0f;
            }
            for (size_t row = 0; row < out_dim; row++) {
                for (size_t col = 0; col < in_dim; col++) {
                    int8_t value = (int8_t)raw[row * in_dim + col];
                    buf[row * in_dim + col] = (float)value * scale_vals[row];
                }
            }
            free(raw);
            free(scale_vals);
            return buf;
        }
        if (packed_bytes == out_dim * bytes_per_row) {
            uint8_t *raw = (uint8_t *)qwen_malloc(packed_bytes, "quantized payload");
            st_read_raw(&m->shards, t->name, raw, 0);
            float *scale_vals = (float *)qwen_calloc(out_dim, sizeof(float), "quantized scales");
            if (scale) {
                st_read_f32(&m->shards, scale->name, scale_vals, 0);
            } else {
                for (size_t row = 0; row < out_dim; row++) scale_vals[row] = 1.0f;
            }
            for (size_t row = 0; row < out_dim; row++) {
                for (size_t col = 0; col < in_dim; col++) {
                    size_t byte_index = row * bytes_per_row + col / 2;
                    uint8_t byte = raw[byte_index];
                    int nibble = (col & 1) ? (int)((byte >> 4) & 0x0f) : (int)(byte & 0x0f);
                    int8_t value = (int8_t)nibble - 8;
                    buf[row * in_dim + col] = (float)value * scale_vals[row];
                }
            }
            free(raw);
            free(scale_vals);
            return buf;
        }
        /* Ornith int8 layout: payload/scale rows are doubled, while the logical tensor
         * has half as many rows (packed bytes are exactly 2x logical elements). */
        bool has_ornith_doubled_int8_payload = (packed_bytes % 2) == 0 && packed_bytes / 2 == nelems && out_dim > 1 && (out_dim % 2) == 0;
        if (has_ornith_doubled_int8_payload) {
            size_t logical_out_dim = out_dim / 2;
            if (logical_out_dim > 0 && nelems % logical_out_dim == 0) {
                size_t logical_in_dim = nelems / logical_out_dim;
                if (packed_bytes == out_dim * logical_in_dim) {
                    uint8_t *raw = (uint8_t *)qwen_malloc(packed_bytes, "quantized payload");
                    st_read_raw(&m->shards, t->name, raw, 0);
                    float *scale_vals = (float *)qwen_calloc(out_dim, sizeof(float), "quantized scales");
                    if (scale) {
                        st_read_f32(&m->shards, scale->name, scale_vals, 0);
                    } else {
                        for (size_t row = 0; row < out_dim; row++) scale_vals[row] = 1.0f;
                    }
                    bool interleaved_rows;
                    if (scale) {
                        size_t split_matches = 0;
                        size_t interleaved_matches = 0;
                        for (size_t row = 0; row < logical_out_dim; row++) {
                            size_t split_left = row;
                            size_t split_right = row + logical_out_dim;
                            size_t interleaved_left = row * 2;
                            size_t interleaved_right = interleaved_left + 1;
                            if (split_right >= out_dim || interleaved_right >= out_dim) {
                                free(raw);
                                free(scale_vals);
                                fail("tensor %s has invalid expanded scale layout (row=%zu out=%zu)", name, row, out_dim);
                            }
                            if (fabsf(scale_vals[split_left] - scale_vals[split_right]) < SCALE_LAYOUT_EPSILON) split_matches++;
                            if (fabsf(scale_vals[interleaved_left] - scale_vals[interleaved_right]) < SCALE_LAYOUT_EPSILON) interleaved_matches++;
                        }
                        interleaved_rows = interleaved_matches > split_matches; /* ties intentionally fall back to consecutive layout */
                    } else {
                        interleaved_rows = false; /* without scales, prefer consecutive first-half rows (most conservative) */
                    }
                    for (size_t row = 0; row < logical_out_dim; row++) {
                        size_t src_row = interleaved_rows ? (row * 2) : row;
                        size_t scale_row = interleaved_rows ? (row * 2) : row;
                        if (src_row >= out_dim || scale_row >= out_dim) {
                            free(raw);
                            free(scale_vals);
                            fail("tensor %s has invalid expanded row mapping (src=%zu scale=%zu out=%zu)", name, src_row, scale_row, out_dim);
                        }
                        for (size_t col = 0; col < logical_in_dim; col++) {
                            size_t src_offset = src_row * logical_in_dim + col;
                            if (src_offset >= packed_bytes) {
                                free(raw);
                                free(scale_vals);
                                fail("tensor %s has invalid expanded payload offset (%zu >= %zu)", name, src_offset, packed_bytes);
                            }
                            int8_t value = (int8_t)raw[src_offset];
                            buf[row * logical_in_dim + col] = (float)value * scale_vals[scale_row];
                        }
                    }
                    free(raw);
                    free(scale_vals);
                    model_debug(
                        "load_tensor_f32: tensor=%s accepting expanded int8 payload (%zu bytes -> %zu elems): decoded %zu rows from %s packed layout",
                        name, packed_bytes, nelems, logical_out_dim, interleaved_rows ? "interleaved" : "consecutive"
                    );
                    return buf;
                }
            }
        }
        fail("tensor %s has unsupported packed size %" PRId64 " for %zu elements", name, t->nbytes, nelems);
    }
    if (t->numel != (int64_t)nelems) {
        fail("tensor %s has %" PRId64 " elements (expected %zu)", name, t->numel, nelems);
    }
    st_read_f32(&m->shards, t->name, buf, 0);
    return buf;
}

static float *load_optional_tensor_f32(qwen35_model *m, const char *name, size_t nelems) {
    if (!tensor_exists(m, name)) return NULL;
    return load_tensor_f32(m, name, nelems);
}

/* Load a weight into a QTensor, preserving the native quantized payload
 * (INT8 -> fmt 1, INT4 -> fmt 2) when it is stored in a standard row-major
 * packed layout with matching per-output-row scales. This keeps the compact
 * representation resident (1 byte/elem for INT8, 0.5 byte/elem for INT4) instead
 * of expanding to F32, which is the source of the memory savings. Irregular
 * quantized layouts (e.g. the Ornith doubled/expanded int8 payloads), missing
 * scales, and genuine F32 tensors fall back to a dequantized fmt 0 buffer using
 * the existing, well-tested load_tensor_f32 decoder. */
static QTensor load_qtensor(qwen35_model *m, const char *name, int O, int I, bool *found) {
    QTensor q;
    q.fmt = 0;
    q.O = O;
    q.I = I;
    q.data = NULL;
    q.scales = NULL;
    q.handle = NULL;
    const size_t nelems = (size_t)O * (size_t)I;
    st_tensor *t = find_tensor(m, name);
    if (!t) {
        if (found) *found = false;
        q.data = qwen_calloc(nelems, sizeof(float), "qtensor zero");
        return q;
    }
    if (found) *found = true;
    if (t->dtype == 3) {
        st_tensor *scale = find_scale_tensor(m, name);
        const size_t out_dim = scale ? (size_t)scale->numel : 0;
        const size_t packed_bytes = (size_t)t->nbytes;
        const size_t bytes_per_row_int4 = ((size_t)I + 1) / 2;
        if (scale && out_dim == (size_t)O) {
            if (packed_bytes == nelems) {
                /* standard INT8: one signed byte per element */
                uint8_t *raw = (uint8_t *)qwen_malloc(packed_bytes, "qtensor int8 payload");
                st_read_raw(&m->shards, t->name, raw, 0);
                float *sc = (float *)qwen_calloc(out_dim, sizeof(float), "qtensor scales");
                st_read_f32(&m->shards, scale->name, sc, 0);
                q.fmt = 1;
                q.data = raw;
                q.scales = sc;
                return q;
            }
            if (packed_bytes == out_dim * bytes_per_row_int4) {
                /* standard INT4: two nibbles per byte, biased by 8 */
                uint8_t *raw = (uint8_t *)qwen_malloc(packed_bytes, "qtensor int4 payload");
                st_read_raw(&m->shards, t->name, raw, 0);
                float *sc = (float *)qwen_calloc(out_dim, sizeof(float), "qtensor scales");
                st_read_f32(&m->shards, scale->name, sc, 0);
                q.fmt = 2;
                q.data = raw;
                q.scales = sc;
                return q;
            }
        }
        /* irregular / unsupported quantized layout: dequantize to F32 */
        q.fmt = 0;
        q.data = load_tensor_f32(m, name, nelems);
        return q;
    }
    /* genuine F32 tensor */
    q.fmt = 0;
    q.data = load_tensor_f32(m, name, nelems);
    return q;
}

/* Load a QTensor into `dst`, overriding it only when the tensor is present in
 * the checkpoint. When absent, the identity/default QTensor already installed by
 * init_layer_defaults is kept, preserving the historical missing-weight
 * behavior. */
static void load_qtensor_into(qwen35_model *m, const char *name, int O, int I, QTensor *dst) {
    bool found = false;
    QTensor q = load_qtensor(m, name, O, I, &found);
    if (found) {
        qt_free(dst);
        *dst = q;
    } else {
        qt_free(&q);
    }
}

static bool load_packed_qkv_tensor(qwen35_model *m, const char *name, int q_out, int kv_out, int hidden_size, QTensor *q_proj, QTensor *k_proj, QTensor *v_proj) {
    if (!tensor_exists(m, name)) return false;
    const size_t q_elems = (size_t)q_out * (size_t)hidden_size;
    const size_t kv_elems = (size_t)kv_out * (size_t)hidden_size;
    const size_t packed_elems = q_elems + 2 * kv_elems;
    float *packed = load_tensor_f32(m, name, packed_elems);
    float *q_buf = qwen_malloc(q_elems * sizeof(float), "packed qkv q_proj");
    float *k_buf = qwen_malloc(kv_elems * sizeof(float), "packed qkv k_proj");
    float *v_buf = qwen_malloc(kv_elems * sizeof(float), "packed qkv v_proj");
    memcpy(q_buf, packed, q_elems * sizeof(float));
    memcpy(k_buf, packed + q_elems, kv_elems * sizeof(float));
    memcpy(v_buf, packed + q_elems + kv_elems, kv_elems * sizeof(float));
    free(packed);
    qt_free(q_proj);
    qt_free(k_proj);
    qt_free(v_proj);
    q_proj->fmt = 0; q_proj->O = q_out; q_proj->I = hidden_size; q_proj->data = q_buf; q_proj->scales = NULL; q_proj->handle = NULL;
    k_proj->fmt = 0; k_proj->O = kv_out; k_proj->I = hidden_size; k_proj->data = k_buf; k_proj->scales = NULL; k_proj->handle = NULL;
    v_proj->fmt = 0; v_proj->O = kv_out; v_proj->I = hidden_size; v_proj->data = v_buf; v_proj->scales = NULL; v_proj->handle = NULL;
    return true;
}

static void init_layer_defaults(QLayer *layer, int hidden_size, int moe_intermediate_size, int shared_expert_intermediate_size, int num_experts, int num_attention_heads, int num_kv_heads, int head_dim) {
    memset(layer, 0, sizeof(*layer));
    layer->in_ln = (float *)qwen_calloc((size_t)hidden_size, sizeof(float), "layer norm");
    layer->post_ln = (float *)qwen_calloc((size_t)hidden_size, sizeof(float), "layer norm");
    for (int i = 0; i < hidden_size; i++) {
        layer->in_ln[i] = 1.0f;
        layer->post_ln[i] = 1.0f;
    }
    layer->la_state = (float *)qwen_calloc((size_t)hidden_size, sizeof(float), "linear attention state");
    const int q_out = num_attention_heads * head_dim;
    const int kv_out = num_kv_heads * head_dim;
    layer->q_proj = qt_alloc_f32(q_out, hidden_size, "q_proj defaults");
    layer->k_proj = qt_alloc_f32(kv_out, hidden_size, "k_proj defaults");
    layer->v_proj = qt_alloc_f32(kv_out, hidden_size, "v_proj defaults");
    layer->o_proj = qt_alloc_f32(hidden_size, q_out, "o_proj defaults");
    layer->mlp_gate_proj = qt_alloc_f32(moe_intermediate_size, hidden_size, "mlp gate defaults");
    layer->mlp_up_proj = qt_alloc_f32(moe_intermediate_size, hidden_size, "mlp up defaults");
    layer->mlp_down_proj = qt_alloc_f32(hidden_size, moe_intermediate_size, "mlp down defaults");
    layer->router = qt_alloc_f32(num_experts, hidden_size, "router defaults");
    layer->sh_gate = qt_alloc_f32(shared_expert_intermediate_size, hidden_size, "shared gate defaults");
    layer->sh_up = qt_alloc_f32(shared_expert_intermediate_size, hidden_size, "shared up defaults");
    layer->sh_down = qt_alloc_f32(hidden_size, shared_expert_intermediate_size, "shared down defaults");
    for (int i = 0; i < q_out; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) qt_f32(&layer->q_proj)[i * hidden_size + j] = 1.0f;
        }
    }
    for (int i = 0; i < kv_out; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) qt_f32(&layer->k_proj)[i * hidden_size + j] = 0.25f;
            if (i == j) qt_f32(&layer->v_proj)[i * hidden_size + j] = 0.125f;
        }
    }
    for (int i = 0; i < hidden_size; i++) {
        for (int j = 0; j < q_out; j++) {
            if (i == j) qt_f32(&layer->o_proj)[i * q_out + j] = 0.5f;
        }
    }
    for (int i = 0; i < moe_intermediate_size; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) qt_f32(&layer->mlp_gate_proj)[i * hidden_size + j] = 0.5f;
            if (i == j) qt_f32(&layer->mlp_up_proj)[i * hidden_size + j] = 0.25f;
        }
    }
    for (int i = 0; i < hidden_size; i++) {
        for (int j = 0; j < moe_intermediate_size; j++) {
            if (i == j) qt_f32(&layer->mlp_down_proj)[i * moe_intermediate_size + j] = 0.75f;
        }
    }
    for (int i = 0; i < shared_expert_intermediate_size; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) qt_f32(&layer->sh_gate)[i * hidden_size + j] = 0.25f;
            if (i == j) qt_f32(&layer->sh_up)[i * hidden_size + j] = 0.1f;
        }
    }
    for (int i = 0; i < hidden_size; i++) {
        for (int j = 0; j < shared_expert_intermediate_size; j++) {
            if (i == j) qt_f32(&layer->sh_down)[i * shared_expert_intermediate_size + j] = 0.2f;
        }
    }
    layer->expert_gate_proj = (QTensor *)qwen_calloc((size_t)num_experts, sizeof(QTensor), "expert table");
    layer->expert_up_proj = (QTensor *)qwen_calloc((size_t)num_experts, sizeof(QTensor), "expert table");
    layer->expert_down_proj = (QTensor *)qwen_calloc((size_t)num_experts, sizeof(QTensor), "expert table");
    for (int expert = 0; expert < num_experts; expert++) {
        layer->expert_gate_proj[expert] = qt_alloc_f32(moe_intermediate_size, hidden_size, "expert gate defaults");
        layer->expert_up_proj[expert] = qt_alloc_f32(moe_intermediate_size, hidden_size, "expert up defaults");
        layer->expert_down_proj[expert] = qt_alloc_f32(hidden_size, moe_intermediate_size, "expert down defaults");
    }
}

static void init_model(qwen35_model *m, const char *snap_dir) {
    memset(m, 0, sizeof(*m));
    m->snap_dir = qwen_strdup(snap_dir);
    char *config_path = join_path(snap_dir, "config.json");
    char *json_text = read_text_file(config_path);
    free(config_path);
    char *arena = NULL;
    jval *root = json_parse(json_text, &arena);
    free(json_text);
    if (!root || root->t != J_OBJ) fail("invalid config.json");
    jval *cfg = root;
    jval *text_cfg = json_get(cfg, "text_config");
    if (!text_cfg || text_cfg->t != J_OBJ) text_cfg = cfg;
    m->vocab_size = parse_int_field_with_fallback(text_cfg, cfg, "vocab_size", 32);
    m->hidden_size = parse_int_field_with_fallback(text_cfg, cfg, "hidden_size", 16);
    m->num_layers = parse_int_field_with_fallback(text_cfg, cfg, "num_hidden_layers", 1);
    m->num_experts = parse_int_field_with_fallback(text_cfg, cfg, "num_experts", 2);
    m->experts_per_tok = parse_int_field_with_fallback(text_cfg, cfg, "num_experts_per_tok", 1);
    m->moe_intermediate_size = parse_int_field_with_fallback(text_cfg, cfg, "moe_intermediate_size", m->hidden_size);
    m->shared_expert_intermediate_size = parse_int_field_with_fallback(text_cfg, cfg, "shared_expert_intermediate_size", m->moe_intermediate_size);
    m->num_attention_heads = parse_int_field_with_fallback(text_cfg, cfg, "num_attention_heads", 1);
    m->num_kv_heads = parse_int_field_with_fallback(text_cfg, cfg, "num_key_value_heads", 1);
    m->head_dim = parse_int_field_with_fallback(text_cfg, cfg, "head_dim", m->hidden_size);
    m->linear_num_key_heads = parse_int_field_with_fallback(text_cfg, cfg, "linear_num_key_heads", m->num_kv_heads);
    m->linear_key_head_dim = parse_int_field_with_fallback(text_cfg, cfg, "linear_key_head_dim", m->head_dim);
    m->linear_num_value_heads = parse_int_field_with_fallback(text_cfg, cfg, "linear_num_value_heads", m->num_kv_heads);
    m->linear_value_head_dim = parse_int_field_with_fallback(text_cfg, cfg, "linear_value_head_dim", m->head_dim);
    m->rope_theta = parse_float_field(text_cfg, "rope_theta", 10000.0f);
    m->partial_rotary_factor = parse_float_field(text_cfg, "partial_rotary_factor", 0.25f);
    m->use_rope = m->rope_theta > 0.0f && m->partial_rotary_factor > 0.0f && m->head_dim > 1;
    m->has_router = parse_bool_field(cfg, "has_moe_router") || m->num_experts > 0;
    const char *kv_slots_env = getenv("KV_SLOTS");
    m->kv_slots = 1;
    if (kv_slots_env && *kv_slots_env) {
        char *end = NULL;
        long parsed = strtol(kv_slots_env, &end, 10);
        if (end && *end == '\0' && parsed > 0 && parsed <= MAX_KV_SLOTS) m->kv_slots = (int)parsed;
    }
    m->kv_cache_k_slots = (float ***)qwen_calloc((size_t)m->num_layers, sizeof(*m->kv_cache_k_slots), "kv cache table");
    m->kv_cache_v_slots = (float ***)qwen_calloc((size_t)m->num_layers, sizeof(*m->kv_cache_v_slots), "kv cache table");
    m->kv_cache_lens = (int **)qwen_calloc((size_t)m->num_layers, sizeof(*m->kv_cache_lens), "kv cache table");
    m->kv_cache_caps = (int **)qwen_calloc((size_t)m->num_layers, sizeof(*m->kv_cache_caps), "kv cache table");
    for (int layer = 0; layer < m->num_layers; layer++) {
        m->kv_cache_k_slots[layer] = (float **)qwen_calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_k_slots[layer]), "kv cache slot table");
        m->kv_cache_v_slots[layer] = (float **)qwen_calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_v_slots[layer]), "kv cache slot table");
        m->kv_cache_lens[layer] = (int *)qwen_calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_lens[layer]), "kv cache lens");
        m->kv_cache_caps[layer] = (int *)qwen_calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_caps[layer]), "kv cache caps");
    }
    model_debug("init_model: parsed geometry vocab=%d hidden=%d layers=%d experts=%d experts_per_tok=%d moe_intermediate=%d shared_expert_intermediate=%d heads=%d kv_heads=%d head_dim=%d", m->vocab_size, m->hidden_size, m->num_layers, m->num_experts, m->experts_per_tok, m->moe_intermediate_size, m->shared_expert_intermediate_size, m->num_attention_heads, m->num_kv_heads, m->head_dim);
    if (m->vocab_size <= 0 || m->hidden_size <= 0 || m->num_layers <= 0) {
        fail("invalid qwen config: vocab/hidden/layer sizes must be positive");
    }
    if (m->num_attention_heads <= 0) m->num_attention_heads = 1;
    if (m->num_kv_heads <= 0) m->num_kv_heads = 1;
    if (m->head_dim <= 0) m->head_dim = m->hidden_size;
    if (m->moe_intermediate_size <= 0) m->moe_intermediate_size = m->hidden_size;
    if (m->shared_expert_intermediate_size <= 0) m->shared_expert_intermediate_size = m->moe_intermediate_size;
    st_init(&m->shards, snap_dir);

    size_t embed_nelems = (size_t)m->vocab_size * (size_t)m->hidden_size;
    model_debug("init_model: loading model.embed_tokens.weight with expected_nelems=%zu", embed_nelems);
    m->embed = load_tensor_f32(m, "model.embed_tokens.weight", embed_nelems);
    m->final_norm = load_optional_tensor_f32(m, "model.norm.weight", (size_t)m->hidden_size);
    if (!m->final_norm) {
        m->final_norm = (float *)qwen_calloc((size_t)m->hidden_size, sizeof(float), "final norm");
        for (int i = 0; i < m->hidden_size; i++) m->final_norm[i] = 1.0f;
    }

    m->lm_head = load_optional_tensor_f32(m, "lm_head.weight", (size_t)m->vocab_size * (size_t)m->hidden_size);
    if (!m->lm_head) {
        m->lm_head = (float *)qwen_malloc((size_t)m->vocab_size * (size_t)m->hidden_size * sizeof(float), "lm_head");
        memcpy(m->lm_head, m->embed, (size_t)m->vocab_size * (size_t)m->hidden_size * sizeof(float));
    }

    m->layers = (QLayer *)qwen_calloc((size_t)m->num_layers, sizeof(*m->layers), "layer table");
    m->layer_types = (int *)qwen_calloc((size_t)m->num_layers, sizeof(*m->layer_types), "layer type table");

    jval *layer_types = json_get(text_cfg, "layer_types");
    if (layer_types && layer_types->t == J_ARR && layer_types->len > 0) {
        for (int layer = 0; layer < m->num_layers; layer++) {
            int type = LAYER_TYPE_FULL;
            if (layer < layer_types->len && layer_types->kids[layer] && layer_types->kids[layer]->t == J_STR) {
                if (strcmp(layer_types->kids[layer]->str, "linear_attention") == 0) {
                    type = LAYER_TYPE_LINEAR;
                }
            }
            m->layer_types[layer] = type;
        }
    } else {
        for (int layer = 0; layer < m->num_layers; layer++) m->layer_types[layer] = LAYER_TYPE_FULL;
    }

    for (int layer = 0; layer < m->num_layers; layer++) {
        QLayer *cur = &m->layers[layer];
        init_layer_defaults(cur, m->hidden_size, m->moe_intermediate_size, m->shared_expert_intermediate_size, m->num_experts, m->num_attention_heads, m->num_kv_heads, m->head_dim);
        cur->is_full_attn = m->layer_types[layer] == LAYER_TYPE_FULL;
        char name[1024];
        snprintf(name, sizeof(name), "model.layers.%d.input_layernorm.weight", layer);
        cur->in_ln = load_optional_tensor_f32(m, name, (size_t)m->hidden_size);
        if (!cur->in_ln) {
            cur->in_ln = (float *)qwen_calloc((size_t)m->hidden_size, sizeof(float), "layer norm");
            for (int i = 0; i < m->hidden_size; i++) cur->in_ln[i] = 1.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.post_attention_layernorm.weight", layer);
        cur->post_ln = load_optional_tensor_f32(m, name, (size_t)m->hidden_size);
        if (!cur->post_ln) {
            cur->post_ln = (float *)qwen_calloc((size_t)m->hidden_size, sizeof(float), "layer norm");
            for (int i = 0; i < m->hidden_size; i++) cur->post_ln[i] = 1.0f;
        }
        if (cur->is_full_attn) {
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.q_norm.weight", layer);
            cur->q_norm = load_optional_tensor_f32(m, name, (size_t)m->head_dim);
            if (!cur->q_norm) {
                cur->q_norm = (float *)qwen_calloc((size_t)m->head_dim, sizeof(float), "q_norm");
                for (int i = 0; i < m->head_dim; i++) cur->q_norm[i] = 1.0f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.k_norm.weight", layer);
            cur->k_norm = load_optional_tensor_f32(m, name, (size_t)m->head_dim);
            if (!cur->k_norm) {
                cur->k_norm = (float *)qwen_calloc((size_t)m->head_dim, sizeof(float), "k_norm");
                for (int i = 0; i < m->head_dim; i++) cur->k_norm[i] = 1.0f;
            }
            const int q_out = m->num_attention_heads * m->head_dim;
            const int kv_out = m->num_kv_heads * m->head_dim;
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.qkv.weight", layer);
            if (!load_packed_qkv_tensor(m, name, q_out, kv_out, m->hidden_size, &cur->q_proj, &cur->k_proj, &cur->v_proj)) {
                snprintf(name, sizeof(name), "model.layers.%d.self_attn.q_proj.weight", layer);
                load_qtensor_into(m, name, q_out, m->hidden_size, &cur->q_proj);
                snprintf(name, sizeof(name), "model.layers.%d.self_attn.k_proj.weight", layer);
                load_qtensor_into(m, name, kv_out, m->hidden_size, &cur->k_proj);
                snprintf(name, sizeof(name), "model.layers.%d.self_attn.v_proj.weight", layer);
                load_qtensor_into(m, name, kv_out, m->hidden_size, &cur->v_proj);
            }
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.o_proj.weight", layer);
            load_qtensor_into(m, name, m->hidden_size, q_out, &cur->o_proj);
            snprintf(name, sizeof(name), "model.layers.%d.mlp.gate.weight", layer);
            {
                bool router_found = false;
                QTensor router_q = load_qtensor(m, name, m->num_experts, m->hidden_size, &router_found);
                if (router_found) {
                    qt_free(&cur->router);
                    cur->router = router_q;
                    m->has_router = true;
                } else {
                    qt_free(&router_q);
                }
            }
            snprintf(name, sizeof(name), "model.layers.%d.mlp.gate_proj.weight", layer);
            load_qtensor_into(m, name, m->moe_intermediate_size, m->hidden_size, &cur->mlp_gate_proj);
            snprintf(name, sizeof(name), "model.layers.%d.mlp.up_proj.weight", layer);
            load_qtensor_into(m, name, m->moe_intermediate_size, m->hidden_size, &cur->mlp_up_proj);
            snprintf(name, sizeof(name), "model.layers.%d.mlp.down_proj.weight", layer);
            load_qtensor_into(m, name, m->hidden_size, m->moe_intermediate_size, &cur->mlp_down_proj);
            /* Experts stream in lazily; free the defaults installed above and
             * mark every expert slot unloaded until first use. */
            for (int expert = 0; expert < m->num_experts; expert++) {
                qt_free(&cur->expert_gate_proj[expert]);
                qt_free(&cur->expert_up_proj[expert]);
                qt_free(&cur->expert_down_proj[expert]);
            }
            cur->expert_state = (int *)qwen_calloc((size_t)m->num_experts, sizeof(int), "expert state table");
            for (int expert = 0; expert < m->num_experts; expert++) {
                cur->expert_state[expert] = QWEN_EXPERT_STATE_UNLOADED;
            }
        } else {
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_a.weight", layer);
            cur->la_in_proj_a = load_optional_tensor_f32(m, name, (size_t)m->linear_num_key_heads * (size_t)m->hidden_size);
            if (!cur->la_in_proj_a) {
                cur->la_in_proj_a = (float *)qwen_calloc((size_t)m->linear_num_key_heads * (size_t)m->hidden_size, sizeof(float));
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_b.weight", layer);
            cur->la_in_proj_b = load_optional_tensor_f32(m, name, (size_t)m->linear_num_key_heads * (size_t)m->hidden_size);
            if (!cur->la_in_proj_b) {
                cur->la_in_proj_b = (float *)qwen_calloc((size_t)m->linear_num_key_heads * (size_t)m->hidden_size, sizeof(float));
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_qkv.weight", layer);
            cur->la_in_proj_qkv = load_optional_tensor_f32(m, name, (size_t)3 * (size_t)m->hidden_size * (size_t)m->hidden_size);
            if (!cur->la_in_proj_qkv) {
                cur->la_in_proj_qkv = (float *)qwen_calloc((size_t)3 * (size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_z.weight", layer);
            cur->la_in_proj_z = load_optional_tensor_f32(m, name, (size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim * (size_t)m->hidden_size);
            if (!cur->la_in_proj_z) {
                cur->la_in_proj_z = (float *)qwen_calloc((size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim * (size_t)m->hidden_size, sizeof(float));
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.out_proj.weight", layer);
            cur->la_out_proj = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim);
            if (!cur->la_out_proj) {
                cur->la_out_proj = (float *)qwen_calloc((size_t)m->hidden_size * (size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim, sizeof(float));
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.norm.weight", layer);
            cur->la_norm = load_optional_tensor_f32(m, name, (size_t)m->linear_key_head_dim);
            if (!cur->la_norm) {
                cur->la_norm = (float *)qwen_calloc((size_t)m->linear_key_head_dim, sizeof(float));
                for (int i = 0; i < m->linear_key_head_dim; i++) cur->la_norm[i] = 1.0f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.A_log.weight", layer);
            cur->la_A_log = load_optional_tensor_f32(m, name, (size_t)m->linear_num_value_heads);
            if (!cur->la_A_log) {
                cur->la_A_log = (float *)qwen_calloc((size_t)m->linear_num_value_heads, sizeof(float));
                for (int i = 0; i < m->linear_num_value_heads; i++) cur->la_A_log[i] = 0.1f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.dt_bias.weight", layer);
            cur->la_dt_bias = load_optional_tensor_f32(m, name, (size_t)m->linear_num_value_heads);
            if (!cur->la_dt_bias) {
                cur->la_dt_bias = (float *)qwen_calloc((size_t)m->linear_num_value_heads, sizeof(float));
                for (int i = 0; i < m->linear_num_value_heads; i++) cur->la_dt_bias[i] = 0.01f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.conv1d.weight", layer);
            cur->la_conv1d = load_optional_tensor_f32(m, name, (size_t)3 * (size_t)m->hidden_size * 1 * 4);
            if (!cur->la_conv1d) {
                cur->la_conv1d = (float *)qwen_calloc((size_t)3 * (size_t)m->hidden_size * 1 * 4, sizeof(float));
            }
        }
        /* shared expert is present on ALL layer types (full and linear attention). */
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert.gate_proj.weight", layer);
        load_qtensor_into(m, name, m->shared_expert_intermediate_size, m->hidden_size, &cur->sh_gate);
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert.up_proj.weight", layer);
        load_qtensor_into(m, name, m->shared_expert_intermediate_size, m->hidden_size, &cur->sh_up);
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert.down_proj.weight", layer);
        load_qtensor_into(m, name, m->hidden_size, m->shared_expert_intermediate_size, &cur->sh_down);
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert_gate.weight", layer);
        cur->shared_expert_gate = load_optional_tensor_f32(m, name, (size_t)m->hidden_size);
        if (!cur->shared_expert_gate) {
            cur->shared_expert_gate = (float *)qwen_calloc((size_t)m->hidden_size, sizeof(float));
        }
    }
    free(arena);
}

static void free_layer(qwen35_model *m, QLayer *layer) {
    free(layer->in_ln);
    free(layer->post_ln);
    free(layer->q_norm);
    free(layer->k_norm);
    qt_free(&layer->q_proj);
    qt_free(&layer->k_proj);
    qt_free(&layer->v_proj);
    qt_free(&layer->o_proj);
    qt_free(&layer->router);
    qt_free(&layer->sh_gate);
    qt_free(&layer->sh_up);
    qt_free(&layer->sh_down);
    free(layer->shared_expert_gate);
    qt_free(&layer->mlp_gate_proj);
    qt_free(&layer->mlp_up_proj);
    qt_free(&layer->mlp_down_proj);
    free(layer->la_in_proj_a);
    free(layer->la_in_proj_b);
    free(layer->la_in_proj_qkv);
    free(layer->la_in_proj_z);
    free(layer->la_out_proj);
    free(layer->la_norm);
    free(layer->la_A_log);
    free(layer->la_dt_bias);
    free(layer->la_conv1d);
    free(layer->la_state);
    free(layer->kv_cache_k);
    free(layer->kv_cache_v);
    if (layer->expert_gate_proj) {
        for (int expert = 0; expert < m->num_experts; expert++) {
            qt_free(&layer->expert_gate_proj[expert]);
        }
        free(layer->expert_gate_proj);
    }
    if (layer->expert_up_proj) {
        for (int expert = 0; expert < m->num_experts; expert++) {
            qt_free(&layer->expert_up_proj[expert]);
        }
        free(layer->expert_up_proj);
    }
    if (layer->expert_down_proj) {
        for (int expert = 0; expert < m->num_experts; expert++) {
            qt_free(&layer->expert_down_proj[expert]);
        }
        free(layer->expert_down_proj);
    }
    free(layer->expert_state);
}

static void free_model(qwen35_model *m) {
    free(m->snap_dir);
    free(m->embed);
    free(m->final_norm);
    free(m->lm_head);
    for (int i = 0; i < m->num_layers; i++) free_layer(m, &m->layers[i]);
    free(m->layers);
    free(m->layer_types);
    for (int layer = 0; layer < m->num_layers; layer++) {
        free(m->kv_cache_k_slots[layer]);
        free(m->kv_cache_v_slots[layer]);
        free(m->kv_cache_lens[layer]);
        free(m->kv_cache_caps[layer]);
    }
    free(m->kv_cache_k_slots);
    free(m->kv_cache_v_slots);
    free(m->kv_cache_lens);
    free(m->kv_cache_caps);
}

typedef enum {
    QWEN_BACKEND_NONE = 0,
    QWEN_BACKEND_ROCM = 1,
    QWEN_BACKEND_NPU = 2,
} qwen_backend_kind_t;

static int g_qwen_backend_kind = QWEN_BACKEND_NONE;
static int g_qwen_backend_checked = 0;
static int g_qwen_backend_ready = 0;

static int qwen_backend_device_nodes_available(void) {
    return access("/dev/dri", F_OK) == 0 && access("/dev/kfd", F_OK) == 0;
}

static int qwen_backend_init_once(void) {
    if (g_qwen_backend_checked) return g_qwen_backend_ready;
    int init_result = 0;
#ifdef _OPENMP
#pragma omp critical(qwen_backend_init_once)
#endif
    {
        if (g_qwen_backend_checked) {
            init_result = g_qwen_backend_ready;
        } else {
            g_qwen_backend_checked = 1;
            g_qwen_backend_kind = QWEN_BACKEND_NONE;
            g_qwen_backend_ready = 0;
#if defined(COLI_ROCM)
            if (qwen_backend_device_nodes_available()) {
                int devices[1] = {0};
                if (coli_rocm_init(devices, 1)) {
                    g_qwen_backend_kind = QWEN_BACKEND_ROCM;
                    g_qwen_backend_ready = 1;
                } else {
#if defined(COLI_ENABLE_NPU)
                    fprintf(stderr, "[qwen35_moe] ROCm init failed; trying the NPU fallback backend\n");
#else
                    fprintf(stderr, "[qwen35_moe] ROCm init failed; no fallback backend is available\n");
#endif
                }
            }
#endif
#if defined(COLI_ENABLE_NPU)
            if (!g_qwen_backend_ready) {
                int devices[1] = {0};
                if (coli_npu_compat_init(devices, 1)) {
                    g_qwen_backend_kind = QWEN_BACKEND_NPU;
                    g_qwen_backend_ready = 1;
                }
            }
#endif
            init_result = g_qwen_backend_ready;
        }
    }
    return init_result;
}

/* Select the initialized backend once and dispatch matmul calls to it.
 * Returns 1 when the chosen backend accepted the operation and 0 otherwise,
 * allowing the caller to fall back to the CPU implementation. */
static int qwen_backend_matmul(ColiCudaTensor **tensor, float *y, const float *x, const void *weights,
                               const float *scales, int fmt, int S, int I, int O, int device) {
    if (!qwen_backend_init_once()) return 0;
#if defined(COLI_ROCM)
    if (g_qwen_backend_kind == QWEN_BACKEND_ROCM) {
        return coli_rocm_matmul(tensor, y, x, weights, scales, fmt, S, I, O, device);
    }
#endif
#if defined(COLI_ENABLE_NPU)
    if (g_qwen_backend_kind == QWEN_BACKEND_NPU) {
        return coli_npu_compat_matmul(tensor, y, x, weights, scales, fmt, S, I, O, device);
    }
#endif
    return 0;
}

static void qwen_backend_tensor_free(ColiCudaTensor *tensor) {
    if (g_qwen_backend_ready) {
#if defined(COLI_ROCM)
        if (g_qwen_backend_kind == QWEN_BACKEND_ROCM) {
            coli_rocm_tensor_free(tensor);
            return;
        }
#endif
#if defined(COLI_ENABLE_NPU)
        if (g_qwen_backend_kind == QWEN_BACKEND_NPU) {
            coli_npu_compat_tensor_free(tensor);
            return;
        }
#endif
    }
#if !defined(COLI_ROCM) && !defined(COLI_ENABLE_NPU)
    coli_cuda_tensor_free(tensor);
#endif
}

static void matmul_vec(const float *x, const float *w, int in_dim, int out_dim, float *out) {
    if (in_dim <= 0 || out_dim <= 0) {
        fprintf(stderr, "warning: invalid matmul dimensions (%d, %d)\n", in_dim, out_dim);
        return;
    }
#if COLI_HAS_BACKEND
    {
        const size_t matmul_size = (size_t)in_dim * (size_t)out_dim;
        if (matmul_size >= COLI_MATMUL_BACKEND_THRESHOLD) {
            ColiCudaTensor *tensor = NULL;
            if (qwen_backend_matmul(&tensor, out, x, w, NULL, 0, 1, in_dim, out_dim, 0)) {
                if (tensor) qwen_backend_tensor_free(tensor);
                return;
            }
            if (tensor) qwen_backend_tensor_free(tensor);
        }
    }
#endif
#ifdef _OPENMP
#pragma omp parallel for schedule(static)
#endif
    for (int out_idx = 0; out_idx < out_dim; out_idx++) {
        float sum = 0.0f;
        const size_t row_offset = (size_t)out_idx * (size_t)in_dim;
        const float *weight_row = w + row_offset;
        for (int in_idx = 0; in_idx < in_dim; in_idx++) {
            sum += x[in_idx] * weight_row[in_idx];
        }
        out[out_idx] = sum;
    }
}

static void layer_norm_inplace(float *x, const float *scale, int dim) {
    for (int i = 0; i < dim; i++) {
        x[i] *= scale ? scale[i] : 1.0f;
    }
}

static float relu(float x) {
    return x > 0.0f ? x : 0.0f;
}

static float silu(float x) {
    float sigmoid = 1.0f / (1.0f + expf(-x));
    return x * sigmoid;
}

static void softmax(float *values, int n) {
    if (n <= 0) {
        fprintf(stderr, "warning: softmax received empty input\n");
        return;
    }
    float maxv = values[0];
    for (int i = 1; i < n; i++) if (values[i] > maxv) maxv = values[i];
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        values[i] = expf(values[i] - maxv);
        sum += values[i];
    }
    for (int i = 0; i < n; i++) values[i] /= sum;
}

static void topk_select(const float *scores, int n, int k, int *out_indices, float *out_weights) {
    if (n <= 0 || k <= 0) return;
    k = k < n ? k : n;
    int *indices = (int *)qwen_malloc((size_t)n * sizeof(int));
    for (int i = 0; i < n; i++) indices[i] = i;
    for (int i = 0; i < n; i++) {
        int best = i;
        for (int j = i + 1; j < n; j++) {
            if (scores[indices[j]] > scores[indices[best]]) best = j;
        }
        int tmp = indices[i];
        indices[i] = indices[best];
        indices[best] = tmp;
    }
    for (int i = 0; i < k; i++) {
        out_indices[i] = indices[i];
        out_weights[i] = scores[indices[i]];
    }
    free(indices);
}

static void blend_vector(float *out, const float *a, const float *b, int n) {
    for (int i = 0; i < n; i++) out[i] = a[i] + b[i];
}

static void apply_rope(float *values, int head_dim, int position, float theta, float partial_factor) {
    if (!values || head_dim < 2 || position < 0) return;
    int rotary_dim = (int)floorf(head_dim * partial_factor);
    if (rotary_dim <= 0) rotary_dim = 2;
    if (rotary_dim > head_dim) rotary_dim = head_dim;
    if ((rotary_dim & 1) != 0) rotary_dim -= 1;
    if (rotary_dim <= 0) return;
    for (int pair = 0; pair < rotary_dim / 2; pair++) {
        int base = pair * 2;
        float x0 = values[base];
        float x1 = values[base + 1];
        float inv_freq = 1.0f / powf(theta, (float)(2 * pair) / (float)head_dim);
        float angle = (float)position * inv_freq;
        float cos_a = cosf(angle);
        float sin_a = sinf(angle);
        values[base] = x0 * cos_a - x1 * sin_a;
        values[base + 1] = x0 * sin_a + x1 * cos_a;
    }
}

static void sample_logits(const float *logits, int n, float temperature, float top_p, float min_p, int top_k, int *out_token) {
    if (n <= 0) return;
    if (temperature <= 0.0f) {
        int best = 0;
        float best_score = logits[0];
        for (int i = 1; i < n; i++) {
            if (logits[i] > best_score) {
                best_score = logits[i];
                best = i;
            }
        }
        *out_token = best;
        return;
    }
    int *indices = NULL;
    float *values = NULL;
    int *kept = NULL;
    float *keep_scores = NULL;
    float *probs = NULL;
    int *filtered_indices = NULL;
    float *filtered_probs = NULL;
    int best = 0;
    indices = (int *)qwen_malloc((size_t)n * sizeof(int));
    values = (float *)qwen_malloc((size_t)n * sizeof(float));
    if (!indices || !values) {
        free(indices);
        free(values);
        fail("out of memory");
    }
    for (int i = 0; i < n; i++) {
        indices[i] = i;
        values[i] = logits[i];
    }
    for (int i = 0; i < n; i++) {
        int local_best = i;
        for (int j = i + 1; j < n; j++) {
            if (values[indices[j]] > values[indices[local_best]]) local_best = j;
        }
        int tmp = indices[i];
        indices[i] = indices[local_best];
        indices[local_best] = tmp;
    }
    int keep = n;
    if (top_k > 0 && top_k < keep) keep = top_k;
    kept = (int *)qwen_malloc((size_t)keep * sizeof(int));
    keep_scores = (float *)qwen_malloc((size_t)keep * sizeof(float));
    if (!kept || !keep_scores) {
        free(kept);
        free(keep_scores);
        free(values);
        free(indices);
        fail("out of memory");
    }
    for (int i = 0; i < keep; i++) {
        kept[i] = indices[i];
        keep_scores[i] = values[indices[i]];
    }
    float maxv = keep_scores[0];
    for (int i = 1; i < keep; i++) if (keep_scores[i] > maxv) maxv = keep_scores[i];
    float sum = 0.0f;
    probs = (float *)qwen_malloc((size_t)keep * sizeof(float));
    if (!probs) {
        free(probs);
        free(keep_scores);
        free(kept);
        free(values);
        free(indices);
        fail("out of memory");
    }
    for (int i = 0; i < keep; i++) {
        probs[i] = expf((keep_scores[i] - maxv) / temperature);
        sum += probs[i];
    }
    if (min_p > 0.0f) {
        filtered_indices = (int *)qwen_malloc((size_t)keep * sizeof(int));
        filtered_probs = (float *)qwen_malloc((size_t)keep * sizeof(float));
        if (!filtered_indices || !filtered_probs) {
            free(filtered_indices);
            free(filtered_probs);
            free(probs);
            free(keep_scores);
            free(kept);
            free(values);
            free(indices);
            fail("out of memory");
        }
        float max_prob = 0.0f;
        for (int i = 0; i < keep; i++) {
            if (probs[i] > max_prob) {
                max_prob = probs[i];
            }
        }
        float threshold = max_prob * min_p;
        int filtered_count = 0;
        for (int i = 0; i < keep; i++) {
            if (probs[i] >= threshold) {
                filtered_indices[filtered_count] = kept[i];
                filtered_probs[filtered_count] = probs[i];
                filtered_count++;
            }
        }
        if (filtered_count == 0) {
            filtered_indices[0] = kept[0];
            filtered_probs[0] = probs[0];
            filtered_count = 1;
        }
        free(kept);
        free(probs);
        keep = filtered_count;
        kept = filtered_indices;
        probs = filtered_probs;
        sum = 0.0f;
        for (int i = 0; i < keep; i++) sum += probs[i];
    }
    if (top_p > 0.0f && top_p < 1.0f) {
        float target = sum * top_p;
        float cumulative = 0.0f;
        int limit = keep;
        for (int i = 0; i < keep; i++) {
            cumulative += probs[i];
            if (cumulative >= target) {
                limit = i + 1;
                break;
            }
        }
        keep = limit;
        sum = 0.0f;
        for (int i = 0; i < keep; i++) {
            sum += probs[i];
        }
    }
    if (keep <= 0) keep = 1;
    float r = (float)rand() / ((float)RAND_MAX + 1.0f) * sum;
    float cumsum = 0.0f;
    best = kept[0];
    for (int i = 0; i < keep; i++) {
        cumsum += probs[i];
        if (cumsum >= r) {
            best = kept[i];
            break;
        }
    }
    *out_token = best;
    free(probs);
    free(keep_scores);
    free(kept);
    free(values);
    free(indices);
}

static bool expert_tables_ready(const QLayer *cur) {
    return cur != NULL && cur->expert_state != NULL && cur->expert_gate_proj != NULL &&
           cur->expert_up_proj != NULL && cur->expert_down_proj != NULL;
}

static void ensure_expert(qwen35_model *m, QLayer *cur, int layer, int expert_idx) {
    if (!expert_tables_ready(cur)) {
        return;
    }
    if (expert_idx < 0 || expert_idx >= m->num_experts) {
        return;
    }
    if (cur->expert_state[expert_idx] != QWEN_EXPERT_STATE_UNLOADED) {
        return;
    }
    char expert_name[1024];
    snprintf(expert_name, sizeof(expert_name), "model.layers.%d.mlp.experts.%d.gate_proj.weight", layer, expert_idx);
    cur->expert_gate_proj[expert_idx] = load_qtensor(m, expert_name, m->moe_intermediate_size, m->hidden_size, NULL);
    snprintf(expert_name, sizeof(expert_name), "model.layers.%d.mlp.experts.%d.up_proj.weight", layer, expert_idx);
    cur->expert_up_proj[expert_idx] = load_qtensor(m, expert_name, m->moe_intermediate_size, m->hidden_size, NULL);
    snprintf(expert_name, sizeof(expert_name), "model.layers.%d.mlp.experts.%d.down_proj.weight", layer, expert_idx);
    cur->expert_down_proj[expert_idx] = load_qtensor(m, expert_name, m->hidden_size, m->moe_intermediate_size, NULL);
    cur->expert_state[expert_idx] = QWEN_EXPERT_STATE_RESIDENT;
}

static void ensure_cache_slot(qwen35_model *m, QLayer *cur, int layer, int slot, int steps) {
    if (slot < 0 || slot >= m->kv_slots) {
        int invalid_slot = slot;
        slot = 0;
        fprintf(stderr, "[qwen35_moe] invalid cache slot %d; using slot 0\n", invalid_slot);
    }
    if (!m->kv_cache_k_slots[layer] || !m->kv_cache_v_slots[layer] || !m->kv_cache_lens[layer] || !m->kv_cache_caps[layer]) fail("invalid cache state");
    int kv_dim = m->num_kv_heads * m->head_dim;
    int *cap = &m->kv_cache_caps[layer][slot];
    if (!m->kv_cache_k_slots[layer][slot] || !m->kv_cache_v_slots[layer][slot] || *cap < steps) {
        size_t bytes = (size_t)steps * (size_t)kv_dim * sizeof(float);
        float *new_k = (float *)qwen_realloc(m->kv_cache_k_slots[layer][slot], bytes, "kv cache k resize");
        float *new_v = (float *)qwen_realloc(m->kv_cache_v_slots[layer][slot], bytes, "kv cache v resize");
        m->kv_cache_k_slots[layer][slot] = new_k;
        m->kv_cache_v_slots[layer][slot] = new_v;
        *cap = steps;
    }
    cur->kv_cache_k = m->kv_cache_k_slots[layer][slot];
    cur->kv_cache_v = m->kv_cache_v_slots[layer][slot];
    cur->kv_cache_cap = *cap;
    cur->kv_cache_len = m->kv_cache_lens[layer][slot];
}

static void save_cache_slot(qwen35_model *m, QLayer *cur, int layer, int slot) {
    if (slot < 0 || slot >= m->kv_slots) {
        int invalid_slot = slot;
        slot = 0;
        fprintf(stderr, "[qwen35_moe] invalid cache slot %d; using slot 0\n", invalid_slot);
    }
    m->kv_cache_lens[layer][slot] = cur->kv_cache_len;
    m->kv_cache_caps[layer][slot] = cur->kv_cache_cap;
}

static void configure_parallelism(int requested_threads) {
#ifdef _OPENMP
    if (requested_threads > 0) {
        omp_set_dynamic(0);
        omp_set_num_threads(requested_threads);
        return;
    }
    omp_set_dynamic(0);
    omp_set_num_threads(coli_detect_thread_count());
#endif
}

static void run_model(qwen35_model *m, const int *tokens, int n_tokens, int steps,
                      int *out_tokens, float temperature, float top_p, float min_p, int top_k, unsigned int seed,
                      int cache_slot) {
    float *hidden = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
    for (int layer = 0; layer < m->num_layers; layer++) {
        QLayer *cur = &m->layers[layer];
        ensure_cache_slot(m, cur, layer, cache_slot, steps);
        cur->kv_cache_len = m->kv_cache_lens[layer][cache_slot];
        cur->kv_cache_cap = m->kv_cache_caps[layer][cache_slot];
    }
    if (temperature > 0.0f) {
        if (seed != 0) srand(seed);
        else srand((unsigned int)time(NULL));
    }
    for (int step = 0; step < steps; step++) {
        int token_id = step < n_tokens ? tokens[step] : (step > 0 ? out_tokens[step - 1] : tokens[0]);
        for (int i = 0; i < m->hidden_size; i++) hidden[i] = m->embed[token_id * m->hidden_size + i];
        for (int layer = 0; layer < m->num_layers; layer++) {
            QLayer *cur = &m->layers[layer];
            float *residual = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
            memcpy(residual, hidden, (size_t)m->hidden_size * sizeof(float));

            if (cur->is_full_attn) {
                const int q_dim = m->num_attention_heads * m->head_dim;
                const int kv_dim = m->num_kv_heads * m->head_dim;
                float *q_out = (float *)qwen_malloc((size_t)q_dim * sizeof(float));
                float *k_out = (float *)qwen_malloc((size_t)kv_dim * sizeof(float));
                float *v_out = (float *)qwen_malloc((size_t)kv_dim * sizeof(float));
                float *attn = (float *)qwen_malloc((size_t)q_dim * sizeof(float));
                float *post = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
                if (!q_out || !k_out || !v_out || !attn || !post) fail("out of memory");
                matmul_qt(&cur->q_proj, hidden, q_out, COLI_OP_ROLE_ATTENTION);
                matmul_qt(&cur->k_proj, hidden, k_out, COLI_OP_ROLE_ATTENTION);
                matmul_qt(&cur->v_proj, hidden, v_out, COLI_OP_ROLE_ATTENTION);
                for (int head = 0; head < m->num_attention_heads; head++) {
                    for (int dim = 0; dim < m->head_dim; dim++) {
                        int index = head * m->head_dim + dim;
                        float scale = cur->q_norm ? cur->q_norm[dim % m->head_dim] : 1.0f;
                        q_out[index] *= scale;
                    }
                }
                for (int head = 0; head < m->num_kv_heads; head++) {
                    for (int dim = 0; dim < m->head_dim; dim++) {
                        int index = head * m->head_dim + dim;
                        float scale = cur->k_norm ? cur->k_norm[dim % m->head_dim] : 1.0f;
                        k_out[index] *= scale;
                    }
                }
                int position = cur->kv_cache_len;
                if (m->use_rope) {
                    for (int head = 0; head < m->num_attention_heads; head++) {
                        apply_rope(q_out + head * m->head_dim, m->head_dim, position, m->rope_theta, m->partial_rotary_factor);
                    }
                }
                int kv_repeat = m->num_attention_heads / (m->num_kv_heads > 0 ? m->num_kv_heads : 1);
                size_t cache_offset = (size_t)position * (size_t)kv_dim;
                for (int i = 0; i < kv_dim; i++) {
                    cur->kv_cache_k[cache_offset + i] = k_out[i];
                    cur->kv_cache_v[cache_offset + i] = v_out[i];
                }
                cur->kv_cache_len++;
                save_cache_slot(m, cur, layer, cache_slot);
                for (int head = 0; head < m->num_attention_heads; head++) {
                    int kv_head = head / (kv_repeat > 0 ? kv_repeat : 1);
                    float *scores = (float *)qwen_calloc((size_t)cur->kv_cache_len, sizeof(float));
                    if (!scores) {
                        free(q_out);
                        free(k_out);
                        free(v_out);
                        free(attn);
                        free(post);
                        fail("out of memory");
                    }
                    for (int pos = 0; pos < cur->kv_cache_len; pos++) {
                        float score = 0.0f;
                        size_t key_offset = (size_t)pos * (size_t)kv_dim + (size_t)kv_head * (size_t)m->head_dim;
                        for (int dim = 0; dim < m->head_dim; dim++) {
                            score += q_out[head * m->head_dim + dim] * cur->kv_cache_k[key_offset + dim];
                        }
                        scores[pos] = score / sqrtf((float)m->head_dim);
                    }
                    softmax(scores, cur->kv_cache_len);
                    for (int dim = 0; dim < m->head_dim; dim++) {
                        float value = 0.0f;
                        for (int pos = 0; pos < cur->kv_cache_len; pos++) {
                            size_t key_offset = (size_t)pos * (size_t)kv_dim + (size_t)kv_head * (size_t)m->head_dim;
                            value += scores[pos] * cur->kv_cache_v[key_offset + dim];
                        }
                        attn[head * m->head_dim + dim] = value;
                    }
                    free(scores);
                }
                matmul_qt(&cur->o_proj, attn, post, COLI_OP_ROLE_ATTENTION);
                layer_norm_inplace(hidden, cur->in_ln, m->hidden_size);
                for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + post[i];
                free(q_out);
                free(k_out);
                free(v_out);
                free(attn);
                free(post);
            } else {
                float *la_out = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
                float *la_gate = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
                if (!la_out || !la_gate) fail("out of memory");
                matmul_vec(hidden, cur->la_in_proj_qkv, m->hidden_size, m->hidden_size, la_out);
                matmul_vec(hidden, cur->la_in_proj_z, m->hidden_size, m->hidden_size, la_gate);
                for (int i = 0; i < m->hidden_size; i++) {
                    float gate = silu(la_gate[i]);
                    float conv_term = 0.0f;
                    if (cur->la_conv1d) {
                        conv_term = cur->la_conv1d[i % LA_CONV1D_WIDTH];
                    }
                    float base = silu(la_out[i]) * 0.5f * (1.0f + gate);
                    if (cur->la_state) {
                        cur->la_state[i] = (cur->la_state[i] * LA_STATE_DECAY) + (base * LA_STATE_UPDATE) + conv_term;
                        la_out[i] = cur->la_state[i];
                    } else {
                        la_out[i] = base + conv_term;
                    }
                }
                layer_norm_inplace(hidden, cur->in_ln, m->hidden_size);
                for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + la_out[i];
                free(la_out);
                free(la_gate);
            }

            float *ffn_in = (float *)qwen_malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *ffn_mid = (float *)qwen_malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *ffn_out = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
            if (!ffn_in || !ffn_mid || !ffn_out) fail("out of memory");
            if (cur->is_full_attn) {
                matmul_qt(&cur->mlp_gate_proj, hidden, ffn_in, COLI_OP_ROLE_DENSE);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_in[i] = silu(ffn_in[i]);
                matmul_qt(&cur->mlp_up_proj, hidden, ffn_mid, COLI_OP_ROLE_DENSE);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_mid[i] = silu(ffn_mid[i]);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_in[i] *= ffn_mid[i];
                matmul_qt(&cur->mlp_down_proj, ffn_in, ffn_out, COLI_OP_ROLE_DENSE);
            } else {
                for (int i = 0; i < m->hidden_size; i++) ffn_out[i] = 0.0f;
            }

            if (cur->is_full_attn && m->has_router && cur->router.data) {
                float *router_logits = (float *)qwen_malloc((size_t)m->num_experts * sizeof(float));
                matmul_qt(&cur->router, hidden, router_logits, COLI_OP_ROLE_SMALL);
                softmax(router_logits, m->num_experts);
                int *expert_indices = (int *)qwen_malloc((size_t)m->experts_per_tok * sizeof(int));
                float *expert_weights = (float *)qwen_malloc((size_t)m->experts_per_tok * sizeof(float));
                if (!expert_indices || !expert_weights) fail("out of memory");
                topk_select(router_logits, m->num_experts, m->experts_per_tok, expert_indices, expert_weights);
                for (int expert_slot = 0; expert_slot < m->experts_per_tok; expert_slot++) {
                    int expert_idx = expert_indices[expert_slot];
                    float weight = expert_weights[expert_slot];
                    ensure_expert(m, cur, layer, expert_idx);
                    if (weight <= 0.0f || !cur->expert_gate_proj[expert_idx].data || !cur->expert_up_proj[expert_idx].data || !cur->expert_down_proj[expert_idx].data) continue;
                    float *expert_gate = (float *)qwen_malloc((size_t)m->moe_intermediate_size * sizeof(float));
                    float *expert_up = (float *)qwen_malloc((size_t)m->moe_intermediate_size * sizeof(float));
                    float *expert_res = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
                    if (!expert_gate || !expert_up || !expert_res) fail("out of memory");
                    matmul_qt(&cur->expert_gate_proj[expert_idx], hidden, expert_gate, COLI_OP_ROLE_ROUTED_EXPERT);
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_gate[i] = silu(expert_gate[i]);
                    matmul_qt(&cur->expert_up_proj[expert_idx], hidden, expert_up, COLI_OP_ROLE_ROUTED_EXPERT);
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_up[i] = silu(expert_up[i]);
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_gate[i] *= expert_up[i];
                    matmul_qt(&cur->expert_down_proj[expert_idx], expert_gate, expert_res, COLI_OP_ROLE_ROUTED_EXPERT);
                    for (int i = 0; i < m->hidden_size; i++) ffn_out[i] += weight * expert_res[i];
                    free(expert_gate);
                    free(expert_up);
                    free(expert_res);
                }
                free(router_logits);
                free(expert_indices);
                free(expert_weights);
            }

            float *shared_in = (float *)qwen_malloc((size_t)m->shared_expert_intermediate_size * sizeof(float));
            float *shared_mid = (float *)qwen_malloc((size_t)m->shared_expert_intermediate_size * sizeof(float));
            float *shared_out = (float *)qwen_malloc((size_t)m->hidden_size * sizeof(float));
            if (!shared_in || !shared_mid || !shared_out) fail("out of memory");
            /* shared expert gate: gate_scalar = sigmoid(shared_expert_gate . hidden) */
            float gate_scalar = 1.0f;
            if (cur->shared_expert_gate) {
                float dot = 0.0f;
                for (int i = 0; i < m->hidden_size; i++) dot += cur->shared_expert_gate[i] * hidden[i];
                gate_scalar = 1.0f / (1.0f + expf(-dot));
            }
            matmul_qt(&cur->sh_gate, hidden, shared_in, COLI_OP_ROLE_SHARED_EXPERT);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_in[i] = silu(shared_in[i]);
            matmul_qt(&cur->sh_up, hidden, shared_mid, COLI_OP_ROLE_SHARED_EXPERT);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_mid[i] = silu(shared_mid[i]);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_in[i] *= shared_mid[i];
            matmul_qt(&cur->sh_down, shared_in, shared_out, COLI_OP_ROLE_SHARED_EXPERT);
            for (int i = 0; i < m->hidden_size; i++) ffn_out[i] += gate_scalar * shared_out[i];
            free(shared_in);
            free(shared_mid);
            free(shared_out);

            layer_norm_inplace(hidden, cur->post_ln, m->hidden_size);
            for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + ffn_out[i];
            free(residual);
            free(ffn_in);
            free(ffn_mid);
            free(ffn_out);
        }
        layer_norm_inplace(hidden, m->final_norm, m->hidden_size);
        float *logits = (float *)qwen_calloc((size_t)m->vocab_size, sizeof(float));
        for (int vocab = 0; vocab < m->vocab_size; vocab++) {
            float sum = 0.0f;
            for (int i = 0; i < m->hidden_size; i++) {
                sum += hidden[i] * m->lm_head[vocab * m->hidden_size + i];
            }
            logits[vocab] = sum;
        }
        int best = 0;
        if (temperature <= 0.0f) {
            best = 0;
            float best_score = logits[0];
            for (int vocab = 1; vocab < m->vocab_size; vocab++) {
                if (logits[vocab] > best_score) {
                    best_score = logits[vocab];
                    best = vocab;
                }
            }
        } else {
            sample_logits(logits, m->vocab_size, temperature, top_p, min_p, top_k, &best);
        }
        out_tokens[step] = best;
        free(logits);
    }
    free(hidden);
}

static int parse_token_ids(const char *text, int *out, int max_ids) {
    if (!text || !*text) return 0;
    char *copy = qwen_strdup(text);
    char *cursor = copy;
    int count = 0;
    while (*cursor && count < max_ids) {
        while (*cursor && isspace((unsigned char)*cursor)) cursor++;
        if (!*cursor) break;
        char *end = cursor;
        while (*end && !isspace((unsigned char)*end) && *end != ',') end++;
        if (*end) *end++ = '\0';
        if (isdigit((unsigned char)cursor[0]) || (cursor[0] == '-' && isdigit((unsigned char)cursor[1]))) {
            out[count++] = atoi(cursor);
        } else {
            unsigned long hash = 1469598103934665603ULL;
            for (const unsigned char *p = (const unsigned char *)cursor; *p; p++) {
                hash ^= *p;
                hash *= 1099511628211ULL;
            }
            out[count++] = (int)(hash % 1000000);
        }
        cursor = end;
    }
    free(copy);
    return count;
}

static int read_exact(FILE *fp, unsigned char *buf, size_t len) {
    size_t total = 0;
    while (total < len) {
        size_t got = fread(buf + total, 1, len - total, fp);
        if (got == 0) return 0;
        total += got;
    }
    return 1;
}

static void usage(const char *prog) {
    fprintf(stderr,
            "usage: %s [--model DIR] [--prompt TEXT] [--steps N] [--threads N] [--temperature F] [--top-k N] [--top-p F] [--min-p F] [--seed N] [--debug] [--verbose] [--ram-limit-mb N]\n",
            prog);
}

static void run_server(qwen35_model *model, int max_tokens, float temperature, float top_p, float min_p, int top_k, unsigned int seed) {
    printf("\x01\x01READY\x01\x01\n");
    printf("STAT 0 0.00 0.0 0.00\n");
    fflush(stdout);
    while (1) {
        char header[4096];
        if (!fgets(header, sizeof(header), stdin)) break;
        if (strncmp(header, "\x02PROMPT ", 9) != 0 && strcmp(header, "\x02MORE\n") != 0) {
            continue;
        }
        if (strncmp(header, "\x02PROMPT ", 9) != 0) {
            continue;
        }
        size_t payload_len = 0;
        int max_tokens_request = max_tokens;
        float temperature_request = temperature;
        float top_p_request = top_p;
        float min_p_request = min_p;
        int top_k_request = top_k;
        unsigned int seed_request = seed;
        char *cursor = header + 9;
        int cache_slot_request = 0;
        int parsed = sscanf(cursor, "%zu %d %f %f %d %d %u %f", &payload_len, &max_tokens_request,
                            &temperature_request, &top_p_request, &cache_slot_request, &top_k_request,
                            &seed_request, &min_p_request);
        if (parsed < 7) {
            fprintf(stderr, "[qwen35_moe] malformed prompt header: %s\n", header);
            continue;
        }
        if (parsed < 8) {
            min_p_request = 0.0f;
        }
        if (payload_len > 0) {
            unsigned char *payload = (unsigned char *)qwen_malloc(payload_len + 1, "server payload buffer");
            if (!read_exact(stdin, payload, payload_len)) {
                fprintf(stderr, "[qwen35_moe] short read while reading prompt payload\n");
                free(payload);
                printf("ERR short read\x01\x01END\x01\x01\n");
                printf("STAT 0 0.00 0.0 0.00\n");
                fflush(stdout);
                break;
            }
            payload[payload_len] = '\0';
            char *prompt_text = (char *)payload;
            int *tokens = (int *)qwen_calloc((size_t)max_tokens_request, sizeof(int));
            int *out = (int *)qwen_calloc((size_t)max_tokens_request, sizeof(int));
            if (!tokens || !out) {
                free(tokens);
                free(out);
                free(payload);
                fail("out of memory");
            }
            int n_tokens = parse_token_ids(prompt_text, tokens, max_tokens_request);
            if (n_tokens <= 0) n_tokens = 1;
            run_model(model, tokens, n_tokens, max_tokens_request, out, temperature_request, top_p_request, min_p_request, top_k_request, seed, cache_slot_request);
            for (int i = 0; i < max_tokens_request; i++) {
                if (i > 0) fputc(' ', stdout);
                fprintf(stdout, "%d", out[i]);
            }
            printf("\x01\x01END\x01\x01\n");
            printf("STAT %d 0.00 0.0 0.00\n", max_tokens_request);
            fflush(stdout);
            free(tokens);
            free(out);
            free(payload);
        } else {
            printf("\x01\x01END\x01\x01\n");
            printf("STAT 0 0.00 0.0 0.00\n");
            fflush(stdout);
        }
    }
}

int main(int argc, char **argv) {
    const char *snap_dir = NULL;
    const char *prompt = NULL;
    int steps = 4;
    int threads = 0;
    float temperature = 0.0f;
    float top_p = 0.0f;
    float min_p = 0.0f;
    int top_k = 0;
    unsigned int seed = 0;
    bool debug_enabled = false;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--model") && i + 1 < argc) {
            snap_dir = argv[++i];
        } else if (!strcmp(argv[i], "--prompt") && i + 1 < argc) {
            prompt = argv[++i];
        } else if (!strcmp(argv[i], "--steps") && i + 1 < argc) {
            steps = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--threads") && i + 1 < argc) {
            const char *threads_arg = argv[++i];
            if (!coli_parse_positive_int(threads_arg, &threads)) {
                fprintf(stderr, "error: invalid thread count: %s\n", threads_arg);
                return 1;
            }
        } else if (!strcmp(argv[i], "--temperature") && i + 1 < argc) {
            temperature = (float)atof(argv[++i]);
        } else if (!strcmp(argv[i], "--top-k") && i + 1 < argc) {
            top_k = atoi(argv[++i]);
        } else if (!strcmp(argv[i], "--top-p") && i + 1 < argc) {
            top_p = (float)atof(argv[++i]);
        } else if (!strcmp(argv[i], "--min-p") && i + 1 < argc) {
            min_p = (float)atof(argv[++i]);
        } else if (!strcmp(argv[i], "--seed") && i + 1 < argc) {
            seed = (unsigned int)strtoul(argv[++i], NULL, 10);
        } else if (!strcmp(argv[i], "--debug") || !strcmp(argv[i], "--verbose")) {
            debug_enabled = true;
        } else if (!strcmp(argv[i], "--ram-limit-mb")) {
            if (i + 1 >= argc) {
                fprintf(stderr, "error: missing value for --ram-limit-mb\n");
                return 1;
            }
            configure_ram_limit(argv[++i]);
        } else {
            usage(argv[0]);
            return 1;
        }
    }
    if (!snap_dir) {
        snap_dir = getenv("SNAP");
    }
    if (!snap_dir) {
        usage(argv[0]);
        return 1;
    }
    if (debug_enabled) set_model_debug_enabled(true);
    if (temperature > 0.0f && seed == 0) srand((unsigned int)time(NULL));
    configure_parallelism(threads);
    qwen35_model model;
    init_model(&model, snap_dir);
    if (getenv("SERVE")) {
        run_server(&model, steps, temperature, top_p, min_p, top_k, seed);
        free_model(&model);
        return 0;
    }
    int *tokens = (int *)qwen_calloc((size_t)steps, sizeof(int));
    int n_tokens = parse_token_ids(prompt ? prompt : "0", tokens, steps);
    if (n_tokens <= 0) n_tokens = 1;
    int *out = (int *)qwen_calloc((size_t)steps, sizeof(int));
    run_model(&model, tokens, n_tokens, steps, out, temperature, top_p, min_p, top_k, seed, 0);
    for (int i = 0; i < steps; i++) {
        printf("%d\n", out[i]);
    }
    free(tokens);
    free(out);
    free_model(&model);
    return 0;
}
