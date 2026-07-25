# Strix Halo INT8 matmul building blocks.
#
# Target: AMD Ryzen AI Max+ 395 "Strix Halo" only — AVX-512 VNNI CPU, RADV
# GFX1151 iGPU, XDNA 2 NPU. There are no portable or fallback build variants:
# these are inference building blocks for one machine.
#
# Build requirements:
#   * gcc with AVX-512 VNNI support
#   * Vulkan headers            (vulkan-headers / libvulkan-dev)
#   * Linux amdxdna UAPI header (<drm/amdxdna_accel.h>, kernel >= 6.14)
#   * glslangValidator          (only to regenerate gpu/comp.spv)

CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -std=c11
CPPFLAGS += -I. -I.. -D_GNU_SOURCE
LDFLAGS += -lm -ldl

# Strix Halo targets require AVX-512 VNNI support.
CPU_FLAGS ?= -mavx512f -mavx512vnni -mavx512bw -mavx512dq

GLSLANG ?= glslangValidator

NPU_DIR := ../c/npu_kernels

TESTS := tests/test_backends tests/vulkan_runtime_test tests/vulkan_debug_harness \
         tests/npu_device_test

all: $(TESTS)

# ── Shader ────────────────────────────────────────────────────────────────────
# gpu/comp.spv is committed so the backend works without glslang installed.
# Use `make shader` to regenerate it from gpu/comp.comp after editing the GLSL.
shader:
	$(GLSLANG) -V gpu/comp.comp -o gpu/comp.spv

# ── Objects ───────────────────────────────────────────────────────────────────

cpu/vnni_cpu_backend.o: cpu/vnni_cpu_backend.c cpu/vnni_cpu_backend.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CPU_FLAGS) -c $< -o $@

gpu/vulkan_backend.o: gpu/vulkan_backend.c gpu/vulkan_backend.h gpu/vulkan_dispatch.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

npu/xdna2_backend.o: npu/xdna2_backend.c npu/xdna2_backend.h $(NPU_DIR)/xdna2_matmul.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(NPU_DIR)/xdna2_driver.o: $(NPU_DIR)/xdna2_driver.c $(NPU_DIR)/xdna2_driver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(NPU_DIR)/xdna2_matmul.o: $(NPU_DIR)/xdna2_matmul.c $(NPU_DIR)/xdna2_matmul.h $(NPU_DIR)/xdna2_driver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

NPU_OBJS := npu/xdna2_backend.o $(NPU_DIR)/xdna2_driver.o $(NPU_DIR)/xdna2_matmul.o

tests/%.o: tests/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# ── Binaries ──────────────────────────────────────────────────────────────────

tests/test_backends: tests/test_backends.o cpu/vnni_cpu_backend.o gpu/vulkan_backend.o $(NPU_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/vulkan_runtime_test: tests/vulkan_runtime_test.o gpu/vulkan_backend.o cpu/vnni_cpu_backend.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/vulkan_debug_harness: tests/vulkan_debug_harness.o gpu/vulkan_backend.o cpu/vnni_cpu_backend.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/npu_device_test: tests/npu_device_test.o $(NPU_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ── Test / clean ──────────────────────────────────────────────────────────────

test: $(TESTS)
	./tests/test_backends
	./tests/vulkan_runtime_test
	./tests/vulkan_debug_harness
	./tests/npu_device_test

podman-test: scripts/strix-halo-podman-test.sh
	./scripts/strix-halo-podman-test.sh

clean:
	rm -f $(TESTS) tests/*.o cpu/*.o npu/*.o gpu/*.o $(NPU_DIR)/*.o test_output.log

.PHONY: all shader test podman-test clean
