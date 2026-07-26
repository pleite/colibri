# Architecture notes

Three independent INT8 matmul backends for AMD Strix Halo. Each targets one
piece of silicon in that SoC. None of them falls back to another.

## CPU backend — AVX-512 VNNI

Row-major int8 matmul; each output element is the dot product of an input row
and a weight row, accumulated with `_mm512_dpbusd_epi32`. Because that
instruction takes an *unsigned* first operand, the kernel makes the weights
unsigned with `_mm512_abs_epi8` and negates the corresponding activation lanes
with a mask taken from the weight sign bits — two extra operations per vector.

It is a correctness reference, not a tuned kernel: one horizontal reduction per
*output element*, no register blocking, no weight packing and no threading. The
alternative sign handling (bias the activations into unsigned range and subtract
a per-weight-row `128·Σb` precomputed at pack time) removes those two ops, and
is one of the things Plan A3 exists to measure.

There is no scalar path. On a host without AVX-512 VNNI the backend reports
`avx512-vnni-unavailable` and the tests skip.

## GPU backend — headless Vulkan compute on GFX1151

A real compute pipeline, not a wrapper: `gpu/comp.comp` is compiled to
`gpu/comp.spv` and dispatched over three std430 storage buffers (A, B
transposed, C) with `{rows, cols, inner}` push constants and a 16×16 local size.

**It is not an int8 engine, and its dataflow is not zero-copy.** The shader is
f32, so `run_matmul()` widens both int8 operands to f32 on the host, transposes
B on the host on *every* call even though the weights are constant across
tokens, and `memcpy`s C back out of the mapped output buffer. All three buffers
are created and freed per call. The *allocation* is on the unified-memory fast
path; the *dataflow* is not, and any GPU int8 throughput number taken from this
path is measuring host-side format conversion as much as the shader.

Load-bearing properties:

* All Vulkan types come from `<vulkan/vulkan_core.h>`. `gpu/vulkan_dispatch.h`
  contains function-pointer tables built from `PFN_vk*` typedefs and declares no
  Vulkan structs of its own. The previous hand-written ABI header is what caused
  the long-standing segfault — see `docs/vulkan_debug_attempts.md` in the
  repository root.
* `dlsym` resolves `vkGetInstanceProcAddr` and nothing else. Everything else
  comes from `vkGetInstanceProcAddr` / `vkGetDeviceProcAddr`, per the Khronos
  Loader Interface.
* Zero instance extensions are requested, which is what makes it headless.
* Device selection requires vendor `0x1002` and `INTEGRATED_GPU`. There is no
  lavapipe path and no discrete-GPU path.
* Memory is allocated from a `DEVICE_LOCAL | HOST_VISIBLE | HOST_COHERENT` heap
  where available — the Strix Halo unified-memory fast path, with no staging
  copy. A `SHADER_WRITE → HOST_READ` pipeline barrier still precedes host
  readback, as the specification requires. Note that "no staging copy" is a
  property of the allocation only: the host widening, transpose and readback
  `memcpy` above are copies all the same.
* The `VkInstance` / `VkDevice` pair is created once and cached for the process.

## NPU backend — XDNA 2

Talks to the in-tree `amdxdna` kernel driver through DRM ioctls on
`/dev/accel/accel0`; XRT is not required. All ioctl structures come from
`<drm/amdxdna_accel.h>`; if the header is missing, the build fails rather than
using a vendored copy.

AIE-2 executes fixed-shape kernels only, so a matmul is dispatched only when a
`.npukernel` artifact matching (rows, inner_dim, out_cols, INT8) exactly has been
loaded. Otherwise `xdna2_matmul_int8()` returns `-ENOENT`. There is no CPU
fallback: a fallback would make NPU measurements meaningless and hide missing
kernels.

AIE-2 has no int4 datapath, so int4 weights are widened by
`xdna2_dequant_int4()` before an INT8 kernel runs.

Full detail: `docs/strix-halo-npu.md` in the repository root.
