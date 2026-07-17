# Plan: Fix ROCm Package Installation for hipcc/clang-17

Date: 2026-07-17
Author: Murderbot (via Hermes)
Status: Draft

== Problem ==

The ROCm build fails with:
```
sh: 1: /opt/rocm/llvm/bin/clang-17: not found
Cant exec /opt/rocm/bin/rocm_agent_enumerator: No such file or directory
