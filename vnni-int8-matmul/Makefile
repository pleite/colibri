CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -std=c11
CPPFLAGS += -I.
LDFLAGS += -lm -ldl

# Strix Halo targets require AVX-512 VNNI support. The default build uses the
# required flags for that target; override CPU_FLAGS only if you need a custom
# toolchain configuration.
CPU_FLAGS ?= -mavx512f -mavx512vnni -mavx512bw -mavx512dq

all: tests/test_backends tests/vulkan_runtime_test

tests/test_backends: tests/test_backends.o cpu/vnni_cpu_backend.o gpu/vulkan_backend.o npu/xdna2_backend.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

tests/vulkan_runtime_test: tests/vulkan_runtime_test.o gpu/vulkan_backend.o cpu/vnni_cpu_backend.o
	$(CC) $(CPPFLAGS) $(CFLAGS) -o $@ $^ $(LDFLAGS)

cpu/vnni_cpu_backend.o: cpu/vnni_cpu_backend.c cpu/vnni_cpu_backend.h
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CPU_FLAGS) -c $< -o $@

gpu/vulkan_backend.o: gpu/vulkan_backend.c gpu/vulkan_backend.h cpu/vnni_cpu_backend.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

npu/xdna2_backend.o: npu/xdna2_backend.c npu/xdna2_backend.h cpu/vnni_cpu_backend.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests/test_backends.o: tests/test_backends.c cpu/vnni_cpu_backend.h gpu/vulkan_backend.h npu/xdna2_backend.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

tests/vulkan_runtime_test.o: tests/vulkan_runtime_test.c gpu/vulkan_backend.h
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

test: tests/test_backends tests/vulkan_runtime_test
	./tests/test_backends
	./tests/vulkan_runtime_test

clean:
	rm -f tests/test_backends tests/vulkan_runtime_test tests/*.o cpu/*.o npu/*.o gpu/*.o

.PHONY: all test clean
