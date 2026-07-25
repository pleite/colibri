cmake_minimum_required(VERSION 3.16)
project(colibri_npu_kernels C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# ── Compiler flags ──

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O3 -march=native -Wall -Wextra")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

# ── XRT paths (optional) ──

if(DEFINED ENV{XILINX_XRT})
    set(XRT_DIR "$ENV{XILINX_XRT}")
elseif(EXISTS "/opt/xrt")
    set(XRT_DIR "/opt/xrt")
else()
    set(XRT_DIR "")
endif()

if(XRT_DIR)
    list(APPEND CMAKE_PREFIX_PATH "${XRT_DIR}/share/cmake/XRT")
    find_package(XRT QUIET)
    if(XRT_FOUND)
        message(STATUS "XRT found: ${XRT_DIR}")
        add_definitions(-DCOLI_NPU_XRT_AVAILABLE)
        include_directories(${XRT_INCLUDE_DIRS})
        link_directories(${XRT_LINK_DIRS})
    else()
        message(STATUS "XRT not found — NPU will use DRM ioctl path only")
    endif()
else()
    message(STATUS "XRT not found — NPU will use DRM ioctl path only")
endif()

# ── NPU kernel sources ──

set(NPU_SOURCES
    npu_kernels/xdna2_driver.c
    npu_kernels/xdna2_matmul.c
)

set(NPU_HEADERS
    npu_kernels/xdna2_driver.h
    npu_kernels/xdna2_matmul.h
    npu_kernels/npu_runtime.h
)

# ── Test binary ──

add_executable(test_npu tests/test_npu.c ${NPU_SOURCES})
target_include_directories(test_npu PRIVATE npu_kernels)
target_link_libraries(test_npu PRIVATE dl pthread)

if(XRT_FOUND)
    target_link_libraries(test_npu PRIVATE ${XRT_CORE_LIBRARIES})
endif()

# ── Install ──

install(TARGETS test_npu DESTINATION bin)
install(FILES ${NPU_HEADERS} DESTINATION include/colibri/npu)

