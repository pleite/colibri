# Plan: CI Test Integration — Add Test Step After Build

Date: 2026-07-17
Author: Murderbot (via Hermes)
Status: Draft

== Problem ==

Current CI workflow (.github/workflows/container-images.yml) only builds container images. No tests run after build. This means:
- Backend compilation errors may not be caught until runtime
- No validation that compiled binaries actually work
- No regression testing for multi-backend builds

== Current Test Files ==

c/tests/:
- test_backend_runtime.c (3789 bytes) - Runtime backend tests
- test_backend_npu.c (2509 bytes) - NPU backend tests
- test_backend_vulkan.c (2509 bytes) - Vulkan backend tests
- test_backend_parallel.c (3611 bytes) - Parallel backend tests
- test_qwen35_moe.c (35302 bytes) - Main model tests
- test_qwen35_quant_converter.py (38292 bytes) - Quantization tests
- test_openai_server.py (14828 bytes) - Server integration tests
- test_doctor.py (8518 bytes) - Doctor/test suite

== Solution ==

Add test step to CI workflow that:
1. Runs inside built container
2. Executes compiled test binaries
3. Reports pass/fail for each backend

== Implementation ==

Update .github/workflows/container-images.yml:

After build-and-push step, add test step:

    - name: Run backend tests
      run: |
        docker run --rm colibri:${{ matrix.backend }} /bin/bash -c 
