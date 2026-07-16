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

typedef struct {
    float *in_ln;
    float *post_ln;
    bool is_full_attn;
    float *q_norm;
    float *k_norm;
    float *q_proj;
    float *k_proj;
    float *v_proj;
    float *o_proj;
    float *router;
    float *sh_gate;
    float *sh_up;
    float *sh_down;
    float *shared_expert_gate;
    float *mlp_gate_proj;
    float *mlp_up_proj;
    float *mlp_down_proj;
    float **expert_gate_proj;
    float **expert_up_proj;
    float **expert_down_proj;
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

static char *read_text_file(const char *path) {
    FILE *fp = fopen(path, "rb");
    if (!fp) fail("cannot open %s", path);
    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    rewind(fp);
    char *buf = (char *)malloc((size_t)size + 1);
    if (!buf) fail("out of memory");
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

static bool model_debug_enabled(void) {
    static bool initialized = false;
    static bool enabled = false;
    if (!initialized) {
        enabled = parse_bool_env("COLI_QWEN_DEBUG");
        initialized = true;
    }
    return enabled;
}

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
    char *out = (char *)malloc(n);
    if (!out) fail("out of memory");
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
            char *candidate = (char *)malloc(total);
            if (!candidate) fail("out of memory");
            snprintf(candidate, total, "%s%s", to, name + from_len);
            t = st_find(&m->shards, candidate);
            free(candidate);
            if (t) return t;
        }
    }
    return NULL;
}

static st_tensor *find_scale_tensor(qwen35_model *m, const char *name) {
    static const char *const suffixes[] = {".qs", ".scale", ".scales", ".weight_scale"};
    for (size_t i = 0; i < sizeof(suffixes) / sizeof(suffixes[0]); i++) {
        size_t total = strlen(name) + strlen(suffixes[i]) + 1;
        char *candidate = (char *)malloc(total);
        if (!candidate) fail("out of memory");
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

static float *load_tensor_f32(qwen35_model *m, const char *name, size_t nelems) {
    model_debug("load_tensor_f32: tensor=%s expected_nelems=%zu", name, nelems);
    float *buf = (float *)calloc(nelems, sizeof(float));
    if (!buf) fail("out of memory");
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
            uint8_t *raw = (uint8_t *)malloc(packed_bytes);
            if (!raw) fail("out of memory");
            st_read_raw(&m->shards, t->name, raw, 0);
            float *scale_vals = (float *)calloc(out_dim, sizeof(float));
            if (!scale_vals) fail("out of memory");
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
            uint8_t *raw = (uint8_t *)malloc(packed_bytes);
            if (!raw) fail("out of memory");
            st_read_raw(&m->shards, t->name, raw, 0);
            float *scale_vals = (float *)calloc(out_dim, sizeof(float));
            if (!scale_vals) fail("out of memory");
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

static bool load_packed_qkv_tensor(qwen35_model *m, const char *name, int q_out, int kv_out, int hidden_size, float **q_proj, float **k_proj, float **v_proj) {
    if (!tensor_exists(m, name)) return false;
    const size_t q_elems = (size_t)q_out * (size_t)hidden_size;
    const size_t kv_elems = (size_t)kv_out * (size_t)hidden_size;
    const size_t packed_elems = q_elems + 2 * kv_elems;
    float *packed = load_tensor_f32(m, name, packed_elems);
    float *q_buf = malloc(q_elems * sizeof(float));
    if (!q_buf) {
        free(packed);
        fail("out of memory allocating q_proj buffer");
    }
    float *k_buf = malloc(kv_elems * sizeof(float));
    if (!k_buf) {
        free(q_buf);
        free(packed);
        fail("out of memory allocating k_proj buffer");
    }
    float *v_buf = malloc(kv_elems * sizeof(float));
    if (!v_buf) {
        free(q_buf);
        free(k_buf);
        free(packed);
        fail("out of memory allocating v_proj buffer");
    }
    memcpy(q_buf, packed, q_elems * sizeof(float));
    memcpy(k_buf, packed + q_elems, kv_elems * sizeof(float));
    memcpy(v_buf, packed + q_elems + kv_elems, kv_elems * sizeof(float));
    free(packed);
    *q_proj = q_buf;
    *k_proj = k_buf;
    *v_proj = v_buf;
    return true;
}

static void init_layer_defaults(QLayer *layer, int hidden_size, int moe_intermediate_size, int shared_expert_intermediate_size, int num_experts, int num_attention_heads, int num_kv_heads, int head_dim) {
    memset(layer, 0, sizeof(*layer));
    layer->in_ln = (float *)calloc((size_t)hidden_size, sizeof(float));
    layer->post_ln = (float *)calloc((size_t)hidden_size, sizeof(float));
    if (!layer->in_ln || !layer->post_ln) fail("out of memory");
    for (int i = 0; i < hidden_size; i++) {
        layer->in_ln[i] = 1.0f;
        layer->post_ln[i] = 1.0f;
    }
    layer->la_state = (float *)calloc((size_t)hidden_size, sizeof(float));
    if (!layer->la_state) fail("out of memory");
    const int q_out = num_attention_heads * head_dim;
    const int kv_out = num_kv_heads * head_dim;
    layer->q_proj = (float *)calloc((size_t)q_out * (size_t)hidden_size, sizeof(float));
    layer->k_proj = (float *)calloc((size_t)kv_out * (size_t)hidden_size, sizeof(float));
    layer->v_proj = (float *)calloc((size_t)kv_out * (size_t)hidden_size, sizeof(float));
    layer->o_proj = (float *)calloc((size_t)hidden_size * (size_t)q_out, sizeof(float));
    layer->mlp_gate_proj = (float *)calloc((size_t)moe_intermediate_size * (size_t)hidden_size, sizeof(float));
    layer->mlp_up_proj = (float *)calloc((size_t)moe_intermediate_size * (size_t)hidden_size, sizeof(float));
    layer->mlp_down_proj = (float *)calloc((size_t)hidden_size * (size_t)moe_intermediate_size, sizeof(float));
    layer->router = (float *)calloc((size_t)num_experts * (size_t)hidden_size, sizeof(float));
    layer->sh_gate = (float *)calloc((size_t)shared_expert_intermediate_size * (size_t)hidden_size, sizeof(float));
    layer->sh_up = (float *)calloc((size_t)shared_expert_intermediate_size * (size_t)hidden_size, sizeof(float));
    layer->sh_down = (float *)calloc((size_t)hidden_size * (size_t)shared_expert_intermediate_size, sizeof(float));
    if (!layer->q_proj || !layer->k_proj || !layer->v_proj || !layer->o_proj || !layer->mlp_gate_proj || !layer->mlp_up_proj || !layer->mlp_down_proj || !layer->router || !layer->sh_gate || !layer->sh_up || !layer->sh_down) fail("out of memory");
    for (int i = 0; i < q_out; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) layer->q_proj[i * hidden_size + j] = 1.0f;
        }
    }
    for (int i = 0; i < kv_out; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) layer->k_proj[i * hidden_size + j] = 0.25f;
            if (i == j) layer->v_proj[i * hidden_size + j] = 0.125f;
        }
    }
    for (int i = 0; i < hidden_size; i++) {
        for (int j = 0; j < q_out; j++) {
            if (i == j) layer->o_proj[i * q_out + j] = 0.5f;
        }
    }
    for (int i = 0; i < moe_intermediate_size; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) layer->mlp_gate_proj[i * hidden_size + j] = 0.5f;
            if (i == j) layer->mlp_up_proj[i * hidden_size + j] = 0.25f;
        }
    }
    for (int i = 0; i < hidden_size; i++) {
        for (int j = 0; j < moe_intermediate_size; j++) {
            if (i == j) layer->mlp_down_proj[i * moe_intermediate_size + j] = 0.75f;
        }
    }
    for (int i = 0; i < shared_expert_intermediate_size; i++) {
        for (int j = 0; j < hidden_size; j++) {
            if (i == j) layer->sh_gate[i * hidden_size + j] = 0.25f;
            if (i == j) layer->sh_up[i * hidden_size + j] = 0.1f;
        }
    }
    for (int i = 0; i < hidden_size; i++) {
        for (int j = 0; j < shared_expert_intermediate_size; j++) {
            if (i == j) layer->sh_down[i * shared_expert_intermediate_size + j] = 0.2f;
        }
    }
    layer->expert_gate_proj = (float **)calloc((size_t)num_experts, sizeof(float *));
    layer->expert_up_proj = (float **)calloc((size_t)num_experts, sizeof(float *));
    layer->expert_down_proj = (float **)calloc((size_t)num_experts, sizeof(float *));
    if (!layer->expert_gate_proj || !layer->expert_up_proj || !layer->expert_down_proj) fail("out of memory");
    for (int expert = 0; expert < num_experts; expert++) {
        layer->expert_gate_proj[expert] = (float *)calloc((size_t)moe_intermediate_size * (size_t)hidden_size, sizeof(float));
        layer->expert_up_proj[expert] = (float *)calloc((size_t)moe_intermediate_size * (size_t)hidden_size, sizeof(float));
        layer->expert_down_proj[expert] = (float *)calloc((size_t)hidden_size * (size_t)moe_intermediate_size, sizeof(float));
        if (!layer->expert_gate_proj[expert] || !layer->expert_up_proj[expert] || !layer->expert_down_proj[expert]) fail("out of memory");
    }
}

static void init_model(qwen35_model *m, const char *snap_dir) {
    memset(m, 0, sizeof(*m));
    m->snap_dir = strdup(snap_dir);
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
    m->kv_cache_k_slots = (float ***)calloc((size_t)m->num_layers, sizeof(*m->kv_cache_k_slots));
    m->kv_cache_v_slots = (float ***)calloc((size_t)m->num_layers, sizeof(*m->kv_cache_v_slots));
    m->kv_cache_lens = (int **)calloc((size_t)m->num_layers, sizeof(*m->kv_cache_lens));
    m->kv_cache_caps = (int **)calloc((size_t)m->num_layers, sizeof(*m->kv_cache_caps));
    if (!m->kv_cache_k_slots || !m->kv_cache_v_slots || !m->kv_cache_lens || !m->kv_cache_caps) fail("out of memory");
    for (int layer = 0; layer < m->num_layers; layer++) {
        m->kv_cache_k_slots[layer] = (float **)calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_k_slots[layer]));
        m->kv_cache_v_slots[layer] = (float **)calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_v_slots[layer]));
        m->kv_cache_lens[layer] = (int *)calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_lens[layer]));
        m->kv_cache_caps[layer] = (int *)calloc((size_t)m->kv_slots, sizeof(*m->kv_cache_caps[layer]));
        if (!m->kv_cache_k_slots[layer] || !m->kv_cache_v_slots[layer] || !m->kv_cache_lens[layer] || !m->kv_cache_caps[layer]) fail("out of memory");
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
        m->final_norm = (float *)calloc((size_t)m->hidden_size, sizeof(float));
        if (!m->final_norm) fail("out of memory");
        for (int i = 0; i < m->hidden_size; i++) m->final_norm[i] = 1.0f;
    }

    m->lm_head = load_optional_tensor_f32(m, "lm_head.weight", (size_t)m->vocab_size * (size_t)m->hidden_size);
    if (!m->lm_head) {
        m->lm_head = (float *)malloc((size_t)m->vocab_size * (size_t)m->hidden_size * sizeof(float));
        if (!m->lm_head) fail("out of memory");
        memcpy(m->lm_head, m->embed, (size_t)m->vocab_size * (size_t)m->hidden_size * sizeof(float));
    }

    m->layers = (QLayer *)calloc((size_t)m->num_layers, sizeof(*m->layers));
    m->layer_types = (int *)calloc((size_t)m->num_layers, sizeof(*m->layer_types));
    if (!m->layers || !m->layer_types) fail("out of memory");

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
            cur->in_ln = (float *)calloc((size_t)m->hidden_size, sizeof(float));
            if (!cur->in_ln) fail("out of memory");
            for (int i = 0; i < m->hidden_size; i++) cur->in_ln[i] = 1.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.post_attention_layernorm.weight", layer);
        cur->post_ln = load_optional_tensor_f32(m, name, (size_t)m->hidden_size);
        if (!cur->post_ln) {
            cur->post_ln = (float *)calloc((size_t)m->hidden_size, sizeof(float));
            if (!cur->post_ln) fail("out of memory");
            for (int i = 0; i < m->hidden_size; i++) cur->post_ln[i] = 1.0f;
        }
        if (cur->is_full_attn) {
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.q_norm.weight", layer);
            cur->q_norm = load_optional_tensor_f32(m, name, (size_t)m->head_dim);
            if (!cur->q_norm) {
                cur->q_norm = (float *)calloc((size_t)m->head_dim, sizeof(float));
                if (!cur->q_norm) fail("out of memory");
                for (int i = 0; i < m->head_dim; i++) cur->q_norm[i] = 1.0f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.k_norm.weight", layer);
            cur->k_norm = load_optional_tensor_f32(m, name, (size_t)m->head_dim);
            if (!cur->k_norm) {
                cur->k_norm = (float *)calloc((size_t)m->head_dim, sizeof(float));
                if (!cur->k_norm) fail("out of memory");
                for (int i = 0; i < m->head_dim; i++) cur->k_norm[i] = 1.0f;
            }
            const int q_out = m->num_attention_heads * m->head_dim;
            const int kv_out = m->num_kv_heads * m->head_dim;
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.qkv.weight", layer);
            if (!load_packed_qkv_tensor(m, name, q_out, kv_out, m->hidden_size, &cur->q_proj, &cur->k_proj, &cur->v_proj)) {
                snprintf(name, sizeof(name), "model.layers.%d.self_attn.q_proj.weight", layer);
                cur->q_proj = load_optional_tensor_f32(m, name, (size_t)q_out * (size_t)m->hidden_size);
                if (!cur->q_proj) {
                    cur->q_proj = (float *)calloc((size_t)q_out * (size_t)m->hidden_size, sizeof(float));
                    if (!cur->q_proj) fail("out of memory");
                    for (int i = 0; i < q_out; i++) for (int j = 0; j < m->hidden_size; j++) if (i == j) cur->q_proj[i * m->hidden_size + j] = 1.0f;
                }
                snprintf(name, sizeof(name), "model.layers.%d.self_attn.k_proj.weight", layer);
                cur->k_proj = load_optional_tensor_f32(m, name, (size_t)kv_out * (size_t)m->hidden_size);
                if (!cur->k_proj) {
                    cur->k_proj = (float *)calloc((size_t)kv_out * (size_t)m->hidden_size, sizeof(float));
                    if (!cur->k_proj) fail("out of memory");
                    for (int i = 0; i < kv_out; i++) for (int j = 0; j < m->hidden_size; j++) if (i == j) cur->k_proj[i * m->hidden_size + j] = 0.25f;
                }
                snprintf(name, sizeof(name), "model.layers.%d.self_attn.v_proj.weight", layer);
                cur->v_proj = load_optional_tensor_f32(m, name, (size_t)kv_out * (size_t)m->hidden_size);
                if (!cur->v_proj) {
                    cur->v_proj = (float *)calloc((size_t)kv_out * (size_t)m->hidden_size, sizeof(float));
                    if (!cur->v_proj) fail("out of memory");
                    for (int i = 0; i < kv_out; i++) for (int j = 0; j < m->hidden_size; j++) if (i == j) cur->v_proj[i * m->hidden_size + j] = 0.125f;
                }
            }
            snprintf(name, sizeof(name), "model.layers.%d.self_attn.o_proj.weight", layer);
            cur->o_proj = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)q_out);
            if (!cur->o_proj) {
                cur->o_proj = (float *)calloc((size_t)m->hidden_size * (size_t)q_out, sizeof(float));
                if (!cur->o_proj) fail("out of memory");
                for (int i = 0; i < m->hidden_size; i++) for (int j = 0; j < q_out; j++) if (i == j) cur->o_proj[i * q_out + j] = 0.5f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.mlp.gate.weight", layer);
            cur->router = load_optional_tensor_f32(m, name, (size_t)m->num_experts * (size_t)m->hidden_size);
            if (cur->router) m->has_router = true;
            snprintf(name, sizeof(name), "model.layers.%d.mlp.gate_proj.weight", layer);
            cur->mlp_gate_proj = load_optional_tensor_f32(m, name, (size_t)m->moe_intermediate_size * (size_t)m->hidden_size);
            if (!cur->mlp_gate_proj) {
                cur->mlp_gate_proj = (float *)calloc((size_t)m->moe_intermediate_size * (size_t)m->hidden_size, sizeof(float));
                if (!cur->mlp_gate_proj) fail("out of memory");
                for (int i = 0; i < m->moe_intermediate_size; i++) for (int j = 0; j < m->hidden_size; j++) if (i == j) cur->mlp_gate_proj[i * m->hidden_size + j] = 0.5f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.mlp.up_proj.weight", layer);
            cur->mlp_up_proj = load_optional_tensor_f32(m, name, (size_t)m->moe_intermediate_size * (size_t)m->hidden_size);
            if (!cur->mlp_up_proj) {
                cur->mlp_up_proj = (float *)calloc((size_t)m->moe_intermediate_size * (size_t)m->hidden_size, sizeof(float));
                if (!cur->mlp_up_proj) fail("out of memory");
                for (int i = 0; i < m->moe_intermediate_size; i++) for (int j = 0; j < m->hidden_size; j++) if (i == j) cur->mlp_up_proj[i * m->hidden_size + j] = 0.25f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.mlp.down_proj.weight", layer);
            cur->mlp_down_proj = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->moe_intermediate_size);
            if (!cur->mlp_down_proj) {
                cur->mlp_down_proj = (float *)calloc((size_t)m->hidden_size * (size_t)m->moe_intermediate_size, sizeof(float));
                if (!cur->mlp_down_proj) fail("out of memory");
                for (int i = 0; i < m->hidden_size; i++) for (int j = 0; j < m->moe_intermediate_size; j++) if (i == j) cur->mlp_down_proj[i * m->moe_intermediate_size + j] = 0.75f;
            }
            cur->expert_gate_proj = (float **)calloc((size_t)m->num_experts, sizeof(float *));
            cur->expert_up_proj = (float **)calloc((size_t)m->num_experts, sizeof(float *));
            cur->expert_down_proj = (float **)calloc((size_t)m->num_experts, sizeof(float *));
            if (!cur->expert_gate_proj || !cur->expert_up_proj || !cur->expert_down_proj) fail("out of memory");
            for (int expert = 0; expert < m->num_experts; expert++) {
                char expert_name[1024];
                snprintf(expert_name, sizeof(expert_name), "model.layers.%d.mlp.experts.%d.gate_proj.weight", layer, expert);
                cur->expert_gate_proj[expert] = load_optional_tensor_f32(m, expert_name, (size_t)m->moe_intermediate_size * (size_t)m->hidden_size);
                if (!cur->expert_gate_proj[expert]) {
                    cur->expert_gate_proj[expert] = (float *)calloc((size_t)m->moe_intermediate_size * (size_t)m->hidden_size, sizeof(float));
                    if (!cur->expert_gate_proj[expert]) fail("out of memory");
                }
                snprintf(expert_name, sizeof(expert_name), "model.layers.%d.mlp.experts.%d.up_proj.weight", layer, expert);
                cur->expert_up_proj[expert] = load_optional_tensor_f32(m, expert_name, (size_t)m->moe_intermediate_size * (size_t)m->hidden_size);
                if (!cur->expert_up_proj[expert]) {
                    cur->expert_up_proj[expert] = (float *)calloc((size_t)m->moe_intermediate_size * (size_t)m->hidden_size, sizeof(float));
                    if (!cur->expert_up_proj[expert]) fail("out of memory");
                }
                snprintf(expert_name, sizeof(expert_name), "model.layers.%d.mlp.experts.%d.down_proj.weight", layer, expert);
                cur->expert_down_proj[expert] = load_optional_tensor_f32(m, expert_name, (size_t)m->hidden_size * (size_t)m->moe_intermediate_size);
                if (!cur->expert_down_proj[expert]) {
                    cur->expert_down_proj[expert] = (float *)calloc((size_t)m->hidden_size * (size_t)m->moe_intermediate_size, sizeof(float));
                    if (!cur->expert_down_proj[expert]) fail("out of memory");
                }
            }
        } else {
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_a.weight", layer);
            cur->la_in_proj_a = load_optional_tensor_f32(m, name, (size_t)m->linear_num_key_heads * (size_t)m->hidden_size);
            if (!cur->la_in_proj_a) {
                cur->la_in_proj_a = (float *)calloc((size_t)m->linear_num_key_heads * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_a) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_b.weight", layer);
            cur->la_in_proj_b = load_optional_tensor_f32(m, name, (size_t)m->linear_num_key_heads * (size_t)m->hidden_size);
            if (!cur->la_in_proj_b) {
                cur->la_in_proj_b = (float *)calloc((size_t)m->linear_num_key_heads * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_b) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_qkv.weight", layer);
            cur->la_in_proj_qkv = load_optional_tensor_f32(m, name, (size_t)3 * (size_t)m->hidden_size * (size_t)m->hidden_size);
            if (!cur->la_in_proj_qkv) {
                cur->la_in_proj_qkv = (float *)calloc((size_t)3 * (size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_qkv) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_z.weight", layer);
            cur->la_in_proj_z = load_optional_tensor_f32(m, name, (size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim * (size_t)m->hidden_size);
            if (!cur->la_in_proj_z) {
                cur->la_in_proj_z = (float *)calloc((size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_z) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.out_proj.weight", layer);
            cur->la_out_proj = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim);
            if (!cur->la_out_proj) {
                cur->la_out_proj = (float *)calloc((size_t)m->hidden_size * (size_t)m->linear_num_value_heads * (size_t)m->linear_value_head_dim, sizeof(float));
                if (!cur->la_out_proj) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.norm.weight", layer);
            cur->la_norm = load_optional_tensor_f32(m, name, (size_t)m->linear_key_head_dim);
            if (!cur->la_norm) {
                cur->la_norm = (float *)calloc((size_t)m->linear_key_head_dim, sizeof(float));
                if (!cur->la_norm) fail("out of memory");
                for (int i = 0; i < m->linear_key_head_dim; i++) cur->la_norm[i] = 1.0f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.A_log.weight", layer);
            cur->la_A_log = load_optional_tensor_f32(m, name, (size_t)m->linear_num_value_heads);
            if (!cur->la_A_log) {
                cur->la_A_log = (float *)calloc((size_t)m->linear_num_value_heads, sizeof(float));
                if (!cur->la_A_log) fail("out of memory");
                for (int i = 0; i < m->linear_num_value_heads; i++) cur->la_A_log[i] = 0.1f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.dt_bias.weight", layer);
            cur->la_dt_bias = load_optional_tensor_f32(m, name, (size_t)m->linear_num_value_heads);
            if (!cur->la_dt_bias) {
                cur->la_dt_bias = (float *)calloc((size_t)m->linear_num_value_heads, sizeof(float));
                if (!cur->la_dt_bias) fail("out of memory");
                for (int i = 0; i < m->linear_num_value_heads; i++) cur->la_dt_bias[i] = 0.01f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.conv1d.weight", layer);
            cur->la_conv1d = load_optional_tensor_f32(m, name, (size_t)3 * (size_t)m->hidden_size * 1 * 4);
            if (!cur->la_conv1d) {
                cur->la_conv1d = (float *)calloc((size_t)3 * (size_t)m->hidden_size * 1 * 4, sizeof(float));
                if (!cur->la_conv1d) fail("out of memory");
            }
        }
        /* shared expert is present on ALL layer types (full and linear attention). */
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert.gate_proj.weight", layer);
        cur->sh_gate = load_optional_tensor_f32(m, name, (size_t)m->shared_expert_intermediate_size * (size_t)m->hidden_size);
        if (!cur->sh_gate) {
            cur->sh_gate = (float *)calloc((size_t)m->shared_expert_intermediate_size * (size_t)m->hidden_size, sizeof(float));
            if (!cur->sh_gate) fail("out of memory");
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) for (int j = 0; j < m->hidden_size; j++) if (i == j) cur->sh_gate[i * m->hidden_size + j] = 0.25f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert.up_proj.weight", layer);
        cur->sh_up = load_optional_tensor_f32(m, name, (size_t)m->shared_expert_intermediate_size * (size_t)m->hidden_size);
        if (!cur->sh_up) {
            cur->sh_up = (float *)calloc((size_t)m->shared_expert_intermediate_size * (size_t)m->hidden_size, sizeof(float));
            if (!cur->sh_up) fail("out of memory");
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) for (int j = 0; j < m->hidden_size; j++) if (i == j) cur->sh_up[i * m->hidden_size + j] = 0.1f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert.down_proj.weight", layer);
        cur->sh_down = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->shared_expert_intermediate_size);
        if (!cur->sh_down) {
            cur->sh_down = (float *)calloc((size_t)m->hidden_size * (size_t)m->shared_expert_intermediate_size, sizeof(float));
            if (!cur->sh_down) fail("out of memory");
            for (int i = 0; i < m->hidden_size; i++) for (int j = 0; j < m->shared_expert_intermediate_size; j++) if (i == j) cur->sh_down[i * m->shared_expert_intermediate_size + j] = 0.2f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.mlp.shared_expert_gate.weight", layer);
        cur->shared_expert_gate = load_optional_tensor_f32(m, name, (size_t)m->hidden_size);
        if (!cur->shared_expert_gate) {
            cur->shared_expert_gate = (float *)calloc((size_t)m->hidden_size, sizeof(float));
            if (!cur->shared_expert_gate) fail("out of memory");
        }
    }
    free(arena);
}

static void free_layer(qwen35_model *m, QLayer *layer) {
    free(layer->in_ln);
    free(layer->post_ln);
    free(layer->q_norm);
    free(layer->k_norm);
    free(layer->q_proj);
    free(layer->k_proj);
    free(layer->v_proj);
    free(layer->o_proj);
    free(layer->router);
    free(layer->sh_gate);
    free(layer->sh_up);
    free(layer->sh_down);
    free(layer->shared_expert_gate);
    free(layer->mlp_gate_proj);
    free(layer->mlp_up_proj);
    free(layer->mlp_down_proj);
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
            free(layer->expert_gate_proj[expert]);
        }
        free(layer->expert_gate_proj);
    }
    if (layer->expert_up_proj) {
        for (int expert = 0; expert < m->num_experts; expert++) {
            free(layer->expert_up_proj[expert]);
        }
        free(layer->expert_up_proj);
    }
    if (layer->expert_down_proj) {
        for (int expert = 0; expert < m->num_experts; expert++) {
            free(layer->expert_down_proj[expert]);
        }
        free(layer->expert_down_proj);
    }
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
    int *indices = (int *)malloc((size_t)n * sizeof(int));
    if (!indices) fail("out of memory");
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

static void sample_logits(const float *logits, int n, float temperature, float top_p, int top_k, int *out_token) {
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
    int best = 0;
    indices = (int *)malloc((size_t)n * sizeof(int));
    values = (float *)malloc((size_t)n * sizeof(float));
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
    kept = (int *)malloc((size_t)keep * sizeof(int));
    keep_scores = (float *)malloc((size_t)keep * sizeof(float));
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
    probs = (float *)malloc((size_t)keep * sizeof(float));
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
        float *new_k = (float *)realloc(m->kv_cache_k_slots[layer][slot], bytes);
        float *new_v = (float *)realloc(m->kv_cache_v_slots[layer][slot], bytes);
        if (!new_k || !new_v) fail("out of memory");
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
                      int *out_tokens, float temperature, float top_p, int top_k, unsigned int seed,
                      int cache_slot) {
    float *hidden = (float *)malloc((size_t)m->hidden_size * sizeof(float));
    if (!hidden) fail("out of memory");
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
            float *residual = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!residual) fail("out of memory");
            memcpy(residual, hidden, (size_t)m->hidden_size * sizeof(float));

            if (cur->is_full_attn) {
                const int q_dim = m->num_attention_heads * m->head_dim;
                const int kv_dim = m->num_kv_heads * m->head_dim;
                float *q_out = (float *)malloc((size_t)q_dim * sizeof(float));
                float *k_out = (float *)malloc((size_t)kv_dim * sizeof(float));
                float *v_out = (float *)malloc((size_t)kv_dim * sizeof(float));
                float *attn = (float *)malloc((size_t)q_dim * sizeof(float));
                float *post = (float *)malloc((size_t)m->hidden_size * sizeof(float));
                if (!q_out || !k_out || !v_out || !attn || !post) fail("out of memory");
                matmul_vec(hidden, cur->q_proj, m->hidden_size, q_dim, q_out);
                matmul_vec(hidden, cur->k_proj, m->hidden_size, kv_dim, k_out);
                matmul_vec(hidden, cur->v_proj, m->hidden_size, kv_dim, v_out);
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
                    float *scores = (float *)calloc((size_t)cur->kv_cache_len, sizeof(float));
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
                matmul_vec(attn, cur->o_proj, q_dim, m->hidden_size, post);
                layer_norm_inplace(hidden, cur->in_ln, m->hidden_size);
                for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + post[i];
                free(q_out);
                free(k_out);
                free(v_out);
                free(attn);
                free(post);
            } else {
                float *la_out = (float *)malloc((size_t)m->hidden_size * sizeof(float));
                float *la_gate = (float *)malloc((size_t)m->hidden_size * sizeof(float));
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

            float *ffn_in = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *ffn_mid = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *ffn_out = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!ffn_in || !ffn_mid || !ffn_out) fail("out of memory");
            if (cur->is_full_attn) {
                matmul_vec(hidden, cur->mlp_gate_proj, m->hidden_size, m->moe_intermediate_size, ffn_in);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_in[i] = silu(ffn_in[i]);
                matmul_vec(hidden, cur->mlp_up_proj, m->hidden_size, m->moe_intermediate_size, ffn_mid);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_mid[i] = silu(ffn_mid[i]);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_in[i] *= ffn_mid[i];
                matmul_vec(ffn_in, cur->mlp_down_proj, m->moe_intermediate_size, m->hidden_size, ffn_out);
            } else {
                for (int i = 0; i < m->hidden_size; i++) ffn_out[i] = 0.0f;
            }

            if (cur->is_full_attn && m->has_router && cur->router) {
                float *router_logits = (float *)malloc((size_t)m->num_experts * sizeof(float));
                if (!router_logits) fail("out of memory");
                matmul_vec(hidden, cur->router, m->hidden_size, m->num_experts, router_logits);
                softmax(router_logits, m->num_experts);
                int *expert_indices = (int *)malloc((size_t)m->experts_per_tok * sizeof(int));
                float *expert_weights = (float *)malloc((size_t)m->experts_per_tok * sizeof(float));
                if (!expert_indices || !expert_weights) fail("out of memory");
                topk_select(router_logits, m->num_experts, m->experts_per_tok, expert_indices, expert_weights);
                for (int expert_slot = 0; expert_slot < m->experts_per_tok; expert_slot++) {
                    int expert_idx = expert_indices[expert_slot];
                    float weight = expert_weights[expert_slot];
                    if (weight <= 0.0f || !cur->expert_gate_proj[expert_idx]) continue;
                    float *expert_gate = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
                    float *expert_up = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
                    float *expert_res = (float *)malloc((size_t)m->hidden_size * sizeof(float));
                    if (!expert_gate || !expert_up || !expert_res) fail("out of memory");
                    matmul_vec(hidden, cur->expert_gate_proj[expert_idx], m->hidden_size, m->moe_intermediate_size, expert_gate);
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_gate[i] = silu(expert_gate[i]);
                    matmul_vec(hidden, cur->expert_up_proj[expert_idx], m->hidden_size, m->moe_intermediate_size, expert_up);
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_up[i] = silu(expert_up[i]);
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_gate[i] *= expert_up[i];
                    matmul_vec(expert_gate, cur->expert_down_proj[expert_idx], m->moe_intermediate_size, m->hidden_size, expert_res);
                    for (int i = 0; i < m->hidden_size; i++) ffn_out[i] += weight * expert_res[i];
                    free(expert_gate);
                    free(expert_up);
                    free(expert_res);
                }
                free(router_logits);
                free(expert_indices);
                free(expert_weights);
            }

            float *shared_in = (float *)malloc((size_t)m->shared_expert_intermediate_size * sizeof(float));
            float *shared_mid = (float *)malloc((size_t)m->shared_expert_intermediate_size * sizeof(float));
            float *shared_out = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!shared_in || !shared_mid || !shared_out) fail("out of memory");
            /* shared expert gate: gate_scalar = sigmoid(shared_expert_gate . hidden) */
            float gate_scalar = 1.0f;
            if (cur->shared_expert_gate) {
                float dot = 0.0f;
                for (int i = 0; i < m->hidden_size; i++) dot += cur->shared_expert_gate[i] * hidden[i];
                gate_scalar = 1.0f / (1.0f + expf(-dot));
            }
            matmul_vec(hidden, cur->sh_gate, m->hidden_size, m->shared_expert_intermediate_size, shared_in);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_in[i] = silu(shared_in[i]);
            matmul_vec(hidden, cur->sh_up, m->hidden_size, m->shared_expert_intermediate_size, shared_mid);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_mid[i] = silu(shared_mid[i]);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_in[i] *= shared_mid[i];
            matmul_vec(shared_in, cur->sh_down, m->shared_expert_intermediate_size, m->hidden_size, shared_out);
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
        float *logits = (float *)calloc((size_t)m->vocab_size, sizeof(float));
        if (!logits) fail("out of memory");
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
            sample_logits(logits, m->vocab_size, temperature, top_p, top_k, &best);
        }
        out_tokens[step] = best;
        free(logits);
    }
    free(hidden);
}

static int parse_token_ids(const char *text, int *out, int max_ids) {
    if (!text || !*text) return 0;
    char *copy = strdup(text);
    if (!copy) fail("out of memory");
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
    fprintf(stderr, "usage: %s [--model DIR] [--prompt TEXT] [--steps N] [--threads N] [--temperature F] [--top-k N] [--top-p F] [--seed N]\n", prog);
}

static void run_server(qwen35_model *model, int max_tokens, float temperature, float top_p, int top_k, unsigned int seed) {
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
        int top_k_request = top_k;
        unsigned int seed_request = seed;
        char *cursor = header + 9;
        int cache_slot_request = 0;
        if (sscanf(cursor, "%zu %d %f %f %d %d %u", &payload_len, &max_tokens_request,
                   &temperature_request, &top_p_request, &cache_slot_request, &top_k_request,
                   &seed_request) != 7) {
            fprintf(stderr, "[qwen35_moe] malformed prompt header: %s\n", header);
            continue;
        }
        if (payload_len > 0) {
            unsigned char *payload = (unsigned char *)malloc(payload_len + 1);
            if (!payload) fail("out of memory");
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
            int *tokens = (int *)calloc((size_t)max_tokens_request, sizeof(int));
            int *out = (int *)calloc((size_t)max_tokens_request, sizeof(int));
            if (!tokens || !out) {
                free(tokens);
                free(out);
                free(payload);
                fail("out of memory");
            }
            int n_tokens = parse_token_ids(prompt_text, tokens, max_tokens_request);
            if (n_tokens <= 0) n_tokens = 1;
            run_model(model, tokens, n_tokens, max_tokens_request, out, temperature_request, top_p_request, top_k_request, seed, cache_slot_request);
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
    int top_k = 0;
    unsigned int seed = 0;
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
        } else if (!strcmp(argv[i], "--seed") && i + 1 < argc) {
            seed = (unsigned int)strtoul(argv[++i], NULL, 10);
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
    if (temperature > 0.0f && seed == 0) srand((unsigned int)time(NULL));
    configure_parallelism(threads);
    qwen35_model model;
    init_model(&model, snap_dir);
    if (getenv("SERVE")) {
        run_server(&model, steps, temperature, top_p, top_k, seed);
        free_model(&model);
        return 0;
    }
    int *tokens = (int *)calloc((size_t)steps, sizeof(int));
    if (!tokens) fail("out of memory");
    int n_tokens = parse_token_ids(prompt ? prompt : "0", tokens, steps);
    if (n_tokens <= 0) n_tokens = 1;
    int *out = (int *)calloc((size_t)steps, sizeof(int));
    if (!out) fail("out of memory");
    run_model(&model, tokens, n_tokens, steps, out, temperature, top_p, top_k, seed, 0);
    for (int i = 0; i < steps; i++) {
        printf("%d\n", out[i]);
    }
    free(tokens);
    free(out);
    free_model(&model);
    return 0;
}
