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

#ifdef _OPENMP
#include <omp.h>
#endif
#if defined(COLI_VULKAN)
#include "backend_vulkan.h"
#elif defined(COLI_ROCM)
#include "backend_rocm.h"
#elif defined(COLI_CUDA)
#include "backend_cuda.h"
#endif

#include "json.h"
#include "st.h"

typedef struct {
    int vocab_size;
    int hidden_size;
    int num_layers;
    int num_experts;
    int experts_per_tok;
    bool has_router;
    char *snap_dir;
    shards shards;
    float *embed;
    float *final_norm;
    float *lm_head;
    float **attn_norm;
    float **ffn_norm;
    float **q_proj;
    float **k_proj;
    float **v_proj;
    float **o_proj;
    float **gate_proj;
    float **up_proj;
    float **down_proj;
    float **router;
    float ***expert_gate_proj;
    float ***expert_up_proj;
    float ***expert_down_proj;
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

static float *load_tensor_f32(qwen35_model *m, const char *name, size_t nelems) {
    float *buf = (float *)calloc(nelems, sizeof(float));
    if (!buf) fail("out of memory");
    st_tensor *t = st_find(&m->shards, name);
    if (!t) {
        return buf;
    }
    if (t->numel != (int64_t)nelems) {
        fail("tensor %s has %lld elements (expected %zu)", name, (long long)t->numel, nelems);
    }
    st_read_f32(&m->shards, name, buf, 0);
    return buf;
}

static float *load_optional_tensor_f32(qwen35_model *m, const char *name, size_t nelems) {
    if (!st_has(&m->shards, name)) return NULL;
    return load_tensor_f32(m, name, nelems);
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
    m->vocab_size = parse_int_field(cfg, "vocab_size", 32);
    m->hidden_size = parse_int_field(cfg, "hidden_size", 16);
    m->num_layers = parse_int_field(cfg, "num_hidden_layers", 1);
    m->num_experts = parse_int_field(cfg, "num_experts", 2);
    m->experts_per_tok = parse_int_field(cfg, "num_experts_per_tok", 1);
    m->has_router = parse_bool_field(cfg, "has_moe_router") || m->num_experts > 0;
    if (m->vocab_size <= 0 || m->hidden_size <= 0 || m->num_layers <= 0) {
        fail("invalid qwen config: vocab/hidden/layer sizes must be positive");
    }
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

    m->attn_norm = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->ffn_norm = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->q_proj = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->k_proj = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->v_proj = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->o_proj = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->gate_proj = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->up_proj = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->down_proj = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->router = (float **)calloc((size_t)m->num_layers, sizeof(float *));
    m->expert_gate_proj = (float ***)calloc((size_t)m->num_layers, sizeof(float **));
    m->expert_up_proj = (float ***)calloc((size_t)m->num_layers, sizeof(float **));
    m->expert_down_proj = (float ***)calloc((size_t)m->num_layers, sizeof(float **));
    if (!m->attn_norm || !m->ffn_norm || !m->q_proj || !m->k_proj || !m->v_proj || !m->o_proj || !m->gate_proj || !m->up_proj || !m->down_proj || !m->router || !m->expert_gate_proj || !m->expert_up_proj || !m->expert_down_proj) fail("out of memory");

    for (int layer = 0; layer < m->num_layers; layer++) {
        char name[1024];
        snprintf(name, sizeof(name), "model.layers.%d.input_layernorm.weight", layer);
        m->attn_norm[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size);
        if (!m->attn_norm[layer]) {
            m->attn_norm[layer] = (float *)calloc((size_t)m->hidden_size, sizeof(float));
            if (!m->attn_norm[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size; i++) m->attn_norm[layer][i] = 1.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.post_attention_layernorm.weight", layer);
        m->ffn_norm[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size);
        if (!m->ffn_norm[layer]) {
            m->ffn_norm[layer] = (float *)calloc((size_t)m->hidden_size, sizeof(float));
            if (!m->ffn_norm[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size; i++) m->ffn_norm[layer][i] = 1.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.self_attn.q_proj.weight", layer);
        m->q_proj[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
        if (!m->q_proj[layer]) {
            m->q_proj[layer] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            if (!m->q_proj[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->q_proj[layer][i] = (i % (m->hidden_size + 1)) == 0 ? 1.0f : 0.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.self_attn.k_proj.weight", layer);
        m->k_proj[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
        if (!m->k_proj[layer]) {
            m->k_proj[layer] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            if (!m->k_proj[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->k_proj[layer][i] = (i % (m->hidden_size + 2)) == 0 ? 0.25f : 0.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.self_attn.v_proj.weight", layer);
        m->v_proj[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
        if (!m->v_proj[layer]) {
            m->v_proj[layer] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            if (!m->v_proj[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->v_proj[layer][i] = (i % (m->hidden_size + 3)) == 0 ? 0.125f : 0.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.self_attn.o_proj.weight", layer);
        m->o_proj[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
        if (!m->o_proj[layer]) {
            m->o_proj[layer] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            if (!m->o_proj[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->o_proj[layer][i] = (i % (m->hidden_size + 4)) == 0 ? 0.5f : 0.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.mlp.gate_proj.weight", layer);
        m->gate_proj[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
        if (!m->gate_proj[layer]) {
            m->gate_proj[layer] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            if (!m->gate_proj[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->gate_proj[layer][i] = (i % (m->hidden_size + 5)) == 0 ? 0.5f : 0.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.mlp.up_proj.weight", layer);
        m->up_proj[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
        if (!m->up_proj[layer]) {
            m->up_proj[layer] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            if (!m->up_proj[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->up_proj[layer][i] = (i % (m->hidden_size + 6)) == 0 ? 0.25f : 0.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.mlp.down_proj.weight", layer);
        m->down_proj[layer] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
        if (!m->down_proj[layer]) {
            m->down_proj[layer] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
            if (!m->down_proj[layer]) fail("out of memory");
            for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->down_proj[layer][i] = (i % (m->hidden_size + 7)) == 0 ? 0.75f : 0.0f;
        }
        snprintf(name, sizeof(name), "model.layers.%d.mlp.router.weight", layer);
        m->router[layer] = load_optional_tensor_f32(m, name, (size_t)m->num_experts * (size_t)m->hidden_size);
        if (m->router[layer]) {
            m->has_router = true;
        }
        if (m->has_router && m->router[layer]) {
            m->expert_gate_proj[layer] = (float **)calloc((size_t)m->num_experts, sizeof(float *));
            m->expert_up_proj[layer] = (float **)calloc((size_t)m->num_experts, sizeof(float *));
            m->expert_down_proj[layer] = (float **)calloc((size_t)m->num_experts, sizeof(float *));
            if (!m->expert_gate_proj[layer] || !m->expert_up_proj[layer] || !m->expert_down_proj[layer]) fail("out of memory");
            for (int expert = 0; expert < m->num_experts; expert++) {
                snprintf(name, sizeof(name), "model.layers.%d.mlp.experts.%d.gate_proj.weight", layer, expert);
                m->expert_gate_proj[layer][expert] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
                if (!m->expert_gate_proj[layer][expert]) {
                    m->expert_gate_proj[layer][expert] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
                    if (!m->expert_gate_proj[layer][expert]) fail("out of memory");
                    for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->expert_gate_proj[layer][expert][i] = 0.0f;
                }
                snprintf(name, sizeof(name), "model.layers.%d.mlp.experts.%d.up_proj.weight", layer, expert);
                m->expert_up_proj[layer][expert] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
                if (!m->expert_up_proj[layer][expert]) {
                    m->expert_up_proj[layer][expert] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
                    if (!m->expert_up_proj[layer][expert]) fail("out of memory");
                    for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->expert_up_proj[layer][expert][i] = 0.0f;
                }
                snprintf(name, sizeof(name), "model.layers.%d.mlp.experts.%d.down_proj.weight", layer, expert);
                m->expert_down_proj[layer][expert] = load_optional_tensor_f32(m, name, (size_t)m->hidden_size * (size_t)m->hidden_size);
                if (!m->expert_down_proj[layer][expert]) {
                    m->expert_down_proj[layer][expert] = (float *)calloc((size_t)m->hidden_size * (size_t)m->hidden_size, sizeof(float));
                    if (!m->expert_down_proj[layer][expert]) fail("out of memory");
                    for (int i = 0; i < m->hidden_size * m->hidden_size; i++) m->expert_down_proj[layer][expert][i] = 0.0f;
                }
            }
        }
    }
    free(arena);
}

static void free_model(qwen35_model *m) {
    free(m->snap_dir);
    free(m->embed);
    free(m->final_norm);
    free(m->lm_head);
    for (int i = 0; i < m->num_layers; i++) {
        free(m->attn_norm[i]);
        free(m->ffn_norm[i]);
        free(m->q_proj[i]);
        free(m->k_proj[i]);
        free(m->v_proj[i]);
        free(m->o_proj[i]);
        free(m->gate_proj[i]);
        free(m->up_proj[i]);
        free(m->down_proj[i]);
        free(m->router[i]);
        if (m->expert_gate_proj[i]) {
            for (int expert = 0; expert < m->num_experts; expert++) {
                free(m->expert_gate_proj[i][expert]);
                free(m->expert_up_proj[i][expert]);
                free(m->expert_down_proj[i][expert]);
            }
            free(m->expert_gate_proj[i]);
            free(m->expert_up_proj[i]);
            free(m->expert_down_proj[i]);
        }
    }
    free(m->attn_norm);
    free(m->ffn_norm);
    free(m->q_proj);
    free(m->k_proj);
    free(m->v_proj);
    free(m->o_proj);
    free(m->gate_proj);
    free(m->up_proj);
    free(m->down_proj);
    free(m->router);
    free(m->expert_gate_proj);
    free(m->expert_up_proj);
    free(m->expert_down_proj);
}

static void matmul_vec(const float *x, const float *w, int in_dim, int out_dim, float *out) {
    if (in_dim <= 0 || out_dim <= 0) {
        fprintf(stderr, "warning: invalid matmul dimensions (%d, %d)\n", in_dim, out_dim);
        return;
    }
    if ((size_t)out_dim > SIZE_MAX / (size_t)in_dim) {
        fprintf(stderr, "warning: overflow in matmul row offset for dimensions (%d, %d)\n", in_dim, out_dim);
        return;
    }
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

static void softmax(float *values, int n) {
    float maxv = values[0];
    for (int i = 1; i < n; i++) if (values[i] > maxv) maxv = values[i];
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        values[i] = expf(values[i] - maxv);
        sum += values[i];
    }
    for (int i = 0; i < n; i++) values[i] /= sum;
}

static void configure_parallelism(void) {
#ifdef _OPENMP
    const char *threads_env = getenv("COLI_THREADS");
    if (!threads_env || !*threads_env) return;
    char *end = NULL;
    errno = 0;
    long requested = strtol(threads_env, &end, 10);
    if (errno != 0) {
        fprintf(stderr, "warning: ignoring invalid COLI_THREADS=%s (invalid number or out of range)\n", threads_env);
        return;
    }
    if (end == threads_env || *end != '\0') {
        fprintf(stderr, "warning: ignoring invalid COLI_THREADS=%s (not a whole number)\n", threads_env);
        return;
    }
    if (requested <= 0 || requested > INT_MAX) {
        fprintf(stderr, "warning: ignoring invalid COLI_THREADS=%s (invalid number or out of range)\n", threads_env);
        return;
    }
    omp_set_num_threads((int)requested);
#endif
}

static void run_model(qwen35_model *m, const int *tokens, int n_tokens, int steps, int *out_tokens) {
    float *hidden = (float *)malloc((size_t)m->hidden_size * sizeof(float));
    if (!hidden) fail("out of memory");
    for (int step = 0; step < steps; step++) {
        int token_id = step < n_tokens ? tokens[step] : (step > 0 ? out_tokens[step - 1] : tokens[0]);
        for (int i = 0; i < m->hidden_size; i++) hidden[i] = m->embed[token_id * m->hidden_size + i];
        for (int layer = 0; layer < m->num_layers; layer++) {
            float *residual = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!residual) fail("out of memory");
            memcpy(residual, hidden, (size_t)m->hidden_size * sizeof(float));
            float *attn = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!attn) fail("out of memory");
            matmul_vec(hidden, m->q_proj[layer], m->hidden_size, m->hidden_size, attn);
            float *mixed = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!mixed) fail("out of memory");
            matmul_vec(hidden, m->k_proj[layer], m->hidden_size, m->hidden_size, mixed);
            for (int i = 0; i < m->hidden_size; i++) attn[i] += mixed[i];
            float *value = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!value) fail("out of memory");
            matmul_vec(hidden, m->v_proj[layer], m->hidden_size, m->hidden_size, value);
            for (int i = 0; i < m->hidden_size; i++) attn[i] += value[i];
            float *post = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!post) fail("out of memory");
            matmul_vec(attn, m->o_proj[layer], m->hidden_size, m->hidden_size, post);
            layer_norm_inplace(hidden, m->attn_norm[layer], m->hidden_size);
            for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + post[i];

            float *ffn_in = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!ffn_in) fail("out of memory");
            matmul_vec(hidden, m->gate_proj[layer], m->hidden_size, m->hidden_size, ffn_in);
            for (int i = 0; i < m->hidden_size; i++) ffn_in[i] = relu(ffn_in[i]);
            float *ffn_mid = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!ffn_mid) fail("out of memory");
            matmul_vec(hidden, m->up_proj[layer], m->hidden_size, m->hidden_size, ffn_mid);
            for (int i = 0; i < m->hidden_size; i++) ffn_mid[i] = relu(ffn_mid[i]);
            float *ffn_out = (float *)malloc((size_t)m->hidden_size * sizeof(float));
            if (!ffn_out) fail("out of memory");
            for (int i = 0; i < m->hidden_size; i++) ffn_in[i] *= ffn_mid[i];
            matmul_vec(ffn_in, m->down_proj[layer], m->hidden_size, m->hidden_size, ffn_out);
            if (m->has_router && m->router[layer]) {
                float *router_logits = (float *)malloc((size_t)m->num_experts * sizeof(float));
                if (!router_logits) fail("out of memory");
                matmul_vec(hidden, m->router[layer], m->hidden_size, m->num_experts, router_logits);
                softmax(router_logits, m->num_experts);
                float *expert_out = (float *)malloc((size_t)m->hidden_size * sizeof(float));
                if (!expert_out) fail("out of memory");
                for (int expert = 0; expert < m->num_experts; expert++) {
                    float gate = router_logits[expert];
                    if (gate <= 0.0f) continue;
                    float *expert_ffn = (float *)malloc((size_t)m->hidden_size * sizeof(float));
                    if (!expert_ffn) fail("out of memory");
                    matmul_vec(hidden, m->expert_gate_proj[layer][expert], m->hidden_size, m->hidden_size, expert_ffn); 
                    for (int i = 0; i < m->hidden_size; i++) expert_ffn[i] = relu(expert_ffn[i]);
                    float *expert_mid = (float *)calloc((size_t)m->hidden_size, sizeof(float));
                    if (!expert_mid) fail("out of memory");
                    matmul_vec(hidden, m->expert_up_proj[layer][expert], m->hidden_size, m->hidden_size, expert_mid);
                    for (int i = 0; i < m->hidden_size; i++) expert_mid[i] = relu(expert_mid[i]);
                    float *expert_res = (float *)calloc((size_t)m->hidden_size, sizeof(float));
                    if (!expert_res) fail("out of memory");
                    matmul_vec(expert_mid, m->expert_down_proj[layer][expert], m->hidden_size, m->hidden_size, expert_res);
                    for (int i = 0; i < m->hidden_size; i++) expert_out[i] += gate * expert_res[i];
                    free(expert_ffn);
                    free(expert_mid);
                    free(expert_res);
                }
                for (int i = 0; i < m->hidden_size; i++) ffn_out[i] += expert_out[i];
                free(router_logits);
                free(expert_out);
            }
            layer_norm_inplace(hidden, m->ffn_norm[layer], m->hidden_size);
            for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + ffn_out[i];
            free(residual);
            free(attn);
            free(mixed);
            free(value);
            free(post);
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
    fprintf(stderr, "usage: %s [--model DIR] [--prompt TEXT] [--steps N]\n", prog);
}

int main(int argc, char **argv) {
    const char *snap_dir = NULL;
    const char *prompt = NULL;
    int steps = 4;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--model") && i + 1 < argc) {
            snap_dir = argv[++i];
        } else if (!strcmp(argv[i], "--prompt") && i + 1 < argc) {
            prompt = argv[++i];
        } else if (!strcmp(argv[i], "--steps") && i + 1 < argc) {
            steps = atoi(argv[++i]);
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
    configure_parallelism();
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
