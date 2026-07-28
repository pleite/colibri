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
PTHREAD_FLAGS ?= -pthread

HAVE_VULKAN := $(shell test -f /usr/include/vulkan/vulkan_core.h && echo 1 || echo 0)
HAVE_NPU_HEADERS := $(shell test -f /usr/include/drm/amdxdna_accel.h && echo 1 || echo 0)
ifeq ($(HAVE_VULKAN),1)
CPPFLAGS += -DCOLI_HAVE_VULKAN
GPU_BACKEND_OBJ := gpu/vulkan_backend.o
else
GPU_BACKEND_OBJ :=
endif
ifeq ($(HAVE_NPU_HEADERS),1)
CPPFLAGS += -DCOLI_HAVE_NPU
NPU_OBJS := npu/xdna2_backend.o $(NPU_DIR)/xdna2_driver.o \
            $(NPU_DIR)/xdna2_xrt_driver.o $(NPU_DIR)/xdna2_matmul.o
else
NPU_OBJS :=
endif

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

TOOLS := benchmark_all_backends bench/backend_bench tools/npu_shapes_list tools/placement_report

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

# The scheduler plans work; it dispatches none, so it needs neither the Vulkan
# nor the amdxdna headers and its tests run on any host.
sched/%.o: sched/%.c sched/shape_profile.h sched/npu_shapes.h \
           sched/backend_placement.h sched/moe_schedule.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

SCHED_OBJS := sched/shape_profile.o sched/npu_shapes.o sched/backend_placement.o \
              sched/moe_schedule.o

# Deliberately not in SCHED_OBJS: this is the one scheduler object that links
# the backends, so the scheduler's own tests keep building on a host with none
# of the silicon.
sched/engine_caps.o: sched/engine_caps.c sched/engine_caps.h \
                     sched/backend_placement.h cpu/vnni_cpu_backend.h \
                     gpu/vulkan_backend.h npu/xdna2_backend.h

# Every consumer sees the scheduler and backend headers. Listing them is not
# cosmetic: these objects embed struct layouts, so a stale object linked against
# a changed header corrupts the stack rather than failing to build.
PUBLIC_HEADERS := sched/shape_profile.h sched/npu_shapes.h \
                  sched/backend_placement.h sched/moe_schedule.h \
                  sched/engine_caps.h cpu/vnni_cpu_backend.h \
                  gpu/vulkan_backend.h npu/xdna2_backend.h

benchmark_all_backends.o: benchmark_all_backends.c cpu/vnni_cpu_backend.h gpu/vulkan_backend.h npu/xdna2_backend.h $(NPU_DIR)/xdna2_matmul.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PTHREAD_FLAGS) -c $< -o $@

bench/backend_bench.o: bench/backend_bench.c $(PUBLIC_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tools/%.o: tools/%.c $(PUBLIC_HEADERS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests/%.o: tests/%.c $(PUBLIC_HEADERS)
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

# Sweeps a compact set of batch/thread/shape settings and writes a CSV for
# placement and memory-bandwidth experiments.
benchmark_all_backends: benchmark_all_backends.o cpu/vnni_cpu_backend.o \
                       $(GPU_BACKEND_OBJ) $(NPU_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) $(PTHREAD_FLAGS) -o $@ $^ $(LDFLAGS)

# Produces the measured table the placement policy reads. It must run on the
# machine that has the silicon; engines that are absent are recorded as skipped
# rather than guessed.
bench/backend_bench: bench/backend_bench.o cpu/vnni_cpu_backend.o \
                     $(GPU_BACKEND_OBJ) $(NPU_OBJS) $(SCHED_OBJS)
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Prints the enumerated NPU shape set so the kernel build reads it from
# sched/npu_shapes.c instead of carrying a second copy of the enumeration.
tools/npu_shapes_list: tools/npu_shapes_list.o sched/npu_shapes.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

# Prints, for this host, which engine each enumerated shape would run on and
# why the others were refused. Probes the devices, so it links the backends.
tools/placement_report: tools/placement_report.o sched/engine_caps.o \
                        $(SCHED_OBJS) cpu/vnni_cpu_backend.o \
                        gpu/vulkan_backend.o $(NPU_OBJS)
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

podman-debug-test: scripts/strix-halo-podman-test.sh scripts/capture_debug_snapshot.sh
	COLI_CAPTURE_DEBUG=1 ./scripts/strix-halo-podman-test.sh

podman-debug-benchmark: scripts/run_npu_benchmark_in_container.sh scripts/capture_debug_snapshot.sh
	COLI_CAPTURE_DEBUG=1 ./scripts/run_npu_benchmark_in_container.sh

# ── NPU kernel artifacts ──────────────────────────────────────────────────────
# The AIE toolchain (IRON + Peano) is pinned in npu/aie/toolchain.lock and lives
# in its own build-only container image; it is not part of the test image and
# needs no NPU device. See npu/aie/README.md.

npu-toolchain-image:
	./scripts/build-npu-kernels.sh --image-only

npu-kernels: tools/npu_shapes_list
	./scripts/build-npu-kernels.sh

# Host-side checks that need no toolchain and no NPU: the container packer
# round-trips, and the enumerated shape set is printable.
npu-kernels-check: tools/npu_shapes_list
	python3 npu/aie/pack_npukernel.py --self-test
	./tools/npu_shapes_list

# test_output.log is deliberately NOT removed here. It is written on the host by
# scripts/strix-halo-podman-test.sh with `tee`, while the container runs
# `make clean` in the same bind-mounted directory: removing it unlinks the file
# the running `tee` still holds open, so the harness finishes with no log on
# disk and CI reports "no test log produced".
clean:
	rm -f $(TESTS) $(TOOLS) tests/*.o cpu/*.o npu/*.o gpu/*.o sched/*.o bench/*.o \
	      tools/*.o $(NPU_DIR)/*.o
	rm -rf npu/aie/build npu/aie/__pycache__

.PHONY: all shader test podman-test podman-debug-test podman-debug-benchmark clean \
        npu-toolchain-image npu-kernels npu-kernels-check
