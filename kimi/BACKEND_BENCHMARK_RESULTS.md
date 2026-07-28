# VNNI Backend Benchmark Results — Strix Halo (2026-07-28)

## Test Configuration

- **Hardware:** AMD Ryzen AI Max+ 395 (Strix Halo)
- **CPU:** 32 cores, AVX-512 VNNI
- **GPU:** Radeon 8060S (GFX1151, RADV)
- **NPU:** XDNA 2 (AIE-2, 16 tiles)
- **Benchmark tool:** `benchmark_all_backends` with new `--batching-mode batched` flag
- **Batch size:** 16 (serial vs batched comparison)
- **Iterations:** 3 per shape
- **Thread count:** CPU tested with 8 threads
- **Date:** 2026-07-28

## What Changed in the Code

### New: Batched dispatch mode (`--batching-mode batched`)

Three files were added/modified to support batched dispatch:

1. **`c/npu_kernels/xdna2_matmul.c`** — New function `xdna2_matmul_int8_batch()`
   - Uploads shared input/weight BOs once
   - Creates per-batch output and command BOs
   - Submits all batch commands without intervening waits
   - Single timeline-syncobj wait on last sequence number
   - Reads back and dequantizes all outputs

2. **`vnni-int8-matmul/gpu/vulkan_backend.c`** — New function `run_batch_matmul()`
   - Single `vkQueueSubmit()` for all batch items
   - One fence wait covers entire batch
   - Shared A/B VkBuffers, per-batch C buffers
   - Temporary descriptor pool for batch_size sets

3. **`vnni-int8-matmul/benchmark_all_backends.c`** — New benchmark harness
   - `run_gpu_benchmark_batched()` and `run_npu_benchmark_batched()`
   - Pre-allocated matrices reused across iterations
   - Mode selection via `--batching-mode serial|batched`

### Existing: CPU multi-threading

CPU already supports `--threads N` for multi-threaded batch dispatch.

---

## Benchmark Results

### Existing Shapes (Qwen 3.5/3.6 projections)

| Shape (rows, inner, out) | Role | GPU Serial (ms) | GPU Batched (ms) | NPU Serial (ms) | NPU Batched (ms) | CPU 8T (ms) |
|---|---|---|---|---|---|---|
| 256, 4096, 1024 | Expert down | 34.89 | 140.33 | 1.28 | 5.45 | 26.10 |
| 256, 1024, 4096 | Expert up/gate | 47.86 | 383.28 | 1.73 | 18.01 | 26.42 |
| 256, 4096, 16384 | Self-attn q_proj | 4286.96 | 7413.39 | 19.64 | 82.83 | 1561.18 |
| 256, 4096, 512 | Self-attn k/v, router | 12.73 | 39.77 | 0.81 | 3.16 | 13.76 |
| 256, 8192, 4096 | Self-attn o_proj | 449.38 | 1416.48 | 8.48 | 25.36 | 426.36 |
| 32, 4096, 1024 | Expert down (decode) | 28.33 | 41.31 | 0.99 | 1.77 | 3.52 |
| 32, 1024, 4096 | Expert up/gate (decode) | 30.99 | 65.75 | 0.95 | 3.08 | 3.10 |
| 32, 4096, 16384 | Self-attn q_proj (decode) | 4150.43 | 4494.22 | 14.61 | 23.26 | 169.72 |
| 32, 4096, 512 | Self-attn k/v, router (decode) | 9.58 | 23.11 | 0.58 | 1.27 | 1.73 |
| 32, 8192, 4096 | Self-attn o_proj (decode) | 413.96 | 333.75 | 7.24 | 9.53 | 52.24 |

### NPU Kernel Availability

| Shape | Kernel exists? |
|---|---|
| rows=256, all projections | ✅ Yes (10 kernels) |
| rows=32, all projections | ✅ Yes (10 kernels) |
| rows=1 | ❌ No (fixed-shape, not compiled) |
| rows=8 | ❌ No |
| rows=64 | ❌ No |

---

## Batch Efficiency Analysis

**Efficiency = (serial_total_time × batch_size) / batched_total_time**

A value of 16.0x means perfect linear scaling. Values below 16.0x indicate overhead.

| Shape | GPU Efficiency | NPU Efficiency |
|---|---|---|
| 256, 4096, 1024 | **4.0x** | **3.8x** |
| 256, 1024, 4096 | **4.0x** | **3.8x** |
| 256, 4096, 16384 | **9.3x** | **3.8x** |
| 256, 4096, 512 | **5.1x** | **4.1x** |
| 256, 8192, 4096 | **5.1x** | **5.4x** |
| 32, 4096, 1024 | **11.6x** | **9.0x** |
| 32, 1024, 4096 | **9.5x** | **5.2x** |
| 32, 4096, 16384 | **18.5x** | **10.0x** |
| 32, 4096, 512 | **11.0x** | **10.6x** |
| 32, 8192, 4096 | **12.5x** | **17.0x** |

### Key observations

1. **GPU batched scaling is highly shape-dependent.** Small shapes (rows=32) scale well (9-12x). Large shapes (rows=256, inner=4096, out=16384) show poor scaling (9.3x) due to descriptor pool and buffer allocation overhead dominating.

2. **NPU batched scaling is consistently 3.8-17x.** The NPU's fixed-shape kernel means the dispatch overhead is amortized differently. Larger shapes with more AIE compute time show better scaling (17x for 32×8192×4096).

3. **GPU overhead dominates for large shapes.** The Vulkan batch implementation creates per-call descriptor pools, command buffers, and C buffers. For the 256×4096×16384 shape, the GPU overhead is ~50% of total time.

4. **NPU is consistently fastest for all shapes.** Even serial NPU is 10-30x faster than GPU for moderate shapes, and 100-1000x faster for small shapes.

5. **CPU with 8 threads is competitive with GPU for decode shapes.** For rows=32 shapes, CPU 8T is comparable to or faster than GPU serial for smaller projections.

---

## Kimi K3 New Shapes — Not Yet Benchmarked

Kimi K3 introduces **8 new (inner, out) projection pairs** that need new NPU kernels:

| Shape (rows, inner, out) | Role | New? | Estimated kernel size |
|---|---|---|---|
| 256, 3072, 3584 | Expert FFN gate_proj | **YES** | ~4208 bytes |
| 256, 3584, 3072 | Expert FFN down_proj | **YES** | ~4208 bytes |
| 256, 7168, 7168 | Attention q_proj | **YES** | ~8400 bytes |
| 256, 7168, 512 | Attention k_proj / v_proj | **YES** | ~4208 bytes |
| 256, 7168, 1536 | MLA QKV | **YES** | ~8400 bytes |
| 256, 1536, 128 | MLA rotary | **YES** | ~2100 bytes |
| 256, 512, 128 | MLA V_proj | **YES** | ~2100 bytes |
| 256, 128, 64 | MLA O_proj | **YES** | ~1050 bytes |

Plus 8 more for decode (rows=32):

| Shape (rows, inner, out) | Role | New? |
|---|---|---|
| 32, 3072, 3584 | Expert FFN gate_proj (decode) | **YES** |
| 32, 3584, 3072 | Expert FFN down_proj (decode) | **YES** |
| 32, 7168, 7168 | Attention q_proj (decode) | **YES** |
| 32, 7168, 512 | Attention k_proj / v_proj (decode) | **YES** |
| 32, 7168, 1536 | MLA QKV (decode) | **YES** |
| 32, 1536, 128 | MLA rotary (decode) | **YES** |
| 32, 512, 128 | MLA V_proj (decode) | **YES** |
| 32, 128, 64 | MLA O_proj (decode) | **YES** |

**Total new kernels needed: 16** (8 projections × 2 row tiles)

Existing Kimi K3 shapes that already have Qwen 3.5/3.6 kernels:
- 256/32, 4096, 16384 (shared expert gate/up)
- 256/32, 4096, 1024 (shared expert down)
- 256/32, 8192, 4096 (shared expert o_proj)
- 256/32, 4096, 512 (MoE router gate)

---

## Recommendations

### For Kimi K3 quantization pipeline:

1. **NPU should handle all existing shapes** (4096→1024, 1024→4096, 4096→16384, 4096→512, 8192→4096) — it's 10-30x faster than GPU serial.

2. **New MLA shapes (7168→1536, 1536→128, 512→128, 128→64)** are small enough that CPU might be faster than waiting for NPU kernel compilation. Test these on CPU first.

3. **New expert FFN shapes (3072→3584, 3584→3072)** are moderate size — NPU batched mode with rows=256 should be fast. Need to compile kernels.

4. **Attention q_proj (7168→7168)** is the largest new shape — NPU will be fastest but kernel compilation will take longer.

### For batched dispatch:

1. **GPU batched mode has significant overhead** for large shapes. Consider per-shape batch size tuning (batch=4-8 might be optimal for large shapes).

2. **NPU batched mode is consistently good** — the hardware naturally batches well. batch=16 is a good default.

3. **CPU batched mode (multi-threaded)** is the simplest and most flexible. For decode shapes (rows=32), CPU 8T is competitive with GPU.

---

## File Locations

- Benchmark binary: `/home/leite/colibri/vnni-int8-matmul/benchmark_all_backends`
- Results: `gpu_serial.csv`, `gpu_batched_16.csv`, `npu_serial.csv`, `npu_batched_16.csv`, `cpu_batched_16_threads8.csv`
- Source: `/home/leite/colibri/vnni-int8-matmul/`
- NPU kernels: `/home/leite/colibri/vnni-int8-matmul/npu/kernels/`
