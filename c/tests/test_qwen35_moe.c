#include <errno.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    const char *name;
    float *data;
    size_t n;
} tensor;

typedef struct {
    const char *name;
    const void *data;
    size_t nelems;
    size_t nbytes;
    const char *dtype;
} raw_tensor;

typedef struct {
    int vocab_size;
    int hidden_size;
    int num_layers;
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
} toy_model;

static void fail(const char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    va_end(ap);
    fputc('\n', stderr);
    exit(1);
}

static void *xmalloc(size_t n) {
    void *p = calloc(1, n);
    if (!p) fail("out of memory");
    return p;
}

static void write_safetensors_file(const char *path, const tensor *tensors, size_t count) {
    size_t data_offset = 0;
    size_t header_capacity = 4096;
    char *header_buf = (char *)malloc(header_capacity);
    if (!header_buf) fail("out of memory");
    strcpy(header_buf, "{");
    size_t header_used = 1;
    for (size_t i = 0; i < count; i++) {
        char entry[2048];
        size_t nbytes = tensors[i].n * sizeof(float);
        int written = snprintf(entry, sizeof(entry), "%s\"%s\":{\"dtype\":\"F32\",\"shape\":[%zu],\"data_offsets\":[%zu,%zu]}",
                               i == 0 ? "" : ",", tensors[i].name, tensors[i].n, data_offset, data_offset + nbytes);
        if (written < 0 || (size_t)written >= sizeof(entry)) fail("header entry too large");
        if (header_used + (size_t)written + 1 >= header_capacity) {
            header_capacity *= 2;
            header_buf = (char *)realloc(header_buf, header_capacity);
            if (!header_buf) fail("out of memory");
        }
        memcpy(header_buf + header_used, entry, (size_t)written);
        header_used += (size_t)written;
        data_offset += nbytes;
    }
    header_buf[header_used++] = '}';
    header_buf[header_used] = '\0';

    FILE *fp = fopen(path, "wb");
    if (!fp) fail("cannot open %s", path);
    uint64_t header_size = header_used - 1;
    fwrite(&header_size, sizeof(header_size), 1, fp);
    fwrite(header_buf, 1, header_used - 1, fp);
    for (size_t i = 0; i < count; i++) {
        fwrite(tensors[i].data, sizeof(float), tensors[i].n, fp);
    }
    fclose(fp);
    free(header_buf);
}

static void write_safetensors_file_raw(const char *path, const raw_tensor *tensors, size_t count) {
    size_t data_offset = 0;
    size_t header_capacity = 4096;
    char *header_buf = (char *)malloc(header_capacity);
    if (!header_buf) fail("out of memory");
    strcpy(header_buf, "{");
    size_t header_used = 1;
    for (size_t i = 0; i < count; i++) {
        char entry[2048];
        int written = snprintf(entry, sizeof(entry), "%s\"%s\":{\"dtype\":\"%s\",\"shape\":[%zu],\"data_offsets\":[%zu,%zu]}",
                               i == 0 ? "" : ",", tensors[i].name, tensors[i].dtype, tensors[i].nelems, data_offset, data_offset + tensors[i].nbytes);
        if (written < 0 || (size_t)written >= sizeof(entry)) fail("header entry too large");
        if (header_used + (size_t)written + 1 >= header_capacity) {
            header_capacity *= 2;
            header_buf = (char *)realloc(header_buf, header_capacity);
            if (!header_buf) fail("out of memory");
        }
        memcpy(header_buf + header_used, entry, (size_t)written);
        header_used += (size_t)written;
        data_offset += tensors[i].nbytes;
    }
    header_buf[header_used++] = '}';
    header_buf[header_used] = '\0';

    FILE *fp = fopen(path, "wb");
    if (!fp) fail("cannot open %s", path);
    uint64_t header_size = header_used - 1;
    fwrite(&header_size, sizeof(header_size), 1, fp);
    fwrite(header_buf, 1, header_used - 1, fp);
    for (size_t i = 0; i < count; i++) {
        fwrite(tensors[i].data, 1, tensors[i].nbytes, fp);
    }
    fclose(fp);
    free(header_buf);
}

static void make_model_dir(const char *dir) {
    if (mkdir(dir, 0777) != 0 && errno != EEXIST) fail("mkdir %s", dir);
}

static void write_model(const char *dir) {
    make_model_dir(dir);
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/config.json", dir);
    FILE *fp = fopen(config_path, "w");
    if (!fp) fail("cannot write %s", config_path);
    fprintf(fp, "{\"vocab_size\":4,\"hidden_size\":4,\"num_hidden_layers\":1,\"num_experts\":2,\"num_experts_per_tok\":1}");
    fclose(fp);

    char tokenizer_path[1024];
    snprintf(tokenizer_path, sizeof(tokenizer_path), "%s/tokenizer.json", dir);
    fp = fopen(tokenizer_path, "w");
    if (!fp) fail("cannot write %s", tokenizer_path);
    fputs("{}\n", fp);
    fclose(fp);

    char shard_path[1024];
    snprintf(shard_path, sizeof(shard_path), "%s/model.safetensors", dir);

    float embed[16] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f};
    float norm[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float lmhead[16] = {0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f};
    float attn_norm[4] = {1.0f, 1.1f, 1.2f, 1.3f};
    float ffn_norm[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float q[16] = {0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f};
    float k[16] = {0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f};
    float v[16] = {0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f};
    float o[16] = {0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f};
    float gate[16] = {0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f};
    float up[16] = {0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f};
    float down[16] = {0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f};
    tensor tensors[] = {
        {"model.embed_tokens.weight", embed, 16},
        {"model.norm.weight", norm, 4},
        {"lm_head.weight", lmhead, 16},
        {"model.layers.0.input_layernorm.weight", attn_norm, 4},
        {"model.layers.0.post_attention_layernorm.weight", ffn_norm, 4},
        {"model.layers.0.self_attn.q_proj.weight", q, 16},
        {"model.layers.0.self_attn.k_proj.weight", k, 16},
        {"model.layers.0.self_attn.v_proj.weight", v, 16},
        {"model.layers.0.self_attn.o_proj.weight", o, 16},
        {"model.layers.0.mlp.gate_proj.weight", gate, 16},
        {"model.layers.0.mlp.up_proj.weight", up, 16},
        {"model.layers.0.mlp.down_proj.weight", down, 16},
    };
    write_safetensors_file(shard_path, tensors, sizeof(tensors) / sizeof(tensors[0]));
}

static void write_quantized_model(const char *dir) {
    make_model_dir(dir);
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/config.json", dir);
    FILE *fp = fopen(config_path, "w");
    if (!fp) fail("cannot write %s", config_path);
    fprintf(fp, "{\"vocab_size\":4,\"hidden_size\":4,\"num_hidden_layers\":1,\"num_experts\":2,\"num_experts_per_tok\":1}");
    fclose(fp);

    char tokenizer_path[1024];
    snprintf(tokenizer_path, sizeof(tokenizer_path), "%s/tokenizer.json", dir);
    fp = fopen(tokenizer_path, "w");
    if (!fp) fail("cannot write %s", tokenizer_path);
    fputs("{}\n", fp);
    fclose(fp);

    char shard_path[1024];
    snprintf(shard_path, sizeof(shard_path), "%s/model.safetensors", dir);

    const uint8_t embed_bytes[16] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16,
    };
    const float embed_scales[4] = {0.1f, 0.2f, 0.3f, 0.4f};
    /* Packed 4-bit values with two nibbles per byte; the bytes below encode
     * a small 4x4 matrix for a simple int4 smoke test. */
    const uint8_t q_proj_bytes[8] = {0x0b, 0x1c, 0x2d, 0x3e, 0x4f, 0x5a, 0x6b, 0x7c};
    const float q_proj_scales[4] = {0.25f, 0.5f, 0.75f, 1.0f};
    raw_tensor tensors[] = {
        {"model.embed_tokens.weight", embed_bytes, 16, sizeof(embed_bytes), "U8"},
        {"model.embed_tokens.weight.qs", embed_scales, 4, sizeof(embed_scales), "F32"},
        {"model.layers.0.self_attn.q_proj.weight", q_proj_bytes, 8, sizeof(q_proj_bytes), "U8"},
        {"model.layers.0.self_attn.q_proj.weight.qs", q_proj_scales, 4, sizeof(q_proj_scales), "F32"},
    };
    write_safetensors_file_raw(shard_path, tensors, sizeof(tensors) / sizeof(tensors[0]));
}

static void write_quantized_model_with_language_model_prefixes(const char *dir) {
    make_model_dir(dir);
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/config.json", dir);
    FILE *fp = fopen(config_path, "w");
    if (!fp) fail("cannot write %s", config_path);
    fprintf(fp, "{\"vocab_size\":4,\"hidden_size\":4,\"num_hidden_layers\":1,\"num_experts\":2,\"num_experts_per_tok\":1}");
    fclose(fp);

    char tokenizer_path[1024];
    snprintf(tokenizer_path, sizeof(tokenizer_path), "%s/tokenizer.json", dir);
    fp = fopen(tokenizer_path, "w");
    if (!fp) fail("cannot write %s", tokenizer_path);
    fputs("{}\n", fp);
    fclose(fp);

    char shard_path[1024];
    snprintf(shard_path, sizeof(shard_path), "%s/model.safetensors", dir);

    const uint8_t embed_bytes[16] = {
        1, 2, 3, 4,
        5, 6, 7, 8,
        9, 10, 11, 12,
        13, 14, 15, 16,
    };
    const float embed_scales[4] = {0.1f, 0.2f, 0.3f, 0.4f};
    const uint8_t q_proj_bytes[8] = {0x0b, 0x1c, 0x2d, 0x3e, 0x4f, 0x5a, 0x6b, 0x7c};
    const float q_proj_scales[4] = {0.25f, 0.5f, 0.75f, 1.0f};
    raw_tensor tensors[] = {
        {"model.language_model.embed_tokens.weight", embed_bytes, 16, sizeof(embed_bytes), "U8"},
        {"model.language_model.embed_tokens.weight.qs", embed_scales, 4, sizeof(embed_scales), "F32"},
        {"model.language_model.norm.weight", embed_scales, 4, sizeof(embed_scales), "F32"},
        {"model.language_model.layers.0.self_attn.q_proj.weight", q_proj_bytes, 8, sizeof(q_proj_bytes), "U8"},
        {"model.language_model.layers.0.self_attn.q_proj.weight.qs", q_proj_scales, 4, sizeof(q_proj_scales), "F32"},
    };
    write_safetensors_file_raw(shard_path, tensors, sizeof(tensors) / sizeof(tensors[0]));
}

static void write_model_with_split_index(const char *dir) {
    make_model_dir(dir);
    char config_path[1024];
    snprintf(config_path, sizeof(config_path), "%s/config.json", dir);
    FILE *fp = fopen(config_path, "w");
    if (!fp) fail("cannot write %s", config_path);
    fprintf(fp, "{\"vocab_size\":4,\"hidden_size\":4,\"num_hidden_layers\":1,\"num_experts\":2,\"num_experts_per_tok\":1}");
    fclose(fp);

    char tokenizer_path[1024];
    snprintf(tokenizer_path, sizeof(tokenizer_path), "%s/tokenizer.json", dir);
    fp = fopen(tokenizer_path, "w");
    if (!fp) fail("cannot write %s", tokenizer_path);
    fputs("{}\n", fp);
    fclose(fp);

    char shard_a[1024];
    char shard_b[1024];
    char index_path[1024];
    snprintf(shard_a, sizeof(shard_a), "%s/model-part-a.safetensors", dir);
    snprintf(shard_b, sizeof(shard_b), "%s/model-part-b.safetensors", dir);
    snprintf(index_path, sizeof(index_path), "%s/model.safetensors.index.json", dir);

    float embed[16] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f};
    float norm[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float lmhead[16] = {0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f};
    float attn_norm[4] = {1.0f, 1.1f, 1.2f, 1.3f};
    float ffn_norm[4] = {1.0f, 1.0f, 1.0f, 1.0f};
    float q[16] = {0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f};
    float k[16] = {0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f};
    float v[16] = {0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f};
    float o[16] = {0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f};
    float gate[16] = {0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f};
    float up[16] = {0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f};
    float down[16] = {0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f};
    tensor part_a[] = {
        {"model.embed_tokens.weight", embed, 16},
        {"model.norm.weight", norm, 4},
        {"lm_head.weight", lmhead, 16},
    };
    tensor part_b[] = {
        {"model.layers.0.input_layernorm.weight", attn_norm, 4},
        {"model.layers.0.post_attention_layernorm.weight", ffn_norm, 4},
        {"model.layers.0.self_attn.q_proj.weight", q, 16},
        {"model.layers.0.self_attn.k_proj.weight", k, 16},
        {"model.layers.0.self_attn.v_proj.weight", v, 16},
        {"model.layers.0.self_attn.o_proj.weight", o, 16},
        {"model.layers.0.mlp.gate_proj.weight", gate, 16},
        {"model.layers.0.mlp.up_proj.weight", up, 16},
        {"model.layers.0.mlp.down_proj.weight", down, 16},
    };
    write_safetensors_file(shard_a, part_a, sizeof(part_a) / sizeof(part_a[0]));
    write_safetensors_file(shard_b, part_b, sizeof(part_b) / sizeof(part_b[0]));

    fp = fopen(index_path, "w");
    if (!fp) fail("cannot write %s", index_path);
    fprintf(fp,
            "{\"weight_map\":{\"model.embed_tokens.weight\":\"model-part-a.safetensors\","
            "\"model.norm.weight\":\"model-part-a.safetensors\","
            "\"lm_head.weight\":\"model-part-a.safetensors\","
            "\"model.layers.0.input_layernorm.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.post_attention_layernorm.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.self_attn.q_proj.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.self_attn.k_proj.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.self_attn.v_proj.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.self_attn.o_proj.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.mlp.gate_proj.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.mlp.up_proj.weight\":\"model-part-b.safetensors\","
            "\"model.layers.0.mlp.down_proj.weight\":\"model-part-b.safetensors\"}}");
    fclose(fp);
}

static void matmul_vec(const float *x, const float *w, int in_dim, int out_dim, float *out) {
    for (int out_idx = 0; out_idx < out_dim; out_idx++) {
        float sum = 0.0f;
        for (int in_idx = 0; in_idx < in_dim; in_idx++) {
            sum += x[in_idx] * w[out_idx * in_dim + in_idx];
        }
        out[out_idx] = sum;
    }
}

static void layer_norm_inplace(float *x, const float *scale, int dim) {
    for (int i = 0; i < dim; i++) x[i] *= scale[i];
}

static float relu(float x) { return x > 0.0f ? x : 0.0f; }

static void run_reference(const toy_model *m, const int *tokens, int n_tokens, int steps, int *out_tokens) {
    float hidden[4] = {0};
    for (int step = 0; step < steps; step++) {
        int token_id = step < n_tokens ? tokens[step] : (step > 0 ? out_tokens[step - 1] : tokens[0]);
        for (int i = 0; i < m->hidden_size; i++) hidden[i] = m->embed[token_id * m->hidden_size + i];
        for (int layer = 0; layer < m->num_layers; layer++) {
            float residual[4];
            memcpy(residual, hidden, sizeof(residual));
            float attn[4] = {0};
            matmul_vec(hidden, m->q_proj[layer], m->hidden_size, m->hidden_size, attn);
            float mixed[4] = {0};
            matmul_vec(hidden, m->k_proj[layer], m->hidden_size, m->hidden_size, mixed);
            for (int i = 0; i < m->hidden_size; i++) attn[i] += mixed[i];
            float value[4] = {0};
            matmul_vec(hidden, m->v_proj[layer], m->hidden_size, m->hidden_size, value);
            for (int i = 0; i < m->hidden_size; i++) attn[i] += value[i];
            float post[4] = {0};
            matmul_vec(attn, m->o_proj[layer], m->hidden_size, m->hidden_size, post);
            layer_norm_inplace(hidden, m->attn_norm[layer], m->hidden_size);
            for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + post[i];
            float ffn_in[4] = {0};
            matmul_vec(hidden, m->gate_proj[layer], m->hidden_size, m->hidden_size, ffn_in);
            for (int i = 0; i < m->hidden_size; i++) ffn_in[i] = relu(ffn_in[i]);
            float ffn_mid[4] = {0};
            matmul_vec(hidden, m->up_proj[layer], m->hidden_size, m->hidden_size, ffn_mid);
            for (int i = 0; i < m->hidden_size; i++) ffn_mid[i] = relu(ffn_mid[i]);
            float ffn_out[4] = {0};
            for (int i = 0; i < m->hidden_size; i++) ffn_in[i] *= ffn_mid[i];
            matmul_vec(ffn_in, m->down_proj[layer], m->hidden_size, m->hidden_size, ffn_out);
            layer_norm_inplace(hidden, m->ffn_norm[layer], m->hidden_size);
            for (int i = 0; i < m->hidden_size; i++) hidden[i] += residual[i] + ffn_out[i];
        }
        layer_norm_inplace(hidden, m->final_norm, m->hidden_size);
        float logits[4] = {0};
        for (int vocab = 0; vocab < m->vocab_size; vocab++) {
            float sum = 0.0f;
            for (int i = 0; i < m->hidden_size; i++) sum += hidden[i] * m->lm_head[vocab * m->hidden_size + i];
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
    }
}

static void run_engine(const char *model_dir, int steps, const int *tokens, int n_tokens, int *out_tokens) {
    char cmd[2048];
    char prompt[128];
    prompt[0] = '\0';
    for (int i = 0; i < n_tokens; i++) {
        char part[32];
        snprintf(part, sizeof(part), "%s%d", i == 0 ? "" : ",", tokens[i]);
        strncat(prompt, part, sizeof(prompt) - strlen(prompt) - 1);
    }
    snprintf(cmd, sizeof(cmd), "./qwen35_moe --model %s --prompt %s --steps %d", model_dir, prompt, steps);
    FILE *pipe = popen(cmd, "r");
    if (!pipe) fail("popen failed");
    char line[64];
    int index = 0;
    while (fgets(line, sizeof(line), pipe) && index < steps) {
        out_tokens[index++] = atoi(line);
    }
    if (pclose(pipe) != 0) fail("engine exited with non-zero status");
}

int main(void) {
    char dir[] = "/tmp/colibri-qwen35-test-XXXXXX";
    char *tmp = mkdtemp(dir);
    if (!tmp) fail("mkdtemp failed");
    write_model(tmp);

    int tokens[4] = {0, 1, 2, 3};
    int expected[4] = {0, 0, 0, 0};
    int actual[4] = {0, 0, 0, 0};
    toy_model model;
    memset(&model, 0, sizeof(model));
    model.vocab_size = 4;
    model.hidden_size = 4;
    model.num_layers = 1;
    model.embed = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.embed, (float[]){0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f, 1.1f, 1.2f, 1.3f, 1.4f, 1.5f, 1.6f}, sizeof(float) * 16);
    model.final_norm = (float *)xmalloc(4 * sizeof(float));
    memcpy(model.final_norm, (float[]){1.0f, 1.0f, 1.0f, 1.0f}, sizeof(float) * 4);
    model.lm_head = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.lm_head, (float[]){0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f, 0.1f, 0.0f, 0.3f, 0.4f, 0.2f}, sizeof(float) * 16);
    model.attn_norm = (float **)xmalloc(sizeof(float *));
    model.attn_norm[0] = (float *)xmalloc(4 * sizeof(float));
    memcpy(model.attn_norm[0], (float[]){1.0f, 1.1f, 1.2f, 1.3f}, sizeof(float) * 4);
    model.ffn_norm = (float **)xmalloc(sizeof(float *));
    model.ffn_norm[0] = (float *)xmalloc(4 * sizeof(float));
    memcpy(model.ffn_norm[0], (float[]){1.0f, 1.0f, 1.0f, 1.0f}, sizeof(float) * 4);
    model.q_proj = (float **)xmalloc(sizeof(float *));
    model.q_proj[0] = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.q_proj[0], (float[]){0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f}, sizeof(float) * 16);
    model.k_proj = (float **)xmalloc(sizeof(float *));
    model.k_proj[0] = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.k_proj[0], (float[]){0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f, 0.0f, 0.0f, 0.0f, 0.0f, 0.05f}, sizeof(float) * 16);
    model.v_proj = (float **)xmalloc(sizeof(float *));
    model.v_proj[0] = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.v_proj[0], (float[]){0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f, 0.0f, 0.0f, 0.0f, 0.0f, 0.02f}, sizeof(float) * 16);
    model.o_proj = (float **)xmalloc(sizeof(float *));
    model.o_proj[0] = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.o_proj[0], (float[]){0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f, 0.0f, 0.0f, 0.0f, 0.0f, 0.3f}, sizeof(float) * 16);
    model.gate_proj = (float **)xmalloc(sizeof(float *));
    model.gate_proj[0] = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.gate_proj[0], (float[]){0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f, 0.0f, 0.0f, 0.0f, 0.0f, 0.1f}, sizeof(float) * 16);
    model.up_proj = (float **)xmalloc(sizeof(float *));
    model.up_proj[0] = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.up_proj[0], (float[]){0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f, 0.0f, 0.0f, 0.0f, 0.0f, 0.2f}, sizeof(float) * 16);
    model.down_proj = (float **)xmalloc(sizeof(float *));
    model.down_proj[0] = (float *)xmalloc(16 * sizeof(float));
    memcpy(model.down_proj[0], (float[]){0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f, 0.0f, 0.0f, 0.0f, 0.0f, 0.4f}, sizeof(float) * 16);
    run_reference(&model, tokens, 2, 4, expected);
    run_engine(tmp, 4, tokens, 2, actual);
    for (int i = 0; i < 4; i++) {
        if (expected[i] != actual[i]) fail("mismatch at token %d: expected %d got %d", i, expected[i], actual[i]);
    }

    char split_dir[] = "/tmp/colibri-qwen35-index-XXXXXX";
    char *split_tmp = mkdtemp(split_dir);
    if (!split_tmp) fail("mkdtemp failed");
    write_model_with_split_index(split_tmp);
    int split_actual[4] = {0, 0, 0, 0};
    run_engine(split_tmp, 4, tokens, 2, split_actual);
    for (int i = 0; i < 4; i++) {
        if (split_actual[i] != expected[i]) fail("split-index model mismatch at token %d: expected %d got %d", i, expected[i], split_actual[i]);
    }

    char quant_dir[] = "/tmp/colibri-qwen35-quant-XXXXXX";
    char *quant_tmp = mkdtemp(quant_dir);
    if (!quant_tmp) fail("mkdtemp failed");
    write_quantized_model(quant_tmp);
    int quant_actual[4] = {0, 0, 0, 0};
    run_engine(quant_tmp, 4, tokens, 2, quant_actual);
    for (int i = 0; i < 4; i++) {
        if (quant_actual[i] < 0) fail("quantized model produced invalid token %d", quant_actual[i]);
    }

    char prefix_dir[] = "/tmp/colibri-qwen35-prefix-XXXXXX";
    char *prefix_tmp = mkdtemp(prefix_dir);
    if (!prefix_tmp) fail("mkdtemp failed");
    write_quantized_model_with_language_model_prefixes(prefix_tmp);
    int prefix_actual[4] = {0, 0, 0, 0};
    run_engine(prefix_tmp, 4, tokens, 2, prefix_actual);
    for (int i = 0; i < 4; i++) {
        if (prefix_actual[i] < 0) fail("prefixed quantized model produced invalid token %d", prefix_actual[i]);
    }

    puts("qwen35_moe test: ok");
    return 0;
}
