module {
  aie.device(npu2_1col) {
    %mem_tile_0_1 = aie.tile(0, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %shim_noc_tile_0_0 = aie.tile(0, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %tile_0_2 = aie.tile(0, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %outC_cons_prod_lock_0 = aie.lock(%shim_noc_tile_0_0, 4) {init = 0 : i32, sym_name = "outC_cons_prod_lock_0"}
    %outC_cons_cons_lock_0 = aie.lock(%shim_noc_tile_0_0, 5) {init = 0 : i32, sym_name = "outC_cons_cons_lock_0"}
    %inA_cons_buff_0 = aie.buffer(%mem_tile_0_1) {address = 16384 : i32, sym_name = "inA_cons_buff_0"} : memref<16x64xi8> 
    %inA_cons_buff_1 = aie.buffer(%mem_tile_0_1) {address = 17408 : i32, sym_name = "inA_cons_buff_1"} : memref<16x64xi8> 
    %inA_cons_prod_lock_0 = aie.lock(%mem_tile_0_1, 4) {init = 2 : i32, sym_name = "inA_cons_prod_lock_0"}
    %inA_cons_cons_lock_0 = aie.lock(%mem_tile_0_1, 5) {init = 0 : i32, sym_name = "inA_cons_cons_lock_0"}
    %memC_buff_0 = aie.buffer(%tile_0_2) {address = 3328 : i32, sym_name = "memC_buff_0"} : memref<16x64xi32> 
    %memC_buff_1 = aie.buffer(%tile_0_2) {address = 7424 : i32, sym_name = "memC_buff_1"} : memref<16x64xi32> 
    %memC_prod_lock_0 = aie.lock(%tile_0_2, 4) {init = 2 : i32, sym_name = "memC_prod_lock_0"}
    %memC_cons_lock_0 = aie.lock(%tile_0_2, 5) {init = 0 : i32, sym_name = "memC_cons_lock_0"}
    %memB_cons_buff_0 = aie.buffer(%tile_0_2) {address = 11520 : i32, sym_name = "memB_cons_buff_0"} : memref<64x64xi8> 
    %memB_cons_buff_1 = aie.buffer(%tile_0_2) {address = 15616 : i32, sym_name = "memB_cons_buff_1"} : memref<64x64xi8> 
    %memB_cons_prod_lock_0 = aie.lock(%tile_0_2, 2) {init = 2 : i32, sym_name = "memB_cons_prod_lock_0"}
    %memB_cons_cons_lock_0 = aie.lock(%tile_0_2, 3) {init = 0 : i32, sym_name = "memB_cons_cons_lock_0"}
    %memC_cons_buff_0 = aie.buffer(%mem_tile_0_1) {address = 0 : i32, sym_name = "memC_cons_buff_0"} : memref<16x64xi32> 
    %memC_cons_buff_1 = aie.buffer(%mem_tile_0_1) {address = 4096 : i32, sym_name = "memC_cons_buff_1"} : memref<16x64xi32> 
    %memC_cons_prod_lock_0 = aie.lock(%mem_tile_0_1, 2) {init = 2 : i32, sym_name = "memC_cons_prod_lock_0"}
    %memC_cons_cons_lock_0 = aie.lock(%mem_tile_0_1, 3) {init = 0 : i32, sym_name = "memC_cons_cons_lock_0"}
    %inB_prod_lock_0 = aie.lock(%shim_noc_tile_0_0, 2) {init = 0 : i32, sym_name = "inB_prod_lock_0"}
    %inB_cons_lock_0 = aie.lock(%shim_noc_tile_0_0, 3) {init = 0 : i32, sym_name = "inB_cons_lock_0"}
    %memA_cons_buff_0 = aie.buffer(%tile_0_2) {address = 19712 : i32, sym_name = "memA_cons_buff_0"} : memref<16x64xi8> 
    %memA_cons_buff_1 = aie.buffer(%tile_0_2) {address = 20736 : i32, sym_name = "memA_cons_buff_1"} : memref<16x64xi8> 
    %memA_cons_prod_lock_0 = aie.lock(%tile_0_2, 0) {init = 2 : i32, sym_name = "memA_cons_prod_lock_0"}
    %memA_cons_cons_lock_0 = aie.lock(%tile_0_2, 1) {init = 0 : i32, sym_name = "memA_cons_cons_lock_0"}
    %inB_cons_buff_0 = aie.buffer(%mem_tile_0_1) {address = 8192 : i32, sym_name = "inB_cons_buff_0"} : memref<64x64xi8> 
    %inB_cons_buff_1 = aie.buffer(%mem_tile_0_1) {address = 12288 : i32, sym_name = "inB_cons_buff_1"} : memref<64x64xi8> 
    %inB_cons_prod_lock_0 = aie.lock(%mem_tile_0_1, 0) {init = 2 : i32, sym_name = "inB_cons_prod_lock_0"}
    %inB_cons_cons_lock_0 = aie.lock(%mem_tile_0_1, 1) {init = 0 : i32, sym_name = "inB_cons_cons_lock_0"}
    %inA_prod_lock_0 = aie.lock(%shim_noc_tile_0_0, 0) {init = 0 : i32, sym_name = "inA_prod_lock_0"}
    %inA_cons_lock_0 = aie.lock(%shim_noc_tile_0_0, 1) {init = 0 : i32, sym_name = "inA_cons_lock_0"}
    aie.flow(%shim_noc_tile_0_0, DMA : 0, %mem_tile_0_1, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_0_2, DMA : 0)
    aie.flow(%shim_noc_tile_0_0, DMA : 1, %mem_tile_0_1, DMA : 1)
    aie.flow(%mem_tile_0_1, DMA : 1, %tile_0_2, DMA : 1)
    aie.flow(%tile_0_2, DMA : 0, %mem_tile_0_1, DMA : 2)
    aie.flow(%mem_tile_0_1, DMA : 2, %shim_noc_tile_0_0, DMA : 0)
    func.func private @zero_i32(memref<1024xi32>) attributes {link_with = "matmul_i8_i32_e5b22372.o"}
    func.func private @e5b22372_matmul_i8_i32(memref<1024xi8>, memref<4096xi8>, memref<1024xi32>) attributes {link_with = "matmul_i8_i32_e5b22372.o"}
    %_anonymous0 = aie.buffer(%tile_0_2) {address = 21760 : i32, sym_name = "_anonymous0"} : memref<3xi32> 
    %core_0_2 = aie.core(%tile_0_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c64 = arith.constant 64 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous0[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous0[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous0[%c2] : memref<3xi32>
      cf.br ^bb1(%c0 : index)
    ^bb1(%0: index):  // 2 preds: ^bb0, ^bb20
      %1 = arith.cmpi slt, %0, %c9223372036854775807 : index
      cf.cond_br %1, ^bb2, ^bb21
    ^bb2:  // pred: ^bb1
      cf.br ^bb3(%c0 : index)
    ^bb3(%2: index):  // 2 preds: ^bb2, ^bb19
      %3 = arith.cmpi slt, %2, %c16 : index
      cf.cond_br %3, ^bb4, ^bb20
    ^bb4:  // pred: ^bb3
      aie.use_lock(%memC_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous0[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%memC_buff_0 : memref<16x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%memC_buff_1 : memref<16x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%memC_buff_0 : memref<16x64xi32>)
    ^bb8(%7: memref<16x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<16x64xi32> into memref<1024xi32>
      func.call @zero_i32(%collapse_shape) : (memref<1024xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c64 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%memA_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous0[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%memA_cons_buff_0 : memref<16x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%memA_cons_buff_1 : memref<16x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%memA_cons_buff_0 : memref<16x64xi8>)
    ^bb14(%13: memref<16x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%memB_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous0[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%memB_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%memB_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%memB_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<16x64xi8> into memref<1024xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @e5b22372_matmul_i8_i32(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<1024xi8>, memref<4096xi8>, memref<1024xi32>) -> ()
      aie.use_lock(%memA_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous0[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous0[%c1] : memref<3xi32>
      aie.use_lock(%memB_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous0[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous0[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%memC_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous0[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous0[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_e5b22372.o"], stack_size = 3328 : i32}
    aie.runtime_sequence(%arg0: memref<131072xi8>, %arg1: memref<2097152xi8>, %arg2: memref<16384xi32>) {
      %0 = aiex.dma_configure_task_for @inA_shim_alloc {
        aie.dma_bd(%arg0 : memref<131072xi8>, 0, 65536, [<size = 8, stride = 0>, <size = 64, stride = 64>, <size = 16, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%0)
      %1 = aiex.dma_configure_task_for @inB_shim_alloc {
        aie.dma_bd(%arg1 : memref<2097152xi8>, 0, 262144, [<size = 8, stride = 262144>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%1)
      %2 = aiex.dma_configure_task_for @inA_shim_alloc {
        aie.dma_bd(%arg0 : memref<131072xi8>, 65536, 65536, [<size = 8, stride = 0>, <size = 64, stride = 64>, <size = 16, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%2)
      %3 = aiex.dma_configure_task_for @inB_shim_alloc {
        aie.dma_bd(%arg1 : memref<2097152xi8>, 0, 262144, [<size = 8, stride = 262144>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%3)
      %4 = aiex.dma_configure_task_for @outC_shim_alloc {
        aie.dma_bd(%arg2 : memref<16384xi32>, 0, 8192, [<size = 2, stride = 8192>, <size = 8, stride = 64>, <size = 16, stride = 512>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%4)
      aiex.dma_await_task(%4)
      aiex.dma_free_task(%0)
      aiex.dma_free_task(%1)
      aiex.dma_free_task(%2)
      aiex.dma_free_task(%3)
    }
    aie.shim_dma_allocation @inA_shim_alloc(%shim_noc_tile_0_0, MM2S, 0)
    %memtile_dma_0_1 = aie.memtile_dma(%mem_tile_0_1) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%inA_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inA_cons_buff_0 : memref<16x64xi8>, 0, 1024) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%inA_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%inA_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inA_cons_buff_1 : memref<16x64xi8>, 0, 1024) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%inA_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(MM2S, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%inA_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inA_cons_buff_0 : memref<16x64xi8>, 0, 1024, [<size = 2, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%inA_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%inA_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inA_cons_buff_1 : memref<16x64xi8>, 0, 1024, [<size = 2, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%inA_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(S2MM, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%inB_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inB_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%inB_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%inB_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inB_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%inB_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(MM2S, 1, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%inB_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inB_cons_buff_0 : memref<64x64xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%inB_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%inB_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%inB_cons_buff_1 : memref<64x64xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%inB_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 2, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%memC_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memC_cons_buff_0 : memref<16x64xi32>, 0, 1024) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%memC_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%memC_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memC_cons_buff_1 : memref<16x64xi32>, 0, 1024) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%memC_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(MM2S, 2, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%memC_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memC_cons_buff_0 : memref<16x64xi32>, 0, 1024, [<size = 2, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%memC_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%memC_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memC_cons_buff_1 : memref<16x64xi32>, 0, 1024, [<size = 2, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%memC_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      aie.end
    }
    %mem_0_2 = aie.mem(%tile_0_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%memA_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memA_cons_buff_0 : memref<16x64xi8>, 0, 1024) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%memA_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%memA_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memA_cons_buff_1 : memref<16x64xi8>, 0, 1024) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%memA_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%memB_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memB_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%memB_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%memB_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memB_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%memB_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%memC_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memC_buff_0 : memref<16x64xi32>, 0, 1024) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%memC_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%memC_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%memC_buff_1 : memref<16x64xi32>, 0, 1024) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%memC_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    aie.shim_dma_allocation @inB_shim_alloc(%shim_noc_tile_0_0, MM2S, 1)
    aie.shim_dma_allocation @outC_shim_alloc(%shim_noc_tile_0_0, S2MM, 0)
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_0_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_0_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
  }
}
