module {
  aie.device(npu2_1col) {
    %logical_core = aie.logical_tile<CoreTile>(?, ?)
    %logical_shim_noc = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_mem = aie.logical_tile<MemTile>(?, ?)
    %logical_shim_noc_0 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_mem_1 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_2 = aie.logical_tile<MemTile>(?, ?)
    %logical_shim_noc_3 = aie.logical_tile<ShimNOCTile>(?, ?)
    aie.objectfifo @inA(%logical_shim_noc, {%logical_mem}, 2 : i32) : !aie.objectfifo<memref<16x32xi8>>  
    aie.objectfifo @memA(%logical_mem dimensionsToStream [<size = 2, stride = 256>, <size = 4, stride = 8>, <size = 8, stride = 32>, <size = 8, stride = 1>], {%logical_core}, 2 : i32) : !aie.objectfifo<memref<16x32xi8>>  
    aie.objectfifo.link [@inA] -> [@memA]([] [0])
    aie.objectfifo @inB(%logical_shim_noc_0, {%logical_mem_1}, 2 : i32) : !aie.objectfifo<memref<32x256xi8>>  
    aie.objectfifo @memB(%logical_mem_1 dimensionsToStream [<size = 32, stride = 256>, <size = 4, stride = 8>, <size = 8, stride = 32>, <size = 8, stride = 1>], {%logical_core}, 2 : i32) : !aie.objectfifo<memref<32x256xi8>>  
    aie.objectfifo.link [@inB] -> [@memB]([] [0])
    aie.objectfifo @memC(%logical_core, {%logical_mem_2}, 2 : i32) : !aie.objectfifo<memref<16x256xi32>>  
    aie.objectfifo @outC(%logical_mem_2 dimensionsToStream [<size = 2, stride = 2048>, <size = 8, stride = 8>, <size = 32, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_3}, 2 : i32) : !aie.objectfifo<memref<16x256xi32>>  
    aie.objectfifo.link [@memC] -> [@outC]([] [0])
    func.func private @zero_i32(memref<4096xi32>) attributes {link_with = "matmul_i8_i32_58a6969e.o"}
    func.func private @"58a6969e_matmul_i8_i32"(memref<512xi8>, memref<8192xi8>, memref<4096xi32>) attributes {link_with = "matmul_i8_i32_58a6969e.o"}
    %0 = aie.core(%logical_core) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_4 = arith.constant 0 : index
        %c128 = arith.constant 128 : index
        %c1_5 = arith.constant 1 : index
        scf.for %arg1 = %c0_4 to %c128 step %c1_5 {
          %1 = aie.objectfifo.acquire @memC(Produce, 1) : !aie.objectfifosubview<memref<16x256xi32>>
          %2 = aie.objectfifo.subview.access %1[0] : !aie.objectfifosubview<memref<16x256xi32>> -> memref<16x256xi32>
          %collapse_shape = memref.collapse_shape %2 [[0, 1]] : memref<16x256xi32> into memref<4096xi32>
          func.call @zero_i32(%collapse_shape) : (memref<4096xi32>) -> ()
          %c0_6 = arith.constant 0 : index
          %c128_7 = arith.constant 128 : index
          %c1_8 = arith.constant 1 : index
          scf.for %arg2 = %c0_6 to %c128_7 step %c1_8 {
            %3 = aie.objectfifo.acquire @memA(Consume, 1) : !aie.objectfifosubview<memref<16x32xi8>>
            %4 = aie.objectfifo.subview.access %3[0] : !aie.objectfifosubview<memref<16x32xi8>> -> memref<16x32xi8>
            %5 = aie.objectfifo.acquire @memB(Consume, 1) : !aie.objectfifosubview<memref<32x256xi8>>
            %6 = aie.objectfifo.subview.access %5[0] : !aie.objectfifosubview<memref<32x256xi8>> -> memref<32x256xi8>
            %collapse_shape_9 = memref.collapse_shape %4 [[0, 1]] : memref<16x32xi8> into memref<512xi8>
            %collapse_shape_10 = memref.collapse_shape %6 [[0, 1]] : memref<32x256xi8> into memref<8192xi8>
            %collapse_shape_11 = memref.collapse_shape %2 [[0, 1]] : memref<16x256xi32> into memref<4096xi32>
            func.call @"58a6969e_matmul_i8_i32"(%collapse_shape_9, %collapse_shape_10, %collapse_shape_11) : (memref<512xi8>, memref<8192xi8>, memref<4096xi32>) -> ()
            aie.objectfifo.release @memA(Consume, 1)
            aie.objectfifo.release @memB(Consume, 1)
          }
          aie.objectfifo.release @memC(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    aie.runtime_sequence(%arg0: memref<131072xi8>, %arg1: memref<67108864xi8>, %arg2: memref<524288xi32>) {
      %1 = aiex.dma_configure_task_for @inA {
        aie.dma_bd(%arg0 : memref<131072xi8>, 0, 65536, [<size = 64, stride = 0>, <size = 128, stride = 32>, <size = 16, stride = 4096>, <size = 32, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 63 : i32}
      aiex.dma_start_task(%1)
      %2 = aiex.dma_configure_task_for @inB {
        aie.dma_bd(%arg1 : memref<67108864xi8>, 0, 1048576, [<size = 64, stride = 1048576>, <size = 128, stride = 32>, <size = 256, stride = 4096>, <size = 32, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 63 : i32}
      aiex.dma_start_task(%2)
      %3 = aiex.dma_configure_task_for @inA {
        aie.dma_bd(%arg0 : memref<131072xi8>, 65536, 65536, [<size = 64, stride = 0>, <size = 128, stride = 32>, <size = 16, stride = 4096>, <size = 32, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 63 : i32}
      aiex.dma_start_task(%3)
      %4 = aiex.dma_configure_task_for @inB {
        aie.dma_bd(%arg1 : memref<67108864xi8>, 0, 1048576, [<size = 64, stride = 1048576>, <size = 128, stride = 32>, <size = 256, stride = 4096>, <size = 32, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 63 : i32}
      aiex.dma_start_task(%4)
      %5 = aiex.dma_configure_task_for @outC {
        aie.dma_bd(%arg2 : memref<524288xi32>, 0, 262144, [<size = 2, stride = 262144>, <size = 64, stride = 256>, <size = 16, stride = 16384>, <size = 256, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%5)
      aiex.dma_await_task(%5)
      aiex.dma_free_task(%1)
      aiex.dma_free_task(%2)
      aiex.dma_free_task(%3)
      aiex.dma_free_task(%4)
    }
  }
}
