# Debug summary

- The AVX-512 VNNI path uses the sign-flip trick with `_mm512_dpbusd_epi32` to keep signed int8 dot products correct.
- The backend is intended to run only on hosts that expose the VNNI instruction set; on non-Strix-Halo hosts the test harness reports that the backend is unavailable instead of using a generic scalar fallback.
- The Vulkan and XDNA2 implementations are intentionally narrow wrappers so the project remains simple and focused on the Strix Halo workload.
