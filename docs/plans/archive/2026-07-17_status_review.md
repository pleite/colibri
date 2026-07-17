# Status Review: Colibri CI and Framework State

Date: 2026-07-17T11:00Z
Author: Murderbot (via Hermes)

== Current CI Status ==

PR 52 (MERGED) - Fix container image builds for Ubuntu-based CI
- Copilot converted Dockerfile.colibri from Fedora to Ubuntu 24.04
- Added XRT_DEB_URL/XRT_DEB_SHA256 build args for NPU fallback
- FAILED because ROCm APT source uses ubuntu main but the repo has noble and jammy subdirectories

PR 53 (OPEN) - Fix: use Ubuntu 24.04 codename noble in ROCm APT source
- Copilot fix: changes ubuntu main to noble main
- Waiting for CI to validate

Latest CI run on main (PR 52 merge):
- FAIL: The repository https://repo.radeon.com/rocm/apt/7.2.4 ubuntu Release does not have a Release file
- Root cause: ROCm repo path is .../7.2.4/dists/noble/ not .../7.2.4/dists/ubuntu/
- Confirmed: curl -sI https://repo.radeon.com/rocm/apt/7.2.4/dists/noble/Release returns 200

CI run on main (before PR 52):
- SUCCESS - commit 96dbbcd (toolchain setup guide) - this was on the Fedora Dockerfile

== What Copilot Fixed (PR 52) ==

1. FROM fedora:43 to FROM ubuntu:24.04
2. dnf to apt-get
3. copr enable xanderlent/amd-npu-driver to apt install xrt libxrt-dev (with fallback)
4. vulkan-devel to libvulkan-dev, glslc to glslang-tools, vulkan-loader to libvulkan1
5. ROCm packages: rocm-llvm rocm-device-libs hip-runtime-amd hip-devel rocblas rocblas-devel hipblas hipblas-devel rocm-cmake libomp-devel libomp to rocm-llvm rocm-device-libs hip-runtime-amd hip-devel librocblas-dev libhipblas-dev rocm-cmake libomp-dev
6. Added DEBIAN_FRONTEND=noninteractive
7. Added XRT_DEB_URL and XRT_DEB_SHA256 ARGs for NPU fallback

== What Still Needs Fixing ==

1. ROCm APT Source - ubuntu to noble in Dockerfile.colibri line 26
   Status: PR 53 open, awaiting merge

2. NPU/XRT package availability - xrt may not be in Ubuntu 24.04 repos
   Status: Needs validation - if xrt is available, no fallback needed

3. ROCm 7.2.4 package availability on Ubuntu - verify packages exist
   Status: Needs validation after noble fix

== Plan Archive Status ==

Active plans:
- 2026-07-15_copilot-ornith-feature-completeness.md (23.8KB)
- 2026-07-15_engine-capability-audit.md (20.3KB)
- 2026-07-16_backend-parity-gap-list.md (11.7KB)
- 2026-07-17_ci-framework-migration.md (9.2KB) - SUPERSEDED by PR 52
- 2026-07-17_llama-server-int4-analysis.md (3.6KB)
- 2026-07-17_toolchain-setup.md (4.7KB)
- backend-gap-analysis.md (15.6KB)

== Gap Analysis ==

HIGH PRIORITY:
1. ROCm APT source codename fix (PR 53)
2. NPU/XRT package availability validation
3. ROCm 7.2.4 package availability verification
4. CI workflow validation after ROCm fix

MEDIUM PRIORITY:
5. Test coverage for multi-backend builds (no test step in CI)
6. Docker image size optimization (no multi-stage build)
7. Backend runtime test compilation (test_backend_runtime.c exists but not in Makefile)
8. OpenAI server integration tests (test_openai_server.py not in CI)
9. Quantized tensor test (test_qwen35_quant_converter.py not in CI)
10. Doctor/test suite (test_doctor.py not in CI)

LOW PRIORITY:
11. Docker image tagging strategy
12. Security scanning (Trivy)
13. Documentation for container usage

== Next Steps ==
1. Merge PR 53 (ROCm noble fix)
2. Monitor CI - verify all 4 builds pass
3. If NPU fails - investigate xrt availability
4. Add test step to CI
5. Archive superseded plans
6. Build optimization plan (multi-stage Dockerfile)
