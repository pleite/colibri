# Debug summary

- The AVX-512 VNNI path uses the sign-flip trick with `_mm512_dpbusd_epi32` to keep signed int8 dot products correct.
- The fallback path is scalar and is used when the local host does not expose the VNNI instruction set.
- The Vulkan and XDNA2 implementations are intentionally narrow wrappers so the project remains simple and focused on the Strix Halo workload.
