CC ?= gcc
CFLAGS ?= -O3 -Wall -Wextra -std=c11
CPPFLAGS += -I.
LDFLAGS += -lm -ldl

CPU_FLAGS ?= -mavx512f -mavx512vnni -mavx512bw -mavx512dq

all: tests/test_backends

tests/test_backends: tests/test_backends.c cpu/vnni_cpu_backend.c gpu/vulkan_backend.c npu/xdna2_backend.c
	$(CC) $(CPPFLAGS) $(CFLAGS) $(CPU_FLAGS) -o $@ $^ $(LDFLAGS)

test: tests/test_backends
	./tests/test_backends

clean:
	rm -f tests/test_backends

.PHONY: all test clean
