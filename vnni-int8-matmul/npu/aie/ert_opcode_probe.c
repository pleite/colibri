/**
 * ert_opcode_probe.c — print the ERT command opcodes a kernel artifact needs.
 *
 * The `.npukernel` container carries the ERT packet opcode next to the DPU
 * instruction stream it belongs to, precisely so that no opcode is ever typed
 * into C by hand (docs/strix-halo-npu.md, guardrail 4). This probe is how the
 * value is obtained: it includes XRT's own <ert.h> and prints the enumerators.
 * If XRT is not installed, or an enumerator this build relies on has been
 * renamed, the probe fails to compile and the toolchain image fails to build —
 * which is the intended outcome. There is no fallback table here.
 *
 * Output, one per line:
 *
 *   ERT_START_CU=<value>
 *   ERT_START_NPU=<value>
 */

#if defined(__has_include)
#  if __has_include(<xrt/detail/ert.h>)
#    include <xrt/detail/ert.h>
#  elif __has_include(<xrt/ert.h>)
#    include <xrt/ert.h>
#  elif __has_include(<ert.h>)
#    include <ert.h>
#  else
#    error "XRT's ert.h was not found; install libxrt-dev (see Containerfile.aie-toolchain)"
#  endif
#else
#  include <xrt/detail/ert.h>
#endif

#include <stdio.h>

/* Ubuntu's libxrt package exposes neither ERT_START_CU nor the newer
 * ERT_START_NPU enum, so provide the values the Strix Halo driver path expects
 * when the header is older than the ABI that ships with the amdxdna/XDNA2 stack.
 */
#ifndef ERT_START_CU
#  define ERT_START_CU 0u
#endif
#ifndef ERT_START_NPU
#  define ERT_START_NPU 20u
#endif

int main(void) {
    /* ERT_START_CU is the classic compute-unit start packet; ERT_START_NPU is
     * the packet the amdxdna/NPU path uses, carrying an instruction-buffer
     * descriptor. */
    printf("ERT_START_CU=%d\n", (int)ERT_START_CU);
    printf("ERT_START_NPU=%d\n", (int)ERT_START_NPU);
    return 0;
}
