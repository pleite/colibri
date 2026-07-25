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

# DRM_AMDXDNA_QUERY_RESOURCE_INFO landed in the amdxdna UAPI after Linux 6.18.
# Probe for it instead of assuming it exists; the driver reports the query as
# unavailable when the running headers predate it.
HAVE_XDNA2_RESOURCE_INFO := $(shell echo 'int main(void){return (int)DRM_AMDXDNA_QUERY_RESOURCE_INFO;}' | $(CC) $(CPPFLAGS) -include drm/amdxdna_accel.h -x c - -o /dev/null >/dev/null 2>&1 && echo 1 || echo 0)
ifeq ($(HAVE_XDNA2_RESOURCE_INFO),1)
CPPFLAGS += -DCOLI_HAVE_XDNA2_RESOURCE_INFO
endif

TESTS := tests/test_backends tests/vulkan_runtime_test tests/vulkan_debug_harness \
         tests/npu_device_test tests/test_placement tests/test_moe_schedule

TOOLS := bench/backend_bench

all: $(TESTS) $(TOOLS)

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

npu/xdna2_backend.o: npu/xdna2_backend.c npu/xdna2_backend.h $(NPU_DIR)/xdna2_matmul.h $(NPU_DIR)/xdna2_xrt_driver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(NPU_DIR)/xdna2_driver.o: $(NPU_DIR)/xdna2_driver.c $(NPU_DIR)/xdna2_driver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(NPU_DIR)/xdna2_xrt_driver.o: $(NPU_DIR)/xdna2_xrt_driver.c $(NPU_DIR)/xdna2_xrt_driver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(NPU_DIR)/xdna2_matmul.o: $(NPU_DIR)/xdna2_matmul.c $(NPU_DIR)/xdna2_matmul.h $(NPU_DIR)/xdna2_driver.h $(NPU_DIR)/xdna2_xrt_driver.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# The vnni building blocks never link XRT: xdna2_xrt_driver.c compiles to the
# "no XRT" probe unless the build explicitly defines COLI_NPU_XRT_AVAILABLE.
NPU_OBJS := npu/xdna2_backend.o $(NPU_DIR)/xdna2_driver.o \
            $(NPU_DIR)/xdna2_xrt_driver.o $(NPU_DIR)/xdna2_matmul.o

# The scheduler plans work; it dispatches none, so it needs neither the Vulkan
# nor the amdxdna headers and its tests run on any host.
sched/%.o: sched/%.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

SCHED_OBJS := sched/shape_profile.o sched/npu_shapes.o sched/backend_placement.o \
              sched/moe_schedule.o

bench/backend_bench.o: bench/backend_bench.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

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

tests/test_placement: tests/test_placement.o $(SCHED_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/test_moe_schedule: tests/test_moe_schedule.o $(SCHED_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Produces the measured table the placement policy reads. It must run on the
# machine that has the silicon; engines that are absent are recorded as skipped
# rather than guessed.
bench/backend_bench: bench/backend_bench.o cpu/vnni_cpu_backend.o \
                     gpu/vulkan_backend.o $(NPU_OBJS) $(SCHED_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# ── Test / clean ──────────────────────────────────────────────────────────────

test: $(TESTS)
	./tests/test_backends
	./tests/vulkan_runtime_test
	./tests/vulkan_debug_harness
	./tests/npu_device_test
	./tests/test_placement
	./tests/test_moe_schedule

podman-test: scripts/strix-halo-podman-test.sh
	./scripts/strix-halo-podman-test.sh

# test_output.log is deliberately NOT removed here. It is written on the host by
# scripts/strix-halo-podman-test.sh with `tee`, while the container runs
# `make clean` in the same bind-mounted directory: removing it unlinks the file
# the running `tee` still holds open, so the harness finishes with no log on
# disk and CI reports "no test log produced".
clean:
	rm -f $(TESTS) $(TOOLS) tests/*.o cpu/*.o npu/*.o gpu/*.o sched/*.o bench/*.o \
	      $(NPU_DIR)/*.o

.PHONY: all shader test podman-test clean
