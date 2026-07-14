UNAME_S := $(shell uname -s)
MINGW    := $(findstring MINGW,$(UNAME_S))
MSYS     := $(findstring MSYS,$(UNAME_S))
IS_WIN   := $(MINGW)$(MSYS)

ifeq ($(UNAME_S),Darwin)
# --- macOS / Apple Silicon ---
# Apple clang non include il runtime OpenMP: se c'e' libomp di Homebrew lo usa
# (brew install libomp), altrimenti compila single-thread (i pragma omp sono ignorati).
# Niente -march: su arm64 NEON e' baseline (i kernel __ARM_NEON si attivano da soli).
CC      = clang
OMPDIR := $(shell brew --prefix libomp 2>/dev/null)
ifneq ($(OMPDIR),)
OMPC    = -Xclang -fopenmp -I$(OMPDIR)/include
OMPL    = -L$(OMPDIR)/lib -lomp
else
$(warning libomp not found: building single-threaded. For multithreading: brew install libomp)
OMPC    =
OMPL    =
endif
CFLAGS  = -O3 $(OMPC) -Wall -Wextra -Wno-unused-parameter -Wno-misleading-indentation -Wno-unused-function
LDFLAGS = -lm $(OMPL)
EXE     =
else ifneq ($(IS_WIN),)
# --- Windows 11 x86-64 (MinGW-w64 / MSYS2) ---
# GCC + libgomp + winpthreads: pthread, OpenMP, clock_gettime, opendir/readdir,
# AVX2 intrinsics — tutto gratis, nessun porting.
# ARCH default = x86-64-v3 (binario portabile con AVX2). Per max velocita'
# su QUESTA macchina: make ARCH=native
CC      = gcc
ARCH   ?= x86-64-v3
CFLAGS  = -D_FILE_OFFSET_BITS=64 -O3 -march=$(ARCH) -fopenmp -Wall -Wextra -Wno-unused-parameter -Wno-misleading-indentation -Wno-unused-function
LDFLAGS = -lm -fopenmp -static
EXE     = .exe
else
UNAME_M := $(shell uname -m)
ifneq (,$(filter ppc64le ppc64,$(UNAME_M)))
# --- Linux PowerPC (POWER8/POWER9/POWER10) ---
# PowerPC GCC uses -mcpu, not -march. ARCH=native works on gcc >= 4.7.
# The AVX2/NEON kernels fall back to the portable scalar C path
# (validated token-exact vs the transformers oracle on a POWER8 S824).
CC      = gcc
ARCH   ?= native
CFLAGS  = -O3 -mcpu=$(ARCH) -fopenmp -Wall -Wextra -Wno-unused-parameter -Wno-misleading-indentation -Wno-unused-function
LDFLAGS = -lm -fopenmp
EXE     =
else
# --- Linux x86-64 (percorso originale, invariato) ---
CC      = gcc
# ARCH=native -> ottimizzato per QUESTA macchina (default, piu' veloce).
# ARCH=x86-64-v3 -> binario PORTABILE su qualsiasi x86-64 moderno con AVX2 (per distribuire).
# ARCH=x86-64 -> massima compatibilita' (niente AVX2: usa il path scalare di fallback).
ARCH   ?= native
CFLAGS  = -O3 -march=$(ARCH) -fopenmp -Wall -Wextra -Wno-unused-parameter -Wno-misleading-indentation -Wno-unused-function
LDFLAGS = -lm -fopenmp
EXE     =
endif
endif

# CUDA=1 adds an opt-in backend for resident tensors. The default build remains
# pure C and keeps the original zero-dependency runtime.
CUDA      ?= 0
CUDA_HOME ?= /usr/local/cuda
NVCC      ?= $(CUDA_HOME)/bin/nvcc
CUDA_ARCH ?= native
NVCCFLAGS ?= -O3 -std=c++17 -arch=$(CUDA_ARCH) -Xcompiler=-Wall,-Wextra
PYTHON    ?= python3
BACKEND_OBJ :=
BACKEND_OBJ += backend_runtime.o
TEST_BINS = tests/test_json$(EXE) tests/test_st$(EXE) tests/test_tier$(EXE) tests/test_grammar$(EXE) tests/test_qwen35_moe$(EXE) tests/test_backend_npu$(EXE) tests/test_backend_runtime$(EXE)
NPU_BACKEND_CFLAGS =
VULKAN_BACKEND_CFLAGS =
ifeq ($(CUDA),1)
ifeq ($(UNAME_S),Darwin)
$(error CUDA=1 is supported only on Linux)
endif
ifneq ($(IS_WIN),)
# GPU: stub only in Phase 1 (G0). G1 builds coli_cuda.dll with MSVC+nvcc.
$(error CUDA=1 on Windows requires G1: build coli_cuda.dll with MSVC+nvcc (see PORT_WINDOWS_PLAN.md §8))
endif
CFLAGS   += -DCOLI_CUDA
LDFLAGS  += -L$(CUDA_HOME)/lib64 -Wl,-rpath,$(CUDA_HOME)/lib64 -lcudart -lstdc++
BACKEND_OBJ  += backend_cuda.o
endif

# ROCM=1 builds the ROCm/HIP backend instead of CUDA.
# Requires ROCm ≥ 6.0 with hipcc at /opt/rocm/bin/hipcc (override via ROCM_HOME).
# Targets AMD GPUs and AMD APUs (Strix Halo, Radeon 890M) with unified memory.
# The ROCm backend exports the same coli_cuda_* API as the CUDA backend, so
# glm.c is compiled with -DCOLI_CUDA -DCOLI_ROCM and uses the same code paths.
ROCM      ?= 0
ROCM_HOME ?= /opt/rocm
HIPCC     ?= $(ROCM_HOME)/bin/hipcc
ROCM_ARCH ?= native
HIPCCFLAGS ?= -O3 -std=c++17 -Xcompiler=-Wall,-Wextra
ifeq ($(ROCM),1)
ifeq ($(CUDA),1)
$(error CUDA=1 and ROCM=1 are mutually exclusive)
endif
ifeq ($(UNAME_S),Darwin)
$(error ROCM=1 is supported only on Linux)
endif
ifneq ($(IS_WIN),)
$(error ROCM=1 is not supported on Windows)
endif
ifneq ($(ROCM_ARCH),native)
HIPCCFLAGS += --offload-arch=$(ROCM_ARCH)
endif
CFLAGS   += -DCOLI_CUDA -DCOLI_ROCM
LDFLAGS  += -L$(ROCM_HOME)/lib -Wl,-rpath,$(ROCM_HOME)/lib -lamdhip64 -lstdc++
BACKEND_OBJ  += backend_rocm.o
endif

# NPU=1 builds the NPU backend shim using the same ABI as the CUDA/ROCm/Vulkan
# backends while keeping a CPU reference path for portability.
NPU ?= 0
ifeq ($(NPU),1)
CFLAGS   += -DCOLI_ENABLE_NPU -DCOLI_NPU
BACKEND_OBJ  += backend_npu.o
NPU_BACKEND_CFLAGS += \
	-Dcoli_cuda_init=coli_npu_compat_init \
	-Dcoli_cuda_shutdown=coli_npu_compat_shutdown \
	-Dcoli_cuda_device_count=coli_npu_compat_device_count \
	-Dcoli_cuda_device_at=coli_npu_compat_device_at \
	-Dcoli_cuda_mem_info=coli_npu_compat_mem_info \
	-Dcoli_cuda_stats=coli_npu_compat_stats \
	-Dcoli_cuda_tensor_upload=coli_npu_compat_tensor_upload \
	-Dcoli_cuda_matmul=coli_npu_compat_matmul \
	-Dcoli_cuda_tensor_free=coli_npu_compat_tensor_free \
	-Dcoli_cuda_tensor_bytes=coli_npu_compat_tensor_bytes \
	-Dcoli_cuda_tensor_device=coli_npu_compat_tensor_device
endif

# VULKAN=1 builds the Vulkan backend shim, which exposes the same coli_cuda_*
# ABI while using a CPU reference path when no Vulkan runtime is present.
VULKAN ?= 0
ifeq ($(VULKAN),1)
ifeq ($(UNAME_S),Darwin)
$(error VULKAN=1 is supported only on Linux)
endif
ifneq ($(IS_WIN),)
$(error VULKAN=1 is not supported on Windows)
endif
CFLAGS   += -DCOLI_ENABLE_VULKAN -DCOLI_CUDA -DCOLI_VULKAN
LDFLAGS  += -ldl
BACKEND_OBJ  += backend_vulkan.o
VULKAN_BACKEND_CFLAGS += \
	-Dcoli_cuda_init=coli_vulkan_init \
	-Dcoli_cuda_shutdown=coli_vulkan_shutdown \
	-Dcoli_cuda_device_count=coli_vulkan_device_count \
	-Dcoli_cuda_device_at=coli_vulkan_device_at \
	-Dcoli_cuda_mem_info=coli_vulkan_mem_info \
	-Dcoli_cuda_stats=coli_vulkan_stats \
	-Dcoli_cuda_tensor_upload=coli_vulkan_tensor_upload \
	-Dcoli_cuda_matmul=coli_vulkan_matmul \
	-Dcoli_cuda_tensor_free=coli_vulkan_tensor_free \
	-Dcoli_cuda_tensor_bytes=coli_vulkan_tensor_bytes \
	-Dcoli_cuda_tensor_device=coli_vulkan_tensor_device
endif

all: glm$(EXE) qwen35_moe$(EXE)

# phony 'glm' → 'glm.exe' on Windows (so 'make glm' and 'coli build' work on every platform)
glm: glm$(EXE)
qwen35_moe_build: qwen35_moe$(EXE)

glm$(EXE): glm.c st.h json.h tok.h tok_unicode.h compat.h grammar.h $(BACKEND_OBJ)
	$(CC) $(CFLAGS) glm.c $(BACKEND_OBJ) -o glm$(EXE) $(LDFLAGS)

qwen35_moe$(EXE): qwen35_moe.c st.h json.h compat.h $(BACKEND_OBJ)
	$(CC) $(CFLAGS) qwen35_moe.c $(BACKEND_OBJ) -o qwen35_moe$(EXE) $(LDFLAGS)

backend_cuda.o: backend_cuda.cu backend_cuda.h
	@command -v $(NVCC) >/dev/null 2>&1 || { echo "nvcc not found: set CUDA_HOME or NVCC" >&2; exit 1; }
	$(NVCC) $(NVCCFLAGS) -c backend_cuda.cu -o $@

cuda-test: backend_cuda.cu backend_cuda.h tests/test_backend_cuda.cu
	@command -v $(NVCC) >/dev/null 2>&1 || { echo "nvcc not found: set CUDA_HOME or NVCC" >&2; exit 1; }
	$(NVCC) $(NVCCFLAGS) backend_cuda.cu tests/test_backend_cuda.cu -o backend_cuda_test$(EXE)
	./backend_cuda_test$(EXE)

backend_rocm.o: backend_rocm.hip backend_rocm.h
	@command -v $(HIPCC) >/dev/null 2>&1 || { echo "hipcc not found: install ROCm or set ROCM_HOME" >&2; exit 1; }
	$(HIPCC) $(HIPCCFLAGS) -c backend_rocm.hip -o $@

rocm-test: backend_rocm.hip backend_rocm.h tests/test_backend_rocm.hip
	@command -v $(HIPCC) >/dev/null 2>&1 || { echo "hipcc not found: install ROCm or set ROCM_HOME" >&2; exit 1; }
	$(HIPCC) $(HIPCCFLAGS) backend_rocm.hip tests/test_backend_rocm.hip -o backend_rocm_test$(EXE)
	./backend_rocm_test$(EXE)

backend_runtime.o: backend_runtime.c backend_runtime.h parallelism.h
	$(CC) $(CFLAGS) -c backend_runtime.c -o $@

backend_npu.o: backend_npu.c backend_npu.h
	$(CC) $(CFLAGS) $(NPU_BACKEND_CFLAGS) -c backend_npu.c -o $@

npu-test: backend_npu.c backend_npu.h tests/test_backend_npu.c
	$(CC) $(CFLAGS) backend_npu.c tests/test_backend_npu.c -o backend_npu_test$(EXE) $(LDFLAGS)
	./backend_npu_test$(EXE)

backend_vulkan.o: backend_vulkan.c backend_vulkan.h
	$(CC) $(CFLAGS) $(VULKAN_BACKEND_CFLAGS) -c backend_vulkan.c -o $@

vulkan-test: backend_vulkan.c backend_vulkan.h tests/test_backend_vulkan.c
	$(CC) $(CFLAGS) backend_vulkan.c tests/test_backend_vulkan.c -o backend_vulkan_test$(EXE) $(LDFLAGS)
	./backend_vulkan_test$(EXE)

olmoe$(EXE): olmoe.c st.h json.h compat.h
	$(CC) $(CFLAGS) olmoe.c -o olmoe$(EXE) $(LDFLAGS)

# binario portabile da distribuire su altre macchine x86-64
portable:
	$(MAKE) glm$(EXE) ARCH=x86-64-v3

iobench$(EXE): iobench.c compat.h
	$(CC) $(CFLAGS) iobench.c -o iobench$(EXE) $(LDFLAGS)

tests/test_json$(EXE): tests/test_json.c json.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

tests/test_st$(EXE): tests/test_st.c st.h json.h compat.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

tests/test_tier$(EXE): tests/test_tier.c tier.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

tests/test_grammar$(EXE): tests/test_grammar.c grammar.h
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

tests/test_backend_npu$(EXE): tests/test_backend_npu.c backend_npu.o
	$(CC) $(CFLAGS) $< backend_npu.o -o $@ $(LDFLAGS)

tests/test_backend_runtime$(EXE): tests/test_backend_runtime.c backend_runtime.o
	$(CC) $(CFLAGS) $< backend_runtime.o -o $@ $(LDFLAGS)

tests/test_qwen35_moe$(EXE): tests/test_qwen35_moe.c qwen35_moe$(EXE)
	$(CC) $(CFLAGS) $< -o $@ $(LDFLAGS)

test-c: qwen35_moe$(EXE) $(TEST_BINS)
	@for test in $(TEST_BINS); do ./$$test || exit 1; done

test-python:
	$(PYTHON) -m unittest discover -s tests -p 'test_*.py'

test: test-c test-python

# Local validation: one portable CPU build and dependency-free tests.
check:
	$(MAKE) clean
	$(MAKE) portable
	$(MAKE) test

clean:
	rm -f olmoe$(EXE) glm$(EXE) qwen35_moe$(EXE) iobench$(EXE) backend_runtime.o backend_cuda.o backend_cuda_test$(EXE) backend_rocm.o backend_rocm_test$(EXE) backend_npu.o backend_npu_test$(EXE) backend_vulkan.o backend_vulkan_test$(EXE) $(TEST_BINS)
	rm -rf tests/__pycache__

.PHONY: all glm qwen35_moe_build cuda-test rocm-test npu-test vulkan-test portable test-c test-python test check clean
