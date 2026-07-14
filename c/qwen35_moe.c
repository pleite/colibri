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
#include <limits.h>
#include <math.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
    bool has_router;
    char *snap_dir;
    shards shards;
    float *embed;
    float *final_norm;
    float *lm_head;
    QLayer *layers;
    int *layer_types;
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

static int parse_int_field(jval *obj, const char *key, int fallback) {
    jval *value = json_get(obj, key);
    if (!value || value->t != J_NUM) return fallback;
    return (int)value->num;
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
    static const char *const old_prefix = "model.layers.";
    static const char *const new_prefix = "model.language_model.layers.";
    size_t old_len = strlen(old_prefix);
    size_t new_len = strlen(new_prefix);
    if (strncmp(name, old_prefix, old_len) == 0) {
        size_t tail_len = strlen(name + old_len);
        size_t total = new_len + tail_len + 1;
        char *candidate = (char *)malloc(total);
        if (!candidate) fail("out of memory");
        snprintf(candidate, total, "%s%s", new_prefix, name + old_len);
        t = st_find(&m->shards, candidate);
        free(candidate);
        return t;
    }
    if (strncmp(name, new_prefix, new_len) == 0) {
        size_t tail_len = strlen(name + new_len);
        size_t total = old_len + tail_len + 1;
        char *candidate = (char *)malloc(total);
        if (!candidate) fail("out of memory");
        snprintf(candidate, total, "%s%s", old_prefix, name + new_len);
        t = st_find(&m->shards, candidate);
        free(candidate);
        return t;
    }
    return NULL;
}

static int tensor_exists(qwen35_model *m, const char *name) {
    return find_tensor(m, name) != NULL;
}

static float *load_tensor_f32(qwen35_model *m, const char *name, size_t nelems) {
    float *buf = (float *)calloc(nelems, sizeof(float));
    if (!buf) fail("out of memory");
    st_tensor *t = find_tensor(m, name);
    if (!t) {
        return buf;
    }
    if (t->dtype == 3) {
        size_t scale_name_len = strlen(name) + 4;
        char *scale_name = (char *)malloc(scale_name_len);
        if (!scale_name) fail("out of memory");
        snprintf(scale_name, scale_name_len, "%s.qs", name);
        st_tensor *scale = find_tensor(m, scale_name);
        free(scale_name);
        if (!scale) {
            fprintf(stderr, "warning: missing scale for quantized tensor %s; assuming unit scale\n", name);
        }
        size_t out_dim = scale ? (size_t)scale->numel : 1;
        if (out_dim == 0 || nelems % out_dim != 0) {
            fail("tensor %s has incompatible shape for quantized load", name);
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
        fail("tensor %s has unsupported packed size %lld for %zu elements", name, (long long)t->nbytes, nelems);
    }
    if (t->numel != (int64_t)nelems) {
        fail("tensor %s has %lld elements (expected %zu)", name, (long long)t->numel, nelems);
    }
    st_read_f32(&m->shards, t->name, buf, 0);
    return buf;
}

static float *load_optional_tensor_f32(qwen35_model *m, const char *name, size_t nelems) {
    if (!tensor_exists(m, name)) return NULL;
    return load_tensor_f32(m, name, nelems);
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
    m->vocab_size = parse_int_field(cfg, "vocab_size", 32);
    m->hidden_size = parse_int_field(cfg, "hidden_size", 16);
    m->num_layers = parse_int_field(cfg, "num_hidden_layers", 1);
    m->num_experts = parse_int_field(cfg, "num_experts", 2);
    m->experts_per_tok = parse_int_field(cfg, "num_experts_per_tok", 1);
    m->moe_intermediate_size = parse_int_field(text_cfg, "moe_intermediate_size", m->hidden_size);
    m->shared_expert_intermediate_size = parse_int_field(text_cfg, "shared_expert_intermediate_size", m->moe_intermediate_size);
    m->num_attention_heads = parse_int_field(text_cfg, "num_attention_heads", 1);
    m->num_kv_heads = parse_int_field(text_cfg, "num_key_value_heads", 1);
    m->head_dim = parse_int_field(text_cfg, "head_dim", m->hidden_size);
    m->has_router = parse_bool_field(cfg, "has_moe_router") || m->num_experts > 0;
    if (m->vocab_size <= 0 || m->hidden_size <= 0 || m->num_layers <= 0) {
        fail("invalid qwen config: vocab/hidden/layer sizes must be positive");
    }
    if (m->num_attention_heads <= 0) m->num_attention_heads = 1;
    if (m->num_kv_heads <= 0) m->num_kv_heads = 1;
    if (m->head_dim <= 0) m->head_dim = m->hidden_size;
    if (m->moe_intermediate_size <= 0) m->moe_intermediate_size = m->hidden_size;
    if (m->shared_expert_intermediate_size <= 0) m->shared_expert_intermediate_size = m->moe_intermediate_size;
    st_init(&m->shards, snap_dir);

    m->embed = load_tensor_f32(m, "model.embed_tokens.weight", (size_t)m->vocab_size * (size_t)m->hidden_size);
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
                for (int i = 0; i < m->hidden_size; i++) cur->shared_expert_gate[i] = 1.0f;
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
            cur->la_in_proj_a = load_optional_tensor_f32(m, name, (size_t)128 * (size_t)m->hidden_size);
            if (!cur->la_in_proj_a) {
                cur->la_in_proj_a = (float *)calloc((size_t)128 * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_a) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_b.weight", layer);
            cur->la_in_proj_b = load_optional_tensor_f32(m, name, (size_t)128 * (size_t)m->hidden_size);
            if (!cur->la_in_proj_b) {
                cur->la_in_proj_b = (float *)calloc((size_t)128 * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_b) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_qkv.weight", layer);
            cur->la_in_proj_qkv = load_optional_tensor_f32(m, name, (size_t)12288 * (size_t)m->hidden_size);
            if (!cur->la_in_proj_qkv) {
                cur->la_in_proj_qkv = (float *)calloc((size_t)12288 * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_qkv) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.in_proj_z.weight", layer);
            cur->la_in_proj_z = load_optional_tensor_f32(m, name, (size_t)8192 * (size_t)m->hidden_size);
            if (!cur->la_in_proj_z) {
                cur->la_in_proj_z = (float *)calloc((size_t)8192 * (size_t)m->hidden_size, sizeof(float));
                if (!cur->la_in_proj_z) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.out_proj.weight", layer);
            cur->la_out_proj = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)8192);
            if (!cur->la_out_proj) {
                cur->la_out_proj = (float *)calloc((size_t)m->hidden_size * (size_t)8192, sizeof(float));
                if (!cur->la_out_proj) fail("out of memory");
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.norm.weight", layer);
            cur->la_norm = load_optional_tensor_f32(m, name, 128);
            if (!cur->la_norm) {
                cur->la_norm = (float *)calloc(128, sizeof(float));
                if (!cur->la_norm) fail("out of memory");
                for (int i = 0; i < 128; i++) cur->la_norm[i] = 1.0f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.A_log.weight", layer);
            cur->la_A_log = load_optional_tensor_f32(m, name, 128);
            if (!cur->la_A_log) {
                cur->la_A_log = (float *)calloc(128, sizeof(float));
                if (!cur->la_A_log) fail("out of memory");
                for (int i = 0; i < 128; i++) cur->la_A_log[i] = 0.1f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.dt_bias.weight", layer);
            cur->la_dt_bias = load_optional_tensor_f32(m, name, 128);
            if (!cur->la_dt_bias) {
                cur->la_dt_bias = (float *)calloc(128, sizeof(float));
                if (!cur->la_dt_bias) fail("out of memory");
                for (int i = 0; i < 128; i++) cur->la_dt_bias[i] = 0.01f;
            }
            snprintf(name, sizeof(name), "model.layers.%d.linear_attn.conv1d.weight", layer);
            cur->la_conv1d = load_optional_tensor_f32(m, name, (size_t)12288 * 1 * 4);
            if (!cur->la_conv1d) {
                cur->la_conv1d = (float *)calloc((size_t)12288 * 1 * 4, sizeof(float));
                if (!cur->la_conv1d) fail("out of memory");
            }
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

static void run_model(qwen35_model *m, const int *tokens, int n_tokens, int steps, int *out_tokens) {
    float *hidden = (float *)malloc((size_t)m->hidden_size * sizeof(float));
    if (!hidden) fail("out of memory");
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
                for (int i = 0; i < q_dim; i++) {
                    float kv_term = i < kv_dim ? k_out[i] : 0.0f;
                    float v_term = i < kv_dim ? v_out[i] : 0.0f;
                    attn[i] = q_out[i] + kv_term + v_term;
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
                if (!la_out) fail("out of memory");
                for (int i = 0; i < m->hidden_size; i++) la_out[i] = hidden[i] * 0.1f;
                layer_norm_inplace(hidden, cur->in_ln, m->hidden_size);
                for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + la_out[i];
                free(la_out);
            }

            float *ffn_in = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *ffn_mid = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *ffn_out = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!ffn_in || !ffn_mid || !ffn_out) fail("out of memory");
            if (cur->is_full_attn) {
                matmul_vec(hidden, cur->mlp_gate_proj, m->hidden_size, m->moe_intermediate_size, ffn_in);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_in[i] = relu(ffn_in[i]);
                matmul_vec(hidden, cur->mlp_up_proj, m->hidden_size, m->moe_intermediate_size, ffn_mid);
                for (int i = 0; i < m->moe_intermediate_size; i++) ffn_mid[i] = relu(ffn_mid[i]);
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
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_gate[i] = relu(expert_gate[i]);
                    matmul_vec(hidden, cur->expert_up_proj[expert_idx], m->hidden_size, m->moe_intermediate_size, expert_up);
                    for (int i = 0; i < m->moe_intermediate_size; i++) expert_up[i] = relu(expert_up[i]);
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

            float *shared_in = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *shared_mid = (float *)malloc((size_t)m->moe_intermediate_size * sizeof(float));
            float *shared_out = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!shared_in || !shared_mid || !shared_out) fail("out of memory");
            matmul_vec(hidden, cur->sh_gate, m->hidden_size, m->shared_expert_intermediate_size, shared_in);
            {
                const int gate_dim = m->shared_expert_intermediate_size < m->hidden_size ? m->shared_expert_intermediate_size : m->hidden_size;
                for (int i = 0; i < m->shared_expert_intermediate_size; i++) {
                    float gate = cur->shared_expert_gate && i < gate_dim ? cur->shared_expert_gate[i] : 1.0f;
                    shared_in[i] = silu(shared_in[i] * gate);
                }
            }
            matmul_vec(hidden, cur->sh_up, m->hidden_size, m->shared_expert_intermediate_size, shared_mid);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_mid[i] = silu(shared_mid[i]);
            for (int i = 0; i < m->shared_expert_intermediate_size; i++) shared_in[i] *= shared_mid[i];
            matmul_vec(shared_in, cur->sh_down, m->shared_expert_intermediate_size, m->hidden_size, shared_out);
            for (int i = 0; i < m->hidden_size; i++) ffn_out[i] += shared_out[i];
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
        float best_score = logits[0];
        for (int vocab = 1; vocab < m->vocab_size; vocab++) {
            if (logits[vocab] > best_score) {
                best_score = logits[vocab];
                best = vocab;
            }
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

static void usage(const char *prog) {
    fprintf(stderr, "usage: %s [--model DIR] [--prompt TEXT] [--steps N] [--threads N]\n", prog);
}

int main(int argc, char **argv) {
    const char *snap_dir = NULL;
    const char *prompt = NULL;
    int steps = 4;
    int threads = 0;
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
    configure_parallelism(threads);
    qwen35_model model;
    init_model(&model, snap_dir);
    int *tokens = (int *)calloc((size_t)steps, sizeof(int));
    if (!tokens) fail("out of memory");
    int n_tokens = parse_token_ids(prompt ? prompt : "0", tokens, steps);
    if (n_tokens <= 0) n_tokens = 1;
    int *out = (int *)calloc((size_t)steps, sizeof(int));
    if (!out) fail("out of memory");
    run_model(&model, tokens, n_tokens, steps, out);
    for (int i = 0; i < steps; i++) {
        printf("%d\n", out[i]);
    }
    free(tokens);
    free(out);
    free_model(&model);
    return 0;
}
