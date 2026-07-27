module {
  aie.device(npu2) {
    %mem_tile_7_1 = aie.tile(7, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %shim_noc_tile_1_0 = aie.tile(1, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %shim_noc_tile_5_0 = aie.tile(5, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %shim_noc_tile_3_0 = aie.tile(3, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %mem_tile_1_1 = aie.tile(1, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %mem_tile_5_1 = aie.tile(5, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %mem_tile_3_1 = aie.tile(3, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %shim_noc_tile_6_0 = aie.tile(6, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %shim_noc_tile_4_0 = aie.tile(4, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %shim_noc_tile_2_0 = aie.tile(2, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %shim_noc_tile_0_0 = aie.tile(0, 0) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 15>}
    %mem_tile_6_1 = aie.tile(6, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %mem_tile_4_1 = aie.tile(4, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %mem_tile_2_1 = aie.tile(2, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %mem_tile_0_1 = aie.tile(0, 1) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 26>}
    %tile_7_5 = aie.tile(7, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_7_4 = aie.tile(7, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_7_3 = aie.tile(7, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_7_2 = aie.tile(7, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %tile_6_5 = aie.tile(6, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_6_4 = aie.tile(6, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_6_3 = aie.tile(6, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_6_2 = aie.tile(6, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %tile_5_5 = aie.tile(5, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_5_4 = aie.tile(5, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_5_3 = aie.tile(5, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_5_2 = aie.tile(5, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %tile_4_5 = aie.tile(4, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_4_4 = aie.tile(4, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_4_3 = aie.tile(4, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_4_2 = aie.tile(4, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %tile_3_5 = aie.tile(3, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_3_4 = aie.tile(3, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_3_3 = aie.tile(3, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_3_2 = aie.tile(3, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %tile_2_5 = aie.tile(2, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_2_4 = aie.tile(2, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_2_3 = aie.tile(2, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_2_2 = aie.tile(2, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %tile_1_5 = aie.tile(1, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_1_4 = aie.tile(1, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_1_3 = aie.tile(1, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_1_2 = aie.tile(1, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %tile_0_5 = aie.tile(0, 5) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 31>}
    %tile_0_4 = aie.tile(0, 4) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 30>}
    %tile_0_3 = aie.tile(0, 3) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 29>}
    %tile_0_2 = aie.tile(0, 2) {controller_id = #aie.packet_info<pkt_type = 0, pkt_id = 27>}
    %C_L2L3_7_cons_prod_lock_0 = aie.lock(%shim_noc_tile_6_0, 4) {init = 0 : i32, sym_name = "C_L2L3_7_cons_prod_lock_0"}
    %C_L2L3_7_cons_cons_lock_0 = aie.lock(%shim_noc_tile_6_0, 5) {init = 0 : i32, sym_name = "C_L2L3_7_cons_cons_lock_0"}
    %A_L3L2_3_cons_buff_0 = aie.buffer(%mem_tile_6_1) {address = 73728 : i32, sym_name = "A_L3L2_3_cons_buff_0"} : memref<2048xi8> 
    %A_L3L2_3_cons_buff_1 = aie.buffer(%mem_tile_6_1) {address = 75776 : i32, sym_name = "A_L3L2_3_cons_buff_1"} : memref<2048xi8> 
    %A_L3L2_3_cons_prod_lock_0 = aie.lock(%mem_tile_6_1, 10) {init = 2 : i32, sym_name = "A_L3L2_3_cons_prod_lock_0"}
    %A_L3L2_3_cons_cons_lock_0 = aie.lock(%mem_tile_6_1, 11) {init = 0 : i32, sym_name = "A_L3L2_3_cons_cons_lock_0"}
    %C_L1L2_7_3_buff_0 = aie.buffer(%tile_7_5) {address = 3328 : i32, sym_name = "C_L1L2_7_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_7_3_buff_1 = aie.buffer(%tile_7_5) {address = 11520 : i32, sym_name = "C_L1L2_7_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_7_3_prod_lock_0 = aie.lock(%tile_7_5, 4) {init = 2 : i32, sym_name = "C_L1L2_7_3_prod_lock_0"}
    %C_L1L2_7_3_cons_lock_0 = aie.lock(%tile_7_5, 5) {init = 0 : i32, sym_name = "C_L1L2_7_3_cons_lock_0"}
    %A_L3L2_2_cons_buff_0 = aie.buffer(%mem_tile_4_1) {address = 73728 : i32, sym_name = "A_L3L2_2_cons_buff_0"} : memref<2048xi8> 
    %A_L3L2_2_cons_buff_1 = aie.buffer(%mem_tile_4_1) {address = 75776 : i32, sym_name = "A_L3L2_2_cons_buff_1"} : memref<2048xi8> 
    %A_L3L2_2_cons_prod_lock_0 = aie.lock(%mem_tile_4_1, 10) {init = 2 : i32, sym_name = "A_L3L2_2_cons_prod_lock_0"}
    %A_L3L2_2_cons_cons_lock_0 = aie.lock(%mem_tile_4_1, 11) {init = 0 : i32, sym_name = "A_L3L2_2_cons_cons_lock_0"}
    %C_L1L2_7_2_buff_0 = aie.buffer(%tile_5_5) {address = 3328 : i32, sym_name = "C_L1L2_7_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_7_2_buff_1 = aie.buffer(%tile_5_5) {address = 11520 : i32, sym_name = "C_L1L2_7_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_7_2_prod_lock_0 = aie.lock(%tile_5_5, 4) {init = 2 : i32, sym_name = "C_L1L2_7_2_prod_lock_0"}
    %C_L1L2_7_2_cons_lock_0 = aie.lock(%tile_5_5, 5) {init = 0 : i32, sym_name = "C_L1L2_7_2_cons_lock_0"}
    %C_L1L2_7_1_buff_0 = aie.buffer(%tile_3_5) {address = 3328 : i32, sym_name = "C_L1L2_7_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_7_1_buff_1 = aie.buffer(%tile_3_5) {address = 11520 : i32, sym_name = "C_L1L2_7_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_7_1_prod_lock_0 = aie.lock(%tile_3_5, 4) {init = 2 : i32, sym_name = "C_L1L2_7_1_prod_lock_0"}
    %C_L1L2_7_1_cons_lock_0 = aie.lock(%tile_3_5, 5) {init = 0 : i32, sym_name = "C_L1L2_7_1_cons_lock_0"}
    %A_L3L2_1_cons_buff_0 = aie.buffer(%mem_tile_2_1) {address = 73728 : i32, sym_name = "A_L3L2_1_cons_buff_0"} : memref<2048xi8> 
    %A_L3L2_1_cons_buff_1 = aie.buffer(%mem_tile_2_1) {address = 75776 : i32, sym_name = "A_L3L2_1_cons_buff_1"} : memref<2048xi8> 
    %A_L3L2_1_cons_prod_lock_0 = aie.lock(%mem_tile_2_1, 10) {init = 2 : i32, sym_name = "A_L3L2_1_cons_prod_lock_0"}
    %A_L3L2_1_cons_cons_lock_0 = aie.lock(%mem_tile_2_1, 11) {init = 0 : i32, sym_name = "A_L3L2_1_cons_cons_lock_0"}
    %C_L1L2_7_0_buff_0 = aie.buffer(%tile_1_5) {address = 3328 : i32, sym_name = "C_L1L2_7_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_7_0_buff_1 = aie.buffer(%tile_1_5) {address = 11520 : i32, sym_name = "C_L1L2_7_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_7_0_prod_lock_0 = aie.lock(%tile_1_5, 4) {init = 2 : i32, sym_name = "C_L1L2_7_0_prod_lock_0"}
    %C_L1L2_7_0_cons_lock_0 = aie.lock(%tile_1_5, 5) {init = 0 : i32, sym_name = "C_L1L2_7_0_cons_lock_0"}
    %C_L2L3_6_cons_prod_lock_0 = aie.lock(%shim_noc_tile_5_0, 6) {init = 0 : i32, sym_name = "C_L2L3_6_cons_prod_lock_0"}
    %C_L2L3_6_cons_cons_lock_0 = aie.lock(%shim_noc_tile_5_0, 7) {init = 0 : i32, sym_name = "C_L2L3_6_cons_cons_lock_0"}
    %A_L3L2_0_cons_buff_0 = aie.buffer(%mem_tile_0_1) {address = 65536 : i32, sym_name = "A_L3L2_0_cons_buff_0"} : memref<2048xi8> 
    %A_L3L2_0_cons_buff_1 = aie.buffer(%mem_tile_0_1) {address = 67584 : i32, sym_name = "A_L3L2_0_cons_buff_1"} : memref<2048xi8> 
    %A_L3L2_0_cons_prod_lock_0 = aie.lock(%mem_tile_0_1, 8) {init = 2 : i32, sym_name = "A_L3L2_0_cons_prod_lock_0"}
    %A_L3L2_0_cons_cons_lock_0 = aie.lock(%mem_tile_0_1, 9) {init = 0 : i32, sym_name = "A_L3L2_0_cons_cons_lock_0"}
    %C_L1L2_6_3_buff_0 = aie.buffer(%tile_7_4) {address = 3328 : i32, sym_name = "C_L1L2_6_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_6_3_buff_1 = aie.buffer(%tile_7_4) {address = 11520 : i32, sym_name = "C_L1L2_6_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_6_3_prod_lock_0 = aie.lock(%tile_7_4, 4) {init = 2 : i32, sym_name = "C_L1L2_6_3_prod_lock_0"}
    %C_L1L2_6_3_cons_lock_0 = aie.lock(%tile_7_4, 5) {init = 0 : i32, sym_name = "C_L1L2_6_3_cons_lock_0"}
    %C_L1L2_6_2_buff_0 = aie.buffer(%tile_5_4) {address = 3328 : i32, sym_name = "C_L1L2_6_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_6_2_buff_1 = aie.buffer(%tile_5_4) {address = 11520 : i32, sym_name = "C_L1L2_6_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_6_2_prod_lock_0 = aie.lock(%tile_5_4, 4) {init = 2 : i32, sym_name = "C_L1L2_6_2_prod_lock_0"}
    %C_L1L2_6_2_cons_lock_0 = aie.lock(%tile_5_4, 5) {init = 0 : i32, sym_name = "C_L1L2_6_2_cons_lock_0"}
    %B_L3L2_7_cons_buff_0 = aie.buffer(%mem_tile_1_1) {address = 65536 : i32, sym_name = "B_L3L2_7_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_7_cons_buff_1 = aie.buffer(%mem_tile_1_1) {address = 69632 : i32, sym_name = "B_L3L2_7_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_7_cons_prod_lock_0 = aie.lock(%mem_tile_1_1, 8) {init = 2 : i32, sym_name = "B_L3L2_7_cons_prod_lock_0"}
    %B_L3L2_7_cons_cons_lock_0 = aie.lock(%mem_tile_1_1, 9) {init = 0 : i32, sym_name = "B_L3L2_7_cons_cons_lock_0"}
    %C_L1L2_6_1_buff_0 = aie.buffer(%tile_3_4) {address = 3328 : i32, sym_name = "C_L1L2_6_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_6_1_buff_1 = aie.buffer(%tile_3_4) {address = 11520 : i32, sym_name = "C_L1L2_6_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_6_1_prod_lock_0 = aie.lock(%tile_3_4, 4) {init = 2 : i32, sym_name = "C_L1L2_6_1_prod_lock_0"}
    %C_L1L2_6_1_cons_lock_0 = aie.lock(%tile_3_4, 5) {init = 0 : i32, sym_name = "C_L1L2_6_1_cons_lock_0"}
    %C_L1L2_6_0_buff_0 = aie.buffer(%tile_1_4) {address = 3328 : i32, sym_name = "C_L1L2_6_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_6_0_buff_1 = aie.buffer(%tile_1_4) {address = 11520 : i32, sym_name = "C_L1L2_6_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_6_0_prod_lock_0 = aie.lock(%tile_1_4, 4) {init = 2 : i32, sym_name = "C_L1L2_6_0_prod_lock_0"}
    %C_L1L2_6_0_cons_lock_0 = aie.lock(%tile_1_4, 5) {init = 0 : i32, sym_name = "C_L1L2_6_0_cons_lock_0"}
    %C_L2L3_5_cons_prod_lock_0 = aie.lock(%shim_noc_tile_5_0, 4) {init = 0 : i32, sym_name = "C_L2L3_5_cons_prod_lock_0"}
    %C_L2L3_5_cons_cons_lock_0 = aie.lock(%shim_noc_tile_5_0, 5) {init = 0 : i32, sym_name = "C_L2L3_5_cons_cons_lock_0"}
    %B_L3L2_6_cons_buff_0 = aie.buffer(%mem_tile_6_1) {address = 65536 : i32, sym_name = "B_L3L2_6_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_6_cons_buff_1 = aie.buffer(%mem_tile_6_1) {address = 69632 : i32, sym_name = "B_L3L2_6_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_6_cons_prod_lock_0 = aie.lock(%mem_tile_6_1, 8) {init = 2 : i32, sym_name = "B_L3L2_6_cons_prod_lock_0"}
    %B_L3L2_6_cons_cons_lock_0 = aie.lock(%mem_tile_6_1, 9) {init = 0 : i32, sym_name = "B_L3L2_6_cons_cons_lock_0"}
    %C_L1L2_5_3_buff_0 = aie.buffer(%tile_7_3) {address = 3328 : i32, sym_name = "C_L1L2_5_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_5_3_buff_1 = aie.buffer(%tile_7_3) {address = 11520 : i32, sym_name = "C_L1L2_5_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_5_3_prod_lock_0 = aie.lock(%tile_7_3, 4) {init = 2 : i32, sym_name = "C_L1L2_5_3_prod_lock_0"}
    %C_L1L2_5_3_cons_lock_0 = aie.lock(%tile_7_3, 5) {init = 0 : i32, sym_name = "C_L1L2_5_3_cons_lock_0"}
    %B_L3L2_5_cons_buff_0 = aie.buffer(%mem_tile_5_1) {address = 65536 : i32, sym_name = "B_L3L2_5_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_5_cons_buff_1 = aie.buffer(%mem_tile_5_1) {address = 69632 : i32, sym_name = "B_L3L2_5_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_5_cons_prod_lock_0 = aie.lock(%mem_tile_5_1, 10) {init = 2 : i32, sym_name = "B_L3L2_5_cons_prod_lock_0"}
    %B_L3L2_5_cons_cons_lock_0 = aie.lock(%mem_tile_5_1, 11) {init = 0 : i32, sym_name = "B_L3L2_5_cons_cons_lock_0"}
    %C_L1L2_5_2_buff_0 = aie.buffer(%tile_5_3) {address = 3328 : i32, sym_name = "C_L1L2_5_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_5_2_buff_1 = aie.buffer(%tile_5_3) {address = 11520 : i32, sym_name = "C_L1L2_5_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_5_2_prod_lock_0 = aie.lock(%tile_5_3, 4) {init = 2 : i32, sym_name = "C_L1L2_5_2_prod_lock_0"}
    %C_L1L2_5_2_cons_lock_0 = aie.lock(%tile_5_3, 5) {init = 0 : i32, sym_name = "C_L1L2_5_2_cons_lock_0"}
    %C_L1L2_5_1_buff_0 = aie.buffer(%tile_3_3) {address = 3328 : i32, sym_name = "C_L1L2_5_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_5_1_buff_1 = aie.buffer(%tile_3_3) {address = 11520 : i32, sym_name = "C_L1L2_5_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_5_1_prod_lock_0 = aie.lock(%tile_3_3, 4) {init = 2 : i32, sym_name = "C_L1L2_5_1_prod_lock_0"}
    %C_L1L2_5_1_cons_lock_0 = aie.lock(%tile_3_3, 5) {init = 0 : i32, sym_name = "C_L1L2_5_1_cons_lock_0"}
    %B_L3L2_4_cons_buff_0 = aie.buffer(%mem_tile_5_1) {address = 73728 : i32, sym_name = "B_L3L2_4_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_4_cons_buff_1 = aie.buffer(%mem_tile_5_1) {address = 77824 : i32, sym_name = "B_L3L2_4_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_4_cons_prod_lock_0 = aie.lock(%mem_tile_5_1, 8) {init = 2 : i32, sym_name = "B_L3L2_4_cons_prod_lock_0"}
    %B_L3L2_4_cons_cons_lock_0 = aie.lock(%mem_tile_5_1, 9) {init = 0 : i32, sym_name = "B_L3L2_4_cons_cons_lock_0"}
    %C_L1L2_5_0_buff_0 = aie.buffer(%tile_1_3) {address = 3328 : i32, sym_name = "C_L1L2_5_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_5_0_buff_1 = aie.buffer(%tile_1_3) {address = 11520 : i32, sym_name = "C_L1L2_5_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_5_0_prod_lock_0 = aie.lock(%tile_1_3, 4) {init = 2 : i32, sym_name = "C_L1L2_5_0_prod_lock_0"}
    %C_L1L2_5_0_cons_lock_0 = aie.lock(%tile_1_3, 5) {init = 0 : i32, sym_name = "C_L1L2_5_0_cons_lock_0"}
    %C_L2L3_4_cons_prod_lock_0 = aie.lock(%shim_noc_tile_4_0, 6) {init = 0 : i32, sym_name = "C_L2L3_4_cons_prod_lock_0"}
    %C_L2L3_4_cons_cons_lock_0 = aie.lock(%shim_noc_tile_4_0, 7) {init = 0 : i32, sym_name = "C_L2L3_4_cons_cons_lock_0"}
    %B_L3L2_3_cons_buff_0 = aie.buffer(%mem_tile_4_1) {address = 65536 : i32, sym_name = "B_L3L2_3_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_3_cons_buff_1 = aie.buffer(%mem_tile_4_1) {address = 69632 : i32, sym_name = "B_L3L2_3_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_3_cons_prod_lock_0 = aie.lock(%mem_tile_4_1, 8) {init = 2 : i32, sym_name = "B_L3L2_3_cons_prod_lock_0"}
    %B_L3L2_3_cons_cons_lock_0 = aie.lock(%mem_tile_4_1, 9) {init = 0 : i32, sym_name = "B_L3L2_3_cons_cons_lock_0"}
    %C_L1L2_4_3_buff_0 = aie.buffer(%tile_7_2) {address = 3328 : i32, sym_name = "C_L1L2_4_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_4_3_buff_1 = aie.buffer(%tile_7_2) {address = 11520 : i32, sym_name = "C_L1L2_4_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_4_3_prod_lock_0 = aie.lock(%tile_7_2, 4) {init = 2 : i32, sym_name = "C_L1L2_4_3_prod_lock_0"}
    %C_L1L2_4_3_cons_lock_0 = aie.lock(%tile_7_2, 5) {init = 0 : i32, sym_name = "C_L1L2_4_3_cons_lock_0"}
    %C_L1L2_4_2_buff_0 = aie.buffer(%tile_5_2) {address = 3328 : i32, sym_name = "C_L1L2_4_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_4_2_buff_1 = aie.buffer(%tile_5_2) {address = 11520 : i32, sym_name = "C_L1L2_4_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_4_2_prod_lock_0 = aie.lock(%tile_5_2, 4) {init = 2 : i32, sym_name = "C_L1L2_4_2_prod_lock_0"}
    %C_L1L2_4_2_cons_lock_0 = aie.lock(%tile_5_2, 5) {init = 0 : i32, sym_name = "C_L1L2_4_2_cons_lock_0"}
    %B_L3L2_2_cons_buff_0 = aie.buffer(%mem_tile_2_1) {address = 65536 : i32, sym_name = "B_L3L2_2_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_2_cons_buff_1 = aie.buffer(%mem_tile_2_1) {address = 69632 : i32, sym_name = "B_L3L2_2_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_2_cons_prod_lock_0 = aie.lock(%mem_tile_2_1, 8) {init = 2 : i32, sym_name = "B_L3L2_2_cons_prod_lock_0"}
    %B_L3L2_2_cons_cons_lock_0 = aie.lock(%mem_tile_2_1, 9) {init = 0 : i32, sym_name = "B_L3L2_2_cons_cons_lock_0"}
    %C_L1L2_4_1_buff_0 = aie.buffer(%tile_3_2) {address = 3328 : i32, sym_name = "C_L1L2_4_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_4_1_buff_1 = aie.buffer(%tile_3_2) {address = 11520 : i32, sym_name = "C_L1L2_4_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_4_1_prod_lock_0 = aie.lock(%tile_3_2, 4) {init = 2 : i32, sym_name = "C_L1L2_4_1_prod_lock_0"}
    %C_L1L2_4_1_cons_lock_0 = aie.lock(%tile_3_2, 5) {init = 0 : i32, sym_name = "C_L1L2_4_1_cons_lock_0"}
    %C_L1L2_4_0_buff_0 = aie.buffer(%tile_1_2) {address = 3328 : i32, sym_name = "C_L1L2_4_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_4_0_buff_1 = aie.buffer(%tile_1_2) {address = 11520 : i32, sym_name = "C_L1L2_4_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_4_0_prod_lock_0 = aie.lock(%tile_1_2, 4) {init = 2 : i32, sym_name = "C_L1L2_4_0_prod_lock_0"}
    %C_L1L2_4_0_cons_lock_0 = aie.lock(%tile_1_2, 5) {init = 0 : i32, sym_name = "C_L1L2_4_0_cons_lock_0"}
    %C_L2L3_3_cons_prod_lock_0 = aie.lock(%shim_noc_tile_4_0, 4) {init = 0 : i32, sym_name = "C_L2L3_3_cons_prod_lock_0"}
    %C_L2L3_3_cons_cons_lock_0 = aie.lock(%shim_noc_tile_4_0, 5) {init = 0 : i32, sym_name = "C_L2L3_3_cons_cons_lock_0"}
    %B_L3L2_1_cons_buff_0 = aie.buffer(%mem_tile_3_1) {address = 65536 : i32, sym_name = "B_L3L2_1_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_1_cons_buff_1 = aie.buffer(%mem_tile_3_1) {address = 69632 : i32, sym_name = "B_L3L2_1_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_1_cons_prod_lock_0 = aie.lock(%mem_tile_3_1, 10) {init = 2 : i32, sym_name = "B_L3L2_1_cons_prod_lock_0"}
    %B_L3L2_1_cons_cons_lock_0 = aie.lock(%mem_tile_3_1, 11) {init = 0 : i32, sym_name = "B_L3L2_1_cons_cons_lock_0"}
    %C_L1L2_3_3_buff_0 = aie.buffer(%tile_6_5) {address = 3328 : i32, sym_name = "C_L1L2_3_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_3_3_buff_1 = aie.buffer(%tile_6_5) {address = 11520 : i32, sym_name = "C_L1L2_3_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_3_3_prod_lock_0 = aie.lock(%tile_6_5, 4) {init = 2 : i32, sym_name = "C_L1L2_3_3_prod_lock_0"}
    %C_L1L2_3_3_cons_lock_0 = aie.lock(%tile_6_5, 5) {init = 0 : i32, sym_name = "C_L1L2_3_3_cons_lock_0"}
    %B_L3L2_0_cons_buff_0 = aie.buffer(%mem_tile_3_1) {address = 73728 : i32, sym_name = "B_L3L2_0_cons_buff_0"} : memref<4096xi8> 
    %B_L3L2_0_cons_buff_1 = aie.buffer(%mem_tile_3_1) {address = 77824 : i32, sym_name = "B_L3L2_0_cons_buff_1"} : memref<4096xi8> 
    %B_L3L2_0_cons_prod_lock_0 = aie.lock(%mem_tile_3_1, 8) {init = 2 : i32, sym_name = "B_L3L2_0_cons_prod_lock_0"}
    %B_L3L2_0_cons_cons_lock_0 = aie.lock(%mem_tile_3_1, 9) {init = 0 : i32, sym_name = "B_L3L2_0_cons_cons_lock_0"}
    %C_L1L2_3_2_buff_0 = aie.buffer(%tile_4_5) {address = 3328 : i32, sym_name = "C_L1L2_3_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_3_2_buff_1 = aie.buffer(%tile_4_5) {address = 11520 : i32, sym_name = "C_L1L2_3_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_3_2_prod_lock_0 = aie.lock(%tile_4_5, 4) {init = 2 : i32, sym_name = "C_L1L2_3_2_prod_lock_0"}
    %C_L1L2_3_2_cons_lock_0 = aie.lock(%tile_4_5, 5) {init = 0 : i32, sym_name = "C_L1L2_3_2_cons_lock_0"}
    %C_L1L2_3_1_buff_0 = aie.buffer(%tile_2_5) {address = 3328 : i32, sym_name = "C_L1L2_3_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_3_1_buff_1 = aie.buffer(%tile_2_5) {address = 11520 : i32, sym_name = "C_L1L2_3_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_3_1_prod_lock_0 = aie.lock(%tile_2_5, 4) {init = 2 : i32, sym_name = "C_L1L2_3_1_prod_lock_0"}
    %C_L1L2_3_1_cons_lock_0 = aie.lock(%tile_2_5, 5) {init = 0 : i32, sym_name = "C_L1L2_3_1_cons_lock_0"}
    %C_L1L2_3_0_buff_0 = aie.buffer(%tile_0_5) {address = 3328 : i32, sym_name = "C_L1L2_3_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_3_0_buff_1 = aie.buffer(%tile_0_5) {address = 11520 : i32, sym_name = "C_L1L2_3_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_3_0_prod_lock_0 = aie.lock(%tile_0_5, 4) {init = 2 : i32, sym_name = "C_L1L2_3_0_prod_lock_0"}
    %C_L1L2_3_0_cons_lock_0 = aie.lock(%tile_0_5, 5) {init = 0 : i32, sym_name = "C_L1L2_3_0_cons_lock_0"}
    %C_L2L3_2_cons_prod_lock_0 = aie.lock(%shim_noc_tile_2_0, 4) {init = 0 : i32, sym_name = "C_L2L3_2_cons_prod_lock_0"}
    %C_L2L3_2_cons_cons_lock_0 = aie.lock(%shim_noc_tile_2_0, 5) {init = 0 : i32, sym_name = "C_L2L3_2_cons_cons_lock_0"}
    %C_L1L2_2_3_buff_0 = aie.buffer(%tile_6_4) {address = 3328 : i32, sym_name = "C_L1L2_2_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_2_3_buff_1 = aie.buffer(%tile_6_4) {address = 11520 : i32, sym_name = "C_L1L2_2_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_2_3_prod_lock_0 = aie.lock(%tile_6_4, 4) {init = 2 : i32, sym_name = "C_L1L2_2_3_prod_lock_0"}
    %C_L1L2_2_3_cons_lock_0 = aie.lock(%tile_6_4, 5) {init = 0 : i32, sym_name = "C_L1L2_2_3_cons_lock_0"}
    %C_L1L2_2_2_buff_0 = aie.buffer(%tile_4_4) {address = 3328 : i32, sym_name = "C_L1L2_2_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_2_2_buff_1 = aie.buffer(%tile_4_4) {address = 11520 : i32, sym_name = "C_L1L2_2_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_2_2_prod_lock_0 = aie.lock(%tile_4_4, 4) {init = 2 : i32, sym_name = "C_L1L2_2_2_prod_lock_0"}
    %C_L1L2_2_2_cons_lock_0 = aie.lock(%tile_4_4, 5) {init = 0 : i32, sym_name = "C_L1L2_2_2_cons_lock_0"}
    %C_L1L2_2_1_buff_0 = aie.buffer(%tile_2_4) {address = 3328 : i32, sym_name = "C_L1L2_2_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_2_1_buff_1 = aie.buffer(%tile_2_4) {address = 11520 : i32, sym_name = "C_L1L2_2_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_2_1_prod_lock_0 = aie.lock(%tile_2_4, 4) {init = 2 : i32, sym_name = "C_L1L2_2_1_prod_lock_0"}
    %C_L1L2_2_1_cons_lock_0 = aie.lock(%tile_2_4, 5) {init = 0 : i32, sym_name = "C_L1L2_2_1_cons_lock_0"}
    %C_L1L2_2_0_buff_0 = aie.buffer(%tile_0_4) {address = 3328 : i32, sym_name = "C_L1L2_2_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_2_0_buff_1 = aie.buffer(%tile_0_4) {address = 11520 : i32, sym_name = "C_L1L2_2_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_2_0_prod_lock_0 = aie.lock(%tile_0_4, 4) {init = 2 : i32, sym_name = "C_L1L2_2_0_prod_lock_0"}
    %C_L1L2_2_0_cons_lock_0 = aie.lock(%tile_0_4, 5) {init = 0 : i32, sym_name = "C_L1L2_2_0_cons_lock_0"}
    %C_L2L3_1_cons_prod_lock_0 = aie.lock(%shim_noc_tile_3_0, 6) {init = 0 : i32, sym_name = "C_L2L3_1_cons_prod_lock_0"}
    %C_L2L3_1_cons_cons_lock_0 = aie.lock(%shim_noc_tile_3_0, 7) {init = 0 : i32, sym_name = "C_L2L3_1_cons_cons_lock_0"}
    %C_L1L2_1_3_buff_0 = aie.buffer(%tile_6_3) {address = 3328 : i32, sym_name = "C_L1L2_1_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_1_3_buff_1 = aie.buffer(%tile_6_3) {address = 11520 : i32, sym_name = "C_L1L2_1_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_1_3_prod_lock_0 = aie.lock(%tile_6_3, 4) {init = 2 : i32, sym_name = "C_L1L2_1_3_prod_lock_0"}
    %C_L1L2_1_3_cons_lock_0 = aie.lock(%tile_6_3, 5) {init = 0 : i32, sym_name = "C_L1L2_1_3_cons_lock_0"}
    %C_L1L2_1_2_buff_0 = aie.buffer(%tile_4_3) {address = 3328 : i32, sym_name = "C_L1L2_1_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_1_2_buff_1 = aie.buffer(%tile_4_3) {address = 11520 : i32, sym_name = "C_L1L2_1_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_1_2_prod_lock_0 = aie.lock(%tile_4_3, 4) {init = 2 : i32, sym_name = "C_L1L2_1_2_prod_lock_0"}
    %C_L1L2_1_2_cons_lock_0 = aie.lock(%tile_4_3, 5) {init = 0 : i32, sym_name = "C_L1L2_1_2_cons_lock_0"}
    %C_L1L2_1_1_buff_0 = aie.buffer(%tile_2_3) {address = 3328 : i32, sym_name = "C_L1L2_1_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_1_1_buff_1 = aie.buffer(%tile_2_3) {address = 11520 : i32, sym_name = "C_L1L2_1_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_1_1_prod_lock_0 = aie.lock(%tile_2_3, 4) {init = 2 : i32, sym_name = "C_L1L2_1_1_prod_lock_0"}
    %C_L1L2_1_1_cons_lock_0 = aie.lock(%tile_2_3, 5) {init = 0 : i32, sym_name = "C_L1L2_1_1_cons_lock_0"}
    %C_L1L2_1_0_buff_0 = aie.buffer(%tile_0_3) {address = 3328 : i32, sym_name = "C_L1L2_1_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_1_0_buff_1 = aie.buffer(%tile_0_3) {address = 11520 : i32, sym_name = "C_L1L2_1_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_1_0_prod_lock_0 = aie.lock(%tile_0_3, 4) {init = 2 : i32, sym_name = "C_L1L2_1_0_prod_lock_0"}
    %C_L1L2_1_0_cons_lock_0 = aie.lock(%tile_0_3, 5) {init = 0 : i32, sym_name = "C_L1L2_1_0_cons_lock_0"}
    %C_L2L3_0_cons_prod_lock_0 = aie.lock(%shim_noc_tile_3_0, 4) {init = 0 : i32, sym_name = "C_L2L3_0_cons_prod_lock_0"}
    %C_L2L3_0_cons_cons_lock_0 = aie.lock(%shim_noc_tile_3_0, 5) {init = 0 : i32, sym_name = "C_L2L3_0_cons_cons_lock_0"}
    %C_L1L2_0_3_buff_0 = aie.buffer(%tile_6_2) {address = 3328 : i32, sym_name = "C_L1L2_0_3_buff_0"} : memref<32x64xi32> 
    %C_L1L2_0_3_buff_1 = aie.buffer(%tile_6_2) {address = 11520 : i32, sym_name = "C_L1L2_0_3_buff_1"} : memref<32x64xi32> 
    %C_L1L2_0_3_prod_lock_0 = aie.lock(%tile_6_2, 4) {init = 2 : i32, sym_name = "C_L1L2_0_3_prod_lock_0"}
    %C_L1L2_0_3_cons_lock_0 = aie.lock(%tile_6_2, 5) {init = 0 : i32, sym_name = "C_L1L2_0_3_cons_lock_0"}
    %C_L1L2_0_2_buff_0 = aie.buffer(%tile_4_2) {address = 3328 : i32, sym_name = "C_L1L2_0_2_buff_0"} : memref<32x64xi32> 
    %C_L1L2_0_2_buff_1 = aie.buffer(%tile_4_2) {address = 11520 : i32, sym_name = "C_L1L2_0_2_buff_1"} : memref<32x64xi32> 
    %C_L1L2_0_2_prod_lock_0 = aie.lock(%tile_4_2, 4) {init = 2 : i32, sym_name = "C_L1L2_0_2_prod_lock_0"}
    %C_L1L2_0_2_cons_lock_0 = aie.lock(%tile_4_2, 5) {init = 0 : i32, sym_name = "C_L1L2_0_2_cons_lock_0"}
    %C_L1L2_0_1_buff_0 = aie.buffer(%tile_2_2) {address = 3328 : i32, sym_name = "C_L1L2_0_1_buff_0"} : memref<32x64xi32> 
    %C_L1L2_0_1_buff_1 = aie.buffer(%tile_2_2) {address = 11520 : i32, sym_name = "C_L1L2_0_1_buff_1"} : memref<32x64xi32> 
    %C_L1L2_0_1_prod_lock_0 = aie.lock(%tile_2_2, 4) {init = 2 : i32, sym_name = "C_L1L2_0_1_prod_lock_0"}
    %C_L1L2_0_1_cons_lock_0 = aie.lock(%tile_2_2, 5) {init = 0 : i32, sym_name = "C_L1L2_0_1_cons_lock_0"}
    %C_L1L2_0_0_buff_0 = aie.buffer(%tile_0_2) {address = 3328 : i32, sym_name = "C_L1L2_0_0_buff_0"} : memref<32x64xi32> 
    %C_L1L2_0_0_buff_1 = aie.buffer(%tile_0_2) {address = 11520 : i32, sym_name = "C_L1L2_0_0_buff_1"} : memref<32x64xi32> 
    %C_L1L2_0_0_prod_lock_0 = aie.lock(%tile_0_2, 4) {init = 2 : i32, sym_name = "C_L1L2_0_0_prod_lock_0"}
    %C_L1L2_0_0_cons_lock_0 = aie.lock(%tile_0_2, 5) {init = 0 : i32, sym_name = "C_L1L2_0_0_cons_lock_0"}
    %B_L3L2_7_prod_lock_0 = aie.lock(%shim_noc_tile_1_0, 0) {init = 0 : i32, sym_name = "B_L3L2_7_prod_lock_0"}
    %B_L3L2_7_cons_lock_0 = aie.lock(%shim_noc_tile_1_0, 1) {init = 0 : i32, sym_name = "B_L3L2_7_cons_lock_0"}
    %B_L2L1_7_0_cons_buff_0 = aie.buffer(%tile_1_5) {address = 19712 : i32, sym_name = "B_L2L1_7_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_7_0_cons_buff_1 = aie.buffer(%tile_1_5) {address = 23808 : i32, sym_name = "B_L2L1_7_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_7_0_cons_prod_lock_0 = aie.lock(%tile_1_5, 2) {init = 2 : i32, sym_name = "B_L2L1_7_0_cons_prod_lock_0"}
    %B_L2L1_7_0_cons_cons_lock_0 = aie.lock(%tile_1_5, 3) {init = 0 : i32, sym_name = "B_L2L1_7_0_cons_cons_lock_0"}
    %B_L2L1_7_1_cons_buff_0 = aie.buffer(%tile_3_5) {address = 19712 : i32, sym_name = "B_L2L1_7_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_7_1_cons_buff_1 = aie.buffer(%tile_3_5) {address = 23808 : i32, sym_name = "B_L2L1_7_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_7_1_cons_prod_lock_0 = aie.lock(%tile_3_5, 2) {init = 2 : i32, sym_name = "B_L2L1_7_1_cons_prod_lock_0"}
    %B_L2L1_7_1_cons_cons_lock_0 = aie.lock(%tile_3_5, 3) {init = 0 : i32, sym_name = "B_L2L1_7_1_cons_cons_lock_0"}
    %B_L2L1_7_2_cons_buff_0 = aie.buffer(%tile_5_5) {address = 19712 : i32, sym_name = "B_L2L1_7_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_7_2_cons_buff_1 = aie.buffer(%tile_5_5) {address = 23808 : i32, sym_name = "B_L2L1_7_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_7_2_cons_prod_lock_0 = aie.lock(%tile_5_5, 2) {init = 2 : i32, sym_name = "B_L2L1_7_2_cons_prod_lock_0"}
    %B_L2L1_7_2_cons_cons_lock_0 = aie.lock(%tile_5_5, 3) {init = 0 : i32, sym_name = "B_L2L1_7_2_cons_cons_lock_0"}
    %B_L2L1_7_3_cons_buff_0 = aie.buffer(%tile_7_5) {address = 19712 : i32, sym_name = "B_L2L1_7_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_7_3_cons_buff_1 = aie.buffer(%tile_7_5) {address = 23808 : i32, sym_name = "B_L2L1_7_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_7_3_cons_prod_lock_0 = aie.lock(%tile_7_5, 2) {init = 2 : i32, sym_name = "B_L2L1_7_3_cons_prod_lock_0"}
    %B_L2L1_7_3_cons_cons_lock_0 = aie.lock(%tile_7_5, 3) {init = 0 : i32, sym_name = "B_L2L1_7_3_cons_cons_lock_0"}
    %B_L3L2_6_prod_lock_0 = aie.lock(%shim_noc_tile_6_0, 2) {init = 0 : i32, sym_name = "B_L3L2_6_prod_lock_0"}
    %B_L3L2_6_cons_lock_0 = aie.lock(%shim_noc_tile_6_0, 3) {init = 0 : i32, sym_name = "B_L3L2_6_cons_lock_0"}
    %B_L2L1_6_0_cons_buff_0 = aie.buffer(%tile_1_4) {address = 19712 : i32, sym_name = "B_L2L1_6_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_6_0_cons_buff_1 = aie.buffer(%tile_1_4) {address = 23808 : i32, sym_name = "B_L2L1_6_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_6_0_cons_prod_lock_0 = aie.lock(%tile_1_4, 2) {init = 2 : i32, sym_name = "B_L2L1_6_0_cons_prod_lock_0"}
    %B_L2L1_6_0_cons_cons_lock_0 = aie.lock(%tile_1_4, 3) {init = 0 : i32, sym_name = "B_L2L1_6_0_cons_cons_lock_0"}
    %B_L2L1_6_1_cons_buff_0 = aie.buffer(%tile_3_4) {address = 19712 : i32, sym_name = "B_L2L1_6_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_6_1_cons_buff_1 = aie.buffer(%tile_3_4) {address = 23808 : i32, sym_name = "B_L2L1_6_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_6_1_cons_prod_lock_0 = aie.lock(%tile_3_4, 2) {init = 2 : i32, sym_name = "B_L2L1_6_1_cons_prod_lock_0"}
    %B_L2L1_6_1_cons_cons_lock_0 = aie.lock(%tile_3_4, 3) {init = 0 : i32, sym_name = "B_L2L1_6_1_cons_cons_lock_0"}
    %B_L2L1_6_2_cons_buff_0 = aie.buffer(%tile_5_4) {address = 19712 : i32, sym_name = "B_L2L1_6_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_6_2_cons_buff_1 = aie.buffer(%tile_5_4) {address = 23808 : i32, sym_name = "B_L2L1_6_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_6_2_cons_prod_lock_0 = aie.lock(%tile_5_4, 2) {init = 2 : i32, sym_name = "B_L2L1_6_2_cons_prod_lock_0"}
    %B_L2L1_6_2_cons_cons_lock_0 = aie.lock(%tile_5_4, 3) {init = 0 : i32, sym_name = "B_L2L1_6_2_cons_cons_lock_0"}
    %B_L2L1_6_3_cons_buff_0 = aie.buffer(%tile_7_4) {address = 19712 : i32, sym_name = "B_L2L1_6_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_6_3_cons_buff_1 = aie.buffer(%tile_7_4) {address = 23808 : i32, sym_name = "B_L2L1_6_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_6_3_cons_prod_lock_0 = aie.lock(%tile_7_4, 2) {init = 2 : i32, sym_name = "B_L2L1_6_3_cons_prod_lock_0"}
    %B_L2L1_6_3_cons_cons_lock_0 = aie.lock(%tile_7_4, 3) {init = 0 : i32, sym_name = "B_L2L1_6_3_cons_cons_lock_0"}
    %B_L3L2_5_prod_lock_0 = aie.lock(%shim_noc_tile_5_0, 2) {init = 0 : i32, sym_name = "B_L3L2_5_prod_lock_0"}
    %B_L3L2_5_cons_lock_0 = aie.lock(%shim_noc_tile_5_0, 3) {init = 0 : i32, sym_name = "B_L3L2_5_cons_lock_0"}
    %B_L2L1_5_0_cons_buff_0 = aie.buffer(%tile_1_3) {address = 19712 : i32, sym_name = "B_L2L1_5_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_5_0_cons_buff_1 = aie.buffer(%tile_1_3) {address = 23808 : i32, sym_name = "B_L2L1_5_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_5_0_cons_prod_lock_0 = aie.lock(%tile_1_3, 2) {init = 2 : i32, sym_name = "B_L2L1_5_0_cons_prod_lock_0"}
    %B_L2L1_5_0_cons_cons_lock_0 = aie.lock(%tile_1_3, 3) {init = 0 : i32, sym_name = "B_L2L1_5_0_cons_cons_lock_0"}
    %B_L2L1_5_1_cons_buff_0 = aie.buffer(%tile_3_3) {address = 19712 : i32, sym_name = "B_L2L1_5_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_5_1_cons_buff_1 = aie.buffer(%tile_3_3) {address = 23808 : i32, sym_name = "B_L2L1_5_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_5_1_cons_prod_lock_0 = aie.lock(%tile_3_3, 2) {init = 2 : i32, sym_name = "B_L2L1_5_1_cons_prod_lock_0"}
    %B_L2L1_5_1_cons_cons_lock_0 = aie.lock(%tile_3_3, 3) {init = 0 : i32, sym_name = "B_L2L1_5_1_cons_cons_lock_0"}
    %B_L2L1_5_2_cons_buff_0 = aie.buffer(%tile_5_3) {address = 19712 : i32, sym_name = "B_L2L1_5_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_5_2_cons_buff_1 = aie.buffer(%tile_5_3) {address = 23808 : i32, sym_name = "B_L2L1_5_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_5_2_cons_prod_lock_0 = aie.lock(%tile_5_3, 2) {init = 2 : i32, sym_name = "B_L2L1_5_2_cons_prod_lock_0"}
    %B_L2L1_5_2_cons_cons_lock_0 = aie.lock(%tile_5_3, 3) {init = 0 : i32, sym_name = "B_L2L1_5_2_cons_cons_lock_0"}
    %B_L2L1_5_3_cons_buff_0 = aie.buffer(%tile_7_3) {address = 19712 : i32, sym_name = "B_L2L1_5_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_5_3_cons_buff_1 = aie.buffer(%tile_7_3) {address = 23808 : i32, sym_name = "B_L2L1_5_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_5_3_cons_prod_lock_0 = aie.lock(%tile_7_3, 2) {init = 2 : i32, sym_name = "B_L2L1_5_3_cons_prod_lock_0"}
    %B_L2L1_5_3_cons_cons_lock_0 = aie.lock(%tile_7_3, 3) {init = 0 : i32, sym_name = "B_L2L1_5_3_cons_cons_lock_0"}
    %B_L3L2_4_prod_lock_0 = aie.lock(%shim_noc_tile_5_0, 0) {init = 0 : i32, sym_name = "B_L3L2_4_prod_lock_0"}
    %B_L3L2_4_cons_lock_0 = aie.lock(%shim_noc_tile_5_0, 1) {init = 0 : i32, sym_name = "B_L3L2_4_cons_lock_0"}
    %B_L2L1_4_0_cons_buff_0 = aie.buffer(%tile_1_2) {address = 19712 : i32, sym_name = "B_L2L1_4_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_4_0_cons_buff_1 = aie.buffer(%tile_1_2) {address = 23808 : i32, sym_name = "B_L2L1_4_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_4_0_cons_prod_lock_0 = aie.lock(%tile_1_2, 2) {init = 2 : i32, sym_name = "B_L2L1_4_0_cons_prod_lock_0"}
    %B_L2L1_4_0_cons_cons_lock_0 = aie.lock(%tile_1_2, 3) {init = 0 : i32, sym_name = "B_L2L1_4_0_cons_cons_lock_0"}
    %B_L2L1_4_1_cons_buff_0 = aie.buffer(%tile_3_2) {address = 19712 : i32, sym_name = "B_L2L1_4_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_4_1_cons_buff_1 = aie.buffer(%tile_3_2) {address = 23808 : i32, sym_name = "B_L2L1_4_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_4_1_cons_prod_lock_0 = aie.lock(%tile_3_2, 2) {init = 2 : i32, sym_name = "B_L2L1_4_1_cons_prod_lock_0"}
    %B_L2L1_4_1_cons_cons_lock_0 = aie.lock(%tile_3_2, 3) {init = 0 : i32, sym_name = "B_L2L1_4_1_cons_cons_lock_0"}
    %B_L2L1_4_2_cons_buff_0 = aie.buffer(%tile_5_2) {address = 19712 : i32, sym_name = "B_L2L1_4_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_4_2_cons_buff_1 = aie.buffer(%tile_5_2) {address = 23808 : i32, sym_name = "B_L2L1_4_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_4_2_cons_prod_lock_0 = aie.lock(%tile_5_2, 2) {init = 2 : i32, sym_name = "B_L2L1_4_2_cons_prod_lock_0"}
    %B_L2L1_4_2_cons_cons_lock_0 = aie.lock(%tile_5_2, 3) {init = 0 : i32, sym_name = "B_L2L1_4_2_cons_cons_lock_0"}
    %B_L2L1_4_3_cons_buff_0 = aie.buffer(%tile_7_2) {address = 19712 : i32, sym_name = "B_L2L1_4_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_4_3_cons_buff_1 = aie.buffer(%tile_7_2) {address = 23808 : i32, sym_name = "B_L2L1_4_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_4_3_cons_prod_lock_0 = aie.lock(%tile_7_2, 2) {init = 2 : i32, sym_name = "B_L2L1_4_3_cons_prod_lock_0"}
    %B_L2L1_4_3_cons_cons_lock_0 = aie.lock(%tile_7_2, 3) {init = 0 : i32, sym_name = "B_L2L1_4_3_cons_cons_lock_0"}
    %B_L3L2_3_prod_lock_0 = aie.lock(%shim_noc_tile_4_0, 2) {init = 0 : i32, sym_name = "B_L3L2_3_prod_lock_0"}
    %B_L3L2_3_cons_lock_0 = aie.lock(%shim_noc_tile_4_0, 3) {init = 0 : i32, sym_name = "B_L3L2_3_cons_lock_0"}
    %B_L2L1_3_0_cons_buff_0 = aie.buffer(%tile_0_5) {address = 19712 : i32, sym_name = "B_L2L1_3_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_3_0_cons_buff_1 = aie.buffer(%tile_0_5) {address = 23808 : i32, sym_name = "B_L2L1_3_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_3_0_cons_prod_lock_0 = aie.lock(%tile_0_5, 2) {init = 2 : i32, sym_name = "B_L2L1_3_0_cons_prod_lock_0"}
    %B_L2L1_3_0_cons_cons_lock_0 = aie.lock(%tile_0_5, 3) {init = 0 : i32, sym_name = "B_L2L1_3_0_cons_cons_lock_0"}
    %B_L2L1_3_1_cons_buff_0 = aie.buffer(%tile_2_5) {address = 19712 : i32, sym_name = "B_L2L1_3_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_3_1_cons_buff_1 = aie.buffer(%tile_2_5) {address = 23808 : i32, sym_name = "B_L2L1_3_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_3_1_cons_prod_lock_0 = aie.lock(%tile_2_5, 2) {init = 2 : i32, sym_name = "B_L2L1_3_1_cons_prod_lock_0"}
    %B_L2L1_3_1_cons_cons_lock_0 = aie.lock(%tile_2_5, 3) {init = 0 : i32, sym_name = "B_L2L1_3_1_cons_cons_lock_0"}
    %B_L2L1_3_2_cons_buff_0 = aie.buffer(%tile_4_5) {address = 19712 : i32, sym_name = "B_L2L1_3_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_3_2_cons_buff_1 = aie.buffer(%tile_4_5) {address = 23808 : i32, sym_name = "B_L2L1_3_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_3_2_cons_prod_lock_0 = aie.lock(%tile_4_5, 2) {init = 2 : i32, sym_name = "B_L2L1_3_2_cons_prod_lock_0"}
    %B_L2L1_3_2_cons_cons_lock_0 = aie.lock(%tile_4_5, 3) {init = 0 : i32, sym_name = "B_L2L1_3_2_cons_cons_lock_0"}
    %B_L2L1_3_3_cons_buff_0 = aie.buffer(%tile_6_5) {address = 19712 : i32, sym_name = "B_L2L1_3_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_3_3_cons_buff_1 = aie.buffer(%tile_6_5) {address = 23808 : i32, sym_name = "B_L2L1_3_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_3_3_cons_prod_lock_0 = aie.lock(%tile_6_5, 2) {init = 2 : i32, sym_name = "B_L2L1_3_3_cons_prod_lock_0"}
    %B_L2L1_3_3_cons_cons_lock_0 = aie.lock(%tile_6_5, 3) {init = 0 : i32, sym_name = "B_L2L1_3_3_cons_cons_lock_0"}
    %B_L3L2_2_prod_lock_0 = aie.lock(%shim_noc_tile_2_0, 2) {init = 0 : i32, sym_name = "B_L3L2_2_prod_lock_0"}
    %B_L3L2_2_cons_lock_0 = aie.lock(%shim_noc_tile_2_0, 3) {init = 0 : i32, sym_name = "B_L3L2_2_cons_lock_0"}
    %B_L2L1_2_0_cons_buff_0 = aie.buffer(%tile_0_4) {address = 19712 : i32, sym_name = "B_L2L1_2_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_2_0_cons_buff_1 = aie.buffer(%tile_0_4) {address = 23808 : i32, sym_name = "B_L2L1_2_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_2_0_cons_prod_lock_0 = aie.lock(%tile_0_4, 2) {init = 2 : i32, sym_name = "B_L2L1_2_0_cons_prod_lock_0"}
    %B_L2L1_2_0_cons_cons_lock_0 = aie.lock(%tile_0_4, 3) {init = 0 : i32, sym_name = "B_L2L1_2_0_cons_cons_lock_0"}
    %B_L2L1_2_1_cons_buff_0 = aie.buffer(%tile_2_4) {address = 19712 : i32, sym_name = "B_L2L1_2_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_2_1_cons_buff_1 = aie.buffer(%tile_2_4) {address = 23808 : i32, sym_name = "B_L2L1_2_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_2_1_cons_prod_lock_0 = aie.lock(%tile_2_4, 2) {init = 2 : i32, sym_name = "B_L2L1_2_1_cons_prod_lock_0"}
    %B_L2L1_2_1_cons_cons_lock_0 = aie.lock(%tile_2_4, 3) {init = 0 : i32, sym_name = "B_L2L1_2_1_cons_cons_lock_0"}
    %B_L2L1_2_2_cons_buff_0 = aie.buffer(%tile_4_4) {address = 19712 : i32, sym_name = "B_L2L1_2_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_2_2_cons_buff_1 = aie.buffer(%tile_4_4) {address = 23808 : i32, sym_name = "B_L2L1_2_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_2_2_cons_prod_lock_0 = aie.lock(%tile_4_4, 2) {init = 2 : i32, sym_name = "B_L2L1_2_2_cons_prod_lock_0"}
    %B_L2L1_2_2_cons_cons_lock_0 = aie.lock(%tile_4_4, 3) {init = 0 : i32, sym_name = "B_L2L1_2_2_cons_cons_lock_0"}
    %B_L2L1_2_3_cons_buff_0 = aie.buffer(%tile_6_4) {address = 19712 : i32, sym_name = "B_L2L1_2_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_2_3_cons_buff_1 = aie.buffer(%tile_6_4) {address = 23808 : i32, sym_name = "B_L2L1_2_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_2_3_cons_prod_lock_0 = aie.lock(%tile_6_4, 2) {init = 2 : i32, sym_name = "B_L2L1_2_3_cons_prod_lock_0"}
    %B_L2L1_2_3_cons_cons_lock_0 = aie.lock(%tile_6_4, 3) {init = 0 : i32, sym_name = "B_L2L1_2_3_cons_cons_lock_0"}
    %B_L3L2_1_prod_lock_0 = aie.lock(%shim_noc_tile_3_0, 2) {init = 0 : i32, sym_name = "B_L3L2_1_prod_lock_0"}
    %B_L3L2_1_cons_lock_0 = aie.lock(%shim_noc_tile_3_0, 3) {init = 0 : i32, sym_name = "B_L3L2_1_cons_lock_0"}
    %B_L2L1_1_0_cons_buff_0 = aie.buffer(%tile_0_3) {address = 19712 : i32, sym_name = "B_L2L1_1_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_1_0_cons_buff_1 = aie.buffer(%tile_0_3) {address = 23808 : i32, sym_name = "B_L2L1_1_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_1_0_cons_prod_lock_0 = aie.lock(%tile_0_3, 2) {init = 2 : i32, sym_name = "B_L2L1_1_0_cons_prod_lock_0"}
    %B_L2L1_1_0_cons_cons_lock_0 = aie.lock(%tile_0_3, 3) {init = 0 : i32, sym_name = "B_L2L1_1_0_cons_cons_lock_0"}
    %B_L2L1_1_1_cons_buff_0 = aie.buffer(%tile_2_3) {address = 19712 : i32, sym_name = "B_L2L1_1_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_1_1_cons_buff_1 = aie.buffer(%tile_2_3) {address = 23808 : i32, sym_name = "B_L2L1_1_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_1_1_cons_prod_lock_0 = aie.lock(%tile_2_3, 2) {init = 2 : i32, sym_name = "B_L2L1_1_1_cons_prod_lock_0"}
    %B_L2L1_1_1_cons_cons_lock_0 = aie.lock(%tile_2_3, 3) {init = 0 : i32, sym_name = "B_L2L1_1_1_cons_cons_lock_0"}
    %B_L2L1_1_2_cons_buff_0 = aie.buffer(%tile_4_3) {address = 19712 : i32, sym_name = "B_L2L1_1_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_1_2_cons_buff_1 = aie.buffer(%tile_4_3) {address = 23808 : i32, sym_name = "B_L2L1_1_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_1_2_cons_prod_lock_0 = aie.lock(%tile_4_3, 2) {init = 2 : i32, sym_name = "B_L2L1_1_2_cons_prod_lock_0"}
    %B_L2L1_1_2_cons_cons_lock_0 = aie.lock(%tile_4_3, 3) {init = 0 : i32, sym_name = "B_L2L1_1_2_cons_cons_lock_0"}
    %B_L2L1_1_3_cons_buff_0 = aie.buffer(%tile_6_3) {address = 19712 : i32, sym_name = "B_L2L1_1_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_1_3_cons_buff_1 = aie.buffer(%tile_6_3) {address = 23808 : i32, sym_name = "B_L2L1_1_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_1_3_cons_prod_lock_0 = aie.lock(%tile_6_3, 2) {init = 2 : i32, sym_name = "B_L2L1_1_3_cons_prod_lock_0"}
    %B_L2L1_1_3_cons_cons_lock_0 = aie.lock(%tile_6_3, 3) {init = 0 : i32, sym_name = "B_L2L1_1_3_cons_cons_lock_0"}
    %B_L3L2_0_prod_lock_0 = aie.lock(%shim_noc_tile_3_0, 0) {init = 0 : i32, sym_name = "B_L3L2_0_prod_lock_0"}
    %B_L3L2_0_cons_lock_0 = aie.lock(%shim_noc_tile_3_0, 1) {init = 0 : i32, sym_name = "B_L3L2_0_cons_lock_0"}
    %B_L2L1_0_0_cons_buff_0 = aie.buffer(%tile_0_2) {address = 19712 : i32, sym_name = "B_L2L1_0_0_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_0_0_cons_buff_1 = aie.buffer(%tile_0_2) {address = 23808 : i32, sym_name = "B_L2L1_0_0_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_0_0_cons_prod_lock_0 = aie.lock(%tile_0_2, 2) {init = 2 : i32, sym_name = "B_L2L1_0_0_cons_prod_lock_0"}
    %B_L2L1_0_0_cons_cons_lock_0 = aie.lock(%tile_0_2, 3) {init = 0 : i32, sym_name = "B_L2L1_0_0_cons_cons_lock_0"}
    %B_L2L1_0_1_cons_buff_0 = aie.buffer(%tile_2_2) {address = 19712 : i32, sym_name = "B_L2L1_0_1_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_0_1_cons_buff_1 = aie.buffer(%tile_2_2) {address = 23808 : i32, sym_name = "B_L2L1_0_1_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_0_1_cons_prod_lock_0 = aie.lock(%tile_2_2, 2) {init = 2 : i32, sym_name = "B_L2L1_0_1_cons_prod_lock_0"}
    %B_L2L1_0_1_cons_cons_lock_0 = aie.lock(%tile_2_2, 3) {init = 0 : i32, sym_name = "B_L2L1_0_1_cons_cons_lock_0"}
    %B_L2L1_0_2_cons_buff_0 = aie.buffer(%tile_4_2) {address = 19712 : i32, sym_name = "B_L2L1_0_2_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_0_2_cons_buff_1 = aie.buffer(%tile_4_2) {address = 23808 : i32, sym_name = "B_L2L1_0_2_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_0_2_cons_prod_lock_0 = aie.lock(%tile_4_2, 2) {init = 2 : i32, sym_name = "B_L2L1_0_2_cons_prod_lock_0"}
    %B_L2L1_0_2_cons_cons_lock_0 = aie.lock(%tile_4_2, 3) {init = 0 : i32, sym_name = "B_L2L1_0_2_cons_cons_lock_0"}
    %B_L2L1_0_3_cons_buff_0 = aie.buffer(%tile_6_2) {address = 19712 : i32, sym_name = "B_L2L1_0_3_cons_buff_0"} : memref<64x64xi8> 
    %B_L2L1_0_3_cons_buff_1 = aie.buffer(%tile_6_2) {address = 23808 : i32, sym_name = "B_L2L1_0_3_cons_buff_1"} : memref<64x64xi8> 
    %B_L2L1_0_3_cons_prod_lock_0 = aie.lock(%tile_6_2, 2) {init = 2 : i32, sym_name = "B_L2L1_0_3_cons_prod_lock_0"}
    %B_L2L1_0_3_cons_cons_lock_0 = aie.lock(%tile_6_2, 3) {init = 0 : i32, sym_name = "B_L2L1_0_3_cons_cons_lock_0"}
    %C_L2L3_7_buff_0 = aie.buffer(%mem_tile_0_1) {address = 0 : i32, sym_name = "C_L2L3_7_buff_0"} : memref<8192xi32> 
    %C_L2L3_7_buff_1 = aie.buffer(%mem_tile_0_1) {address = 32768 : i32, sym_name = "C_L2L3_7_buff_1"} : memref<8192xi32> 
    %C_L2L3_7_prod_lock_0 = aie.lock(%mem_tile_0_1, 0) {init = 2 : i32, sym_name = "C_L2L3_7_prod_lock_0"}
    %C_L2L3_7_cons_lock_0 = aie.lock(%mem_tile_0_1, 1) {init = 0 : i32, sym_name = "C_L2L3_7_cons_lock_0"}
    %C_L2L3_7_prod_lock_1 = aie.lock(%mem_tile_0_1, 2) {init = 2 : i32, sym_name = "C_L2L3_7_prod_lock_1"}
    %C_L2L3_7_cons_lock_1 = aie.lock(%mem_tile_0_1, 3) {init = 0 : i32, sym_name = "C_L2L3_7_cons_lock_1"}
    %C_L2L3_7_prod_lock_2 = aie.lock(%mem_tile_0_1, 4) {init = 2 : i32, sym_name = "C_L2L3_7_prod_lock_2"}
    %C_L2L3_7_cons_lock_2 = aie.lock(%mem_tile_0_1, 5) {init = 0 : i32, sym_name = "C_L2L3_7_cons_lock_2"}
    %C_L2L3_7_prod_lock_3 = aie.lock(%mem_tile_0_1, 6) {init = 2 : i32, sym_name = "C_L2L3_7_prod_lock_3"}
    %C_L2L3_7_cons_lock_3 = aie.lock(%mem_tile_0_1, 7) {init = 0 : i32, sym_name = "C_L2L3_7_cons_lock_3"}
    %A_L3L2_3_prod_lock_0 = aie.lock(%shim_noc_tile_6_0, 0) {init = 0 : i32, sym_name = "A_L3L2_3_prod_lock_0"}
    %A_L3L2_3_cons_lock_0 = aie.lock(%shim_noc_tile_6_0, 1) {init = 0 : i32, sym_name = "A_L3L2_3_cons_lock_0"}
    %A_L2L1_3_0_cons_buff_0 = aie.buffer(%tile_6_2) {address = 27904 : i32, sym_name = "A_L2L1_3_0_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_0_cons_buff_1 = aie.buffer(%tile_6_2) {address = 29952 : i32, sym_name = "A_L2L1_3_0_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_0_cons_prod_lock_0 = aie.lock(%tile_6_2, 0) {init = 2 : i32, sym_name = "A_L2L1_3_0_cons_prod_lock_0"}
    %A_L2L1_3_0_cons_cons_lock_0 = aie.lock(%tile_6_2, 1) {init = 0 : i32, sym_name = "A_L2L1_3_0_cons_cons_lock_0"}
    %A_L2L1_3_1_cons_buff_0 = aie.buffer(%tile_6_3) {address = 27904 : i32, sym_name = "A_L2L1_3_1_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_1_cons_buff_1 = aie.buffer(%tile_6_3) {address = 29952 : i32, sym_name = "A_L2L1_3_1_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_1_cons_prod_lock_0 = aie.lock(%tile_6_3, 0) {init = 2 : i32, sym_name = "A_L2L1_3_1_cons_prod_lock_0"}
    %A_L2L1_3_1_cons_cons_lock_0 = aie.lock(%tile_6_3, 1) {init = 0 : i32, sym_name = "A_L2L1_3_1_cons_cons_lock_0"}
    %A_L2L1_3_2_cons_buff_0 = aie.buffer(%tile_6_4) {address = 27904 : i32, sym_name = "A_L2L1_3_2_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_2_cons_buff_1 = aie.buffer(%tile_6_4) {address = 29952 : i32, sym_name = "A_L2L1_3_2_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_2_cons_prod_lock_0 = aie.lock(%tile_6_4, 0) {init = 2 : i32, sym_name = "A_L2L1_3_2_cons_prod_lock_0"}
    %A_L2L1_3_2_cons_cons_lock_0 = aie.lock(%tile_6_4, 1) {init = 0 : i32, sym_name = "A_L2L1_3_2_cons_cons_lock_0"}
    %A_L2L1_3_3_cons_buff_0 = aie.buffer(%tile_6_5) {address = 27904 : i32, sym_name = "A_L2L1_3_3_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_3_cons_buff_1 = aie.buffer(%tile_6_5) {address = 29952 : i32, sym_name = "A_L2L1_3_3_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_3_cons_prod_lock_0 = aie.lock(%tile_6_5, 0) {init = 2 : i32, sym_name = "A_L2L1_3_3_cons_prod_lock_0"}
    %A_L2L1_3_3_cons_cons_lock_0 = aie.lock(%tile_6_5, 1) {init = 0 : i32, sym_name = "A_L2L1_3_3_cons_cons_lock_0"}
    %A_L2L1_3_4_cons_buff_0 = aie.buffer(%tile_7_2) {address = 27904 : i32, sym_name = "A_L2L1_3_4_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_4_cons_buff_1 = aie.buffer(%tile_7_2) {address = 29952 : i32, sym_name = "A_L2L1_3_4_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_4_cons_prod_lock_0 = aie.lock(%tile_7_2, 0) {init = 2 : i32, sym_name = "A_L2L1_3_4_cons_prod_lock_0"}
    %A_L2L1_3_4_cons_cons_lock_0 = aie.lock(%tile_7_2, 1) {init = 0 : i32, sym_name = "A_L2L1_3_4_cons_cons_lock_0"}
    %A_L2L1_3_5_cons_buff_0 = aie.buffer(%tile_7_3) {address = 27904 : i32, sym_name = "A_L2L1_3_5_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_5_cons_buff_1 = aie.buffer(%tile_7_3) {address = 29952 : i32, sym_name = "A_L2L1_3_5_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_5_cons_prod_lock_0 = aie.lock(%tile_7_3, 0) {init = 2 : i32, sym_name = "A_L2L1_3_5_cons_prod_lock_0"}
    %A_L2L1_3_5_cons_cons_lock_0 = aie.lock(%tile_7_3, 1) {init = 0 : i32, sym_name = "A_L2L1_3_5_cons_cons_lock_0"}
    %A_L2L1_3_6_cons_buff_0 = aie.buffer(%tile_7_4) {address = 27904 : i32, sym_name = "A_L2L1_3_6_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_6_cons_buff_1 = aie.buffer(%tile_7_4) {address = 29952 : i32, sym_name = "A_L2L1_3_6_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_6_cons_prod_lock_0 = aie.lock(%tile_7_4, 0) {init = 2 : i32, sym_name = "A_L2L1_3_6_cons_prod_lock_0"}
    %A_L2L1_3_6_cons_cons_lock_0 = aie.lock(%tile_7_4, 1) {init = 0 : i32, sym_name = "A_L2L1_3_6_cons_cons_lock_0"}
    %A_L2L1_3_7_cons_buff_0 = aie.buffer(%tile_7_5) {address = 27904 : i32, sym_name = "A_L2L1_3_7_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_3_7_cons_buff_1 = aie.buffer(%tile_7_5) {address = 29952 : i32, sym_name = "A_L2L1_3_7_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_3_7_cons_prod_lock_0 = aie.lock(%tile_7_5, 0) {init = 2 : i32, sym_name = "A_L2L1_3_7_cons_prod_lock_0"}
    %A_L2L1_3_7_cons_cons_lock_0 = aie.lock(%tile_7_5, 1) {init = 0 : i32, sym_name = "A_L2L1_3_7_cons_cons_lock_0"}
    %C_L2L3_6_buff_0 = aie.buffer(%mem_tile_7_1) {address = 0 : i32, sym_name = "C_L2L3_6_buff_0"} : memref<8192xi32> 
    %C_L2L3_6_buff_1 = aie.buffer(%mem_tile_7_1) {address = 32768 : i32, sym_name = "C_L2L3_6_buff_1"} : memref<8192xi32> 
    %C_L2L3_6_prod_lock_0 = aie.lock(%mem_tile_7_1, 0) {init = 2 : i32, sym_name = "C_L2L3_6_prod_lock_0"}
    %C_L2L3_6_cons_lock_0 = aie.lock(%mem_tile_7_1, 1) {init = 0 : i32, sym_name = "C_L2L3_6_cons_lock_0"}
    %C_L2L3_6_prod_lock_1 = aie.lock(%mem_tile_7_1, 2) {init = 2 : i32, sym_name = "C_L2L3_6_prod_lock_1"}
    %C_L2L3_6_cons_lock_1 = aie.lock(%mem_tile_7_1, 3) {init = 0 : i32, sym_name = "C_L2L3_6_cons_lock_1"}
    %C_L2L3_6_prod_lock_2 = aie.lock(%mem_tile_7_1, 4) {init = 2 : i32, sym_name = "C_L2L3_6_prod_lock_2"}
    %C_L2L3_6_cons_lock_2 = aie.lock(%mem_tile_7_1, 5) {init = 0 : i32, sym_name = "C_L2L3_6_cons_lock_2"}
    %C_L2L3_6_prod_lock_3 = aie.lock(%mem_tile_7_1, 6) {init = 2 : i32, sym_name = "C_L2L3_6_prod_lock_3"}
    %C_L2L3_6_cons_lock_3 = aie.lock(%mem_tile_7_1, 7) {init = 0 : i32, sym_name = "C_L2L3_6_cons_lock_3"}
    %C_L2L3_5_buff_0 = aie.buffer(%mem_tile_6_1) {address = 0 : i32, sym_name = "C_L2L3_5_buff_0"} : memref<8192xi32> 
    %C_L2L3_5_buff_1 = aie.buffer(%mem_tile_6_1) {address = 32768 : i32, sym_name = "C_L2L3_5_buff_1"} : memref<8192xi32> 
    %C_L2L3_5_prod_lock_0 = aie.lock(%mem_tile_6_1, 0) {init = 2 : i32, sym_name = "C_L2L3_5_prod_lock_0"}
    %C_L2L3_5_cons_lock_0 = aie.lock(%mem_tile_6_1, 1) {init = 0 : i32, sym_name = "C_L2L3_5_cons_lock_0"}
    %C_L2L3_5_prod_lock_1 = aie.lock(%mem_tile_6_1, 2) {init = 2 : i32, sym_name = "C_L2L3_5_prod_lock_1"}
    %C_L2L3_5_cons_lock_1 = aie.lock(%mem_tile_6_1, 3) {init = 0 : i32, sym_name = "C_L2L3_5_cons_lock_1"}
    %C_L2L3_5_prod_lock_2 = aie.lock(%mem_tile_6_1, 4) {init = 2 : i32, sym_name = "C_L2L3_5_prod_lock_2"}
    %C_L2L3_5_cons_lock_2 = aie.lock(%mem_tile_6_1, 5) {init = 0 : i32, sym_name = "C_L2L3_5_cons_lock_2"}
    %C_L2L3_5_prod_lock_3 = aie.lock(%mem_tile_6_1, 6) {init = 2 : i32, sym_name = "C_L2L3_5_prod_lock_3"}
    %C_L2L3_5_cons_lock_3 = aie.lock(%mem_tile_6_1, 7) {init = 0 : i32, sym_name = "C_L2L3_5_cons_lock_3"}
    %A_L3L2_2_prod_lock_0 = aie.lock(%shim_noc_tile_4_0, 0) {init = 0 : i32, sym_name = "A_L3L2_2_prod_lock_0"}
    %A_L3L2_2_cons_lock_0 = aie.lock(%shim_noc_tile_4_0, 1) {init = 0 : i32, sym_name = "A_L3L2_2_cons_lock_0"}
    %A_L2L1_2_0_cons_buff_0 = aie.buffer(%tile_4_2) {address = 27904 : i32, sym_name = "A_L2L1_2_0_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_0_cons_buff_1 = aie.buffer(%tile_4_2) {address = 29952 : i32, sym_name = "A_L2L1_2_0_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_0_cons_prod_lock_0 = aie.lock(%tile_4_2, 0) {init = 2 : i32, sym_name = "A_L2L1_2_0_cons_prod_lock_0"}
    %A_L2L1_2_0_cons_cons_lock_0 = aie.lock(%tile_4_2, 1) {init = 0 : i32, sym_name = "A_L2L1_2_0_cons_cons_lock_0"}
    %A_L2L1_2_1_cons_buff_0 = aie.buffer(%tile_4_3) {address = 27904 : i32, sym_name = "A_L2L1_2_1_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_1_cons_buff_1 = aie.buffer(%tile_4_3) {address = 29952 : i32, sym_name = "A_L2L1_2_1_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_1_cons_prod_lock_0 = aie.lock(%tile_4_3, 0) {init = 2 : i32, sym_name = "A_L2L1_2_1_cons_prod_lock_0"}
    %A_L2L1_2_1_cons_cons_lock_0 = aie.lock(%tile_4_3, 1) {init = 0 : i32, sym_name = "A_L2L1_2_1_cons_cons_lock_0"}
    %A_L2L1_2_2_cons_buff_0 = aie.buffer(%tile_4_4) {address = 27904 : i32, sym_name = "A_L2L1_2_2_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_2_cons_buff_1 = aie.buffer(%tile_4_4) {address = 29952 : i32, sym_name = "A_L2L1_2_2_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_2_cons_prod_lock_0 = aie.lock(%tile_4_4, 0) {init = 2 : i32, sym_name = "A_L2L1_2_2_cons_prod_lock_0"}
    %A_L2L1_2_2_cons_cons_lock_0 = aie.lock(%tile_4_4, 1) {init = 0 : i32, sym_name = "A_L2L1_2_2_cons_cons_lock_0"}
    %A_L2L1_2_3_cons_buff_0 = aie.buffer(%tile_4_5) {address = 27904 : i32, sym_name = "A_L2L1_2_3_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_3_cons_buff_1 = aie.buffer(%tile_4_5) {address = 29952 : i32, sym_name = "A_L2L1_2_3_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_3_cons_prod_lock_0 = aie.lock(%tile_4_5, 0) {init = 2 : i32, sym_name = "A_L2L1_2_3_cons_prod_lock_0"}
    %A_L2L1_2_3_cons_cons_lock_0 = aie.lock(%tile_4_5, 1) {init = 0 : i32, sym_name = "A_L2L1_2_3_cons_cons_lock_0"}
    %A_L2L1_2_4_cons_buff_0 = aie.buffer(%tile_5_2) {address = 27904 : i32, sym_name = "A_L2L1_2_4_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_4_cons_buff_1 = aie.buffer(%tile_5_2) {address = 29952 : i32, sym_name = "A_L2L1_2_4_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_4_cons_prod_lock_0 = aie.lock(%tile_5_2, 0) {init = 2 : i32, sym_name = "A_L2L1_2_4_cons_prod_lock_0"}
    %A_L2L1_2_4_cons_cons_lock_0 = aie.lock(%tile_5_2, 1) {init = 0 : i32, sym_name = "A_L2L1_2_4_cons_cons_lock_0"}
    %A_L2L1_2_5_cons_buff_0 = aie.buffer(%tile_5_3) {address = 27904 : i32, sym_name = "A_L2L1_2_5_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_5_cons_buff_1 = aie.buffer(%tile_5_3) {address = 29952 : i32, sym_name = "A_L2L1_2_5_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_5_cons_prod_lock_0 = aie.lock(%tile_5_3, 0) {init = 2 : i32, sym_name = "A_L2L1_2_5_cons_prod_lock_0"}
    %A_L2L1_2_5_cons_cons_lock_0 = aie.lock(%tile_5_3, 1) {init = 0 : i32, sym_name = "A_L2L1_2_5_cons_cons_lock_0"}
    %A_L2L1_2_6_cons_buff_0 = aie.buffer(%tile_5_4) {address = 27904 : i32, sym_name = "A_L2L1_2_6_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_6_cons_buff_1 = aie.buffer(%tile_5_4) {address = 29952 : i32, sym_name = "A_L2L1_2_6_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_6_cons_prod_lock_0 = aie.lock(%tile_5_4, 0) {init = 2 : i32, sym_name = "A_L2L1_2_6_cons_prod_lock_0"}
    %A_L2L1_2_6_cons_cons_lock_0 = aie.lock(%tile_5_4, 1) {init = 0 : i32, sym_name = "A_L2L1_2_6_cons_cons_lock_0"}
    %A_L2L1_2_7_cons_buff_0 = aie.buffer(%tile_5_5) {address = 27904 : i32, sym_name = "A_L2L1_2_7_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_2_7_cons_buff_1 = aie.buffer(%tile_5_5) {address = 29952 : i32, sym_name = "A_L2L1_2_7_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_2_7_cons_prod_lock_0 = aie.lock(%tile_5_5, 0) {init = 2 : i32, sym_name = "A_L2L1_2_7_cons_prod_lock_0"}
    %A_L2L1_2_7_cons_cons_lock_0 = aie.lock(%tile_5_5, 1) {init = 0 : i32, sym_name = "A_L2L1_2_7_cons_cons_lock_0"}
    %C_L2L3_4_buff_0 = aie.buffer(%mem_tile_5_1) {address = 0 : i32, sym_name = "C_L2L3_4_buff_0"} : memref<8192xi32> 
    %C_L2L3_4_buff_1 = aie.buffer(%mem_tile_5_1) {address = 32768 : i32, sym_name = "C_L2L3_4_buff_1"} : memref<8192xi32> 
    %C_L2L3_4_prod_lock_0 = aie.lock(%mem_tile_5_1, 0) {init = 2 : i32, sym_name = "C_L2L3_4_prod_lock_0"}
    %C_L2L3_4_cons_lock_0 = aie.lock(%mem_tile_5_1, 1) {init = 0 : i32, sym_name = "C_L2L3_4_cons_lock_0"}
    %C_L2L3_4_prod_lock_1 = aie.lock(%mem_tile_5_1, 2) {init = 2 : i32, sym_name = "C_L2L3_4_prod_lock_1"}
    %C_L2L3_4_cons_lock_1 = aie.lock(%mem_tile_5_1, 3) {init = 0 : i32, sym_name = "C_L2L3_4_cons_lock_1"}
    %C_L2L3_4_prod_lock_2 = aie.lock(%mem_tile_5_1, 4) {init = 2 : i32, sym_name = "C_L2L3_4_prod_lock_2"}
    %C_L2L3_4_cons_lock_2 = aie.lock(%mem_tile_5_1, 5) {init = 0 : i32, sym_name = "C_L2L3_4_cons_lock_2"}
    %C_L2L3_4_prod_lock_3 = aie.lock(%mem_tile_5_1, 6) {init = 2 : i32, sym_name = "C_L2L3_4_prod_lock_3"}
    %C_L2L3_4_cons_lock_3 = aie.lock(%mem_tile_5_1, 7) {init = 0 : i32, sym_name = "C_L2L3_4_cons_lock_3"}
    %C_L2L3_3_buff_0 = aie.buffer(%mem_tile_1_1) {address = 0 : i32, sym_name = "C_L2L3_3_buff_0"} : memref<8192xi32> 
    %C_L2L3_3_buff_1 = aie.buffer(%mem_tile_1_1) {address = 32768 : i32, sym_name = "C_L2L3_3_buff_1"} : memref<8192xi32> 
    %C_L2L3_3_prod_lock_0 = aie.lock(%mem_tile_1_1, 0) {init = 2 : i32, sym_name = "C_L2L3_3_prod_lock_0"}
    %C_L2L3_3_cons_lock_0 = aie.lock(%mem_tile_1_1, 1) {init = 0 : i32, sym_name = "C_L2L3_3_cons_lock_0"}
    %C_L2L3_3_prod_lock_1 = aie.lock(%mem_tile_1_1, 2) {init = 2 : i32, sym_name = "C_L2L3_3_prod_lock_1"}
    %C_L2L3_3_cons_lock_1 = aie.lock(%mem_tile_1_1, 3) {init = 0 : i32, sym_name = "C_L2L3_3_cons_lock_1"}
    %C_L2L3_3_prod_lock_2 = aie.lock(%mem_tile_1_1, 4) {init = 2 : i32, sym_name = "C_L2L3_3_prod_lock_2"}
    %C_L2L3_3_cons_lock_2 = aie.lock(%mem_tile_1_1, 5) {init = 0 : i32, sym_name = "C_L2L3_3_cons_lock_2"}
    %C_L2L3_3_prod_lock_3 = aie.lock(%mem_tile_1_1, 6) {init = 2 : i32, sym_name = "C_L2L3_3_prod_lock_3"}
    %C_L2L3_3_cons_lock_3 = aie.lock(%mem_tile_1_1, 7) {init = 0 : i32, sym_name = "C_L2L3_3_cons_lock_3"}
    %A_L3L2_1_prod_lock_0 = aie.lock(%shim_noc_tile_2_0, 0) {init = 0 : i32, sym_name = "A_L3L2_1_prod_lock_0"}
    %A_L3L2_1_cons_lock_0 = aie.lock(%shim_noc_tile_2_0, 1) {init = 0 : i32, sym_name = "A_L3L2_1_cons_lock_0"}
    %A_L2L1_1_0_cons_buff_0 = aie.buffer(%tile_2_2) {address = 27904 : i32, sym_name = "A_L2L1_1_0_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_0_cons_buff_1 = aie.buffer(%tile_2_2) {address = 29952 : i32, sym_name = "A_L2L1_1_0_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_0_cons_prod_lock_0 = aie.lock(%tile_2_2, 0) {init = 2 : i32, sym_name = "A_L2L1_1_0_cons_prod_lock_0"}
    %A_L2L1_1_0_cons_cons_lock_0 = aie.lock(%tile_2_2, 1) {init = 0 : i32, sym_name = "A_L2L1_1_0_cons_cons_lock_0"}
    %A_L2L1_1_1_cons_buff_0 = aie.buffer(%tile_2_3) {address = 27904 : i32, sym_name = "A_L2L1_1_1_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_1_cons_buff_1 = aie.buffer(%tile_2_3) {address = 29952 : i32, sym_name = "A_L2L1_1_1_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_1_cons_prod_lock_0 = aie.lock(%tile_2_3, 0) {init = 2 : i32, sym_name = "A_L2L1_1_1_cons_prod_lock_0"}
    %A_L2L1_1_1_cons_cons_lock_0 = aie.lock(%tile_2_3, 1) {init = 0 : i32, sym_name = "A_L2L1_1_1_cons_cons_lock_0"}
    %A_L2L1_1_2_cons_buff_0 = aie.buffer(%tile_2_4) {address = 27904 : i32, sym_name = "A_L2L1_1_2_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_2_cons_buff_1 = aie.buffer(%tile_2_4) {address = 29952 : i32, sym_name = "A_L2L1_1_2_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_2_cons_prod_lock_0 = aie.lock(%tile_2_4, 0) {init = 2 : i32, sym_name = "A_L2L1_1_2_cons_prod_lock_0"}
    %A_L2L1_1_2_cons_cons_lock_0 = aie.lock(%tile_2_4, 1) {init = 0 : i32, sym_name = "A_L2L1_1_2_cons_cons_lock_0"}
    %A_L2L1_1_3_cons_buff_0 = aie.buffer(%tile_2_5) {address = 27904 : i32, sym_name = "A_L2L1_1_3_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_3_cons_buff_1 = aie.buffer(%tile_2_5) {address = 29952 : i32, sym_name = "A_L2L1_1_3_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_3_cons_prod_lock_0 = aie.lock(%tile_2_5, 0) {init = 2 : i32, sym_name = "A_L2L1_1_3_cons_prod_lock_0"}
    %A_L2L1_1_3_cons_cons_lock_0 = aie.lock(%tile_2_5, 1) {init = 0 : i32, sym_name = "A_L2L1_1_3_cons_cons_lock_0"}
    %A_L2L1_1_4_cons_buff_0 = aie.buffer(%tile_3_2) {address = 27904 : i32, sym_name = "A_L2L1_1_4_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_4_cons_buff_1 = aie.buffer(%tile_3_2) {address = 29952 : i32, sym_name = "A_L2L1_1_4_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_4_cons_prod_lock_0 = aie.lock(%tile_3_2, 0) {init = 2 : i32, sym_name = "A_L2L1_1_4_cons_prod_lock_0"}
    %A_L2L1_1_4_cons_cons_lock_0 = aie.lock(%tile_3_2, 1) {init = 0 : i32, sym_name = "A_L2L1_1_4_cons_cons_lock_0"}
    %A_L2L1_1_5_cons_buff_0 = aie.buffer(%tile_3_3) {address = 27904 : i32, sym_name = "A_L2L1_1_5_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_5_cons_buff_1 = aie.buffer(%tile_3_3) {address = 29952 : i32, sym_name = "A_L2L1_1_5_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_5_cons_prod_lock_0 = aie.lock(%tile_3_3, 0) {init = 2 : i32, sym_name = "A_L2L1_1_5_cons_prod_lock_0"}
    %A_L2L1_1_5_cons_cons_lock_0 = aie.lock(%tile_3_3, 1) {init = 0 : i32, sym_name = "A_L2L1_1_5_cons_cons_lock_0"}
    %A_L2L1_1_6_cons_buff_0 = aie.buffer(%tile_3_4) {address = 27904 : i32, sym_name = "A_L2L1_1_6_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_6_cons_buff_1 = aie.buffer(%tile_3_4) {address = 29952 : i32, sym_name = "A_L2L1_1_6_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_6_cons_prod_lock_0 = aie.lock(%tile_3_4, 0) {init = 2 : i32, sym_name = "A_L2L1_1_6_cons_prod_lock_0"}
    %A_L2L1_1_6_cons_cons_lock_0 = aie.lock(%tile_3_4, 1) {init = 0 : i32, sym_name = "A_L2L1_1_6_cons_cons_lock_0"}
    %A_L2L1_1_7_cons_buff_0 = aie.buffer(%tile_3_5) {address = 27904 : i32, sym_name = "A_L2L1_1_7_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_1_7_cons_buff_1 = aie.buffer(%tile_3_5) {address = 29952 : i32, sym_name = "A_L2L1_1_7_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_1_7_cons_prod_lock_0 = aie.lock(%tile_3_5, 0) {init = 2 : i32, sym_name = "A_L2L1_1_7_cons_prod_lock_0"}
    %A_L2L1_1_7_cons_cons_lock_0 = aie.lock(%tile_3_5, 1) {init = 0 : i32, sym_name = "A_L2L1_1_7_cons_cons_lock_0"}
    %C_L2L3_2_buff_0 = aie.buffer(%mem_tile_4_1) {address = 0 : i32, sym_name = "C_L2L3_2_buff_0"} : memref<8192xi32> 
    %C_L2L3_2_buff_1 = aie.buffer(%mem_tile_4_1) {address = 32768 : i32, sym_name = "C_L2L3_2_buff_1"} : memref<8192xi32> 
    %C_L2L3_2_prod_lock_0 = aie.lock(%mem_tile_4_1, 0) {init = 2 : i32, sym_name = "C_L2L3_2_prod_lock_0"}
    %C_L2L3_2_cons_lock_0 = aie.lock(%mem_tile_4_1, 1) {init = 0 : i32, sym_name = "C_L2L3_2_cons_lock_0"}
    %C_L2L3_2_prod_lock_1 = aie.lock(%mem_tile_4_1, 2) {init = 2 : i32, sym_name = "C_L2L3_2_prod_lock_1"}
    %C_L2L3_2_cons_lock_1 = aie.lock(%mem_tile_4_1, 3) {init = 0 : i32, sym_name = "C_L2L3_2_cons_lock_1"}
    %C_L2L3_2_prod_lock_2 = aie.lock(%mem_tile_4_1, 4) {init = 2 : i32, sym_name = "C_L2L3_2_prod_lock_2"}
    %C_L2L3_2_cons_lock_2 = aie.lock(%mem_tile_4_1, 5) {init = 0 : i32, sym_name = "C_L2L3_2_cons_lock_2"}
    %C_L2L3_2_prod_lock_3 = aie.lock(%mem_tile_4_1, 6) {init = 2 : i32, sym_name = "C_L2L3_2_prod_lock_3"}
    %C_L2L3_2_cons_lock_3 = aie.lock(%mem_tile_4_1, 7) {init = 0 : i32, sym_name = "C_L2L3_2_cons_lock_3"}
    %C_L2L3_1_buff_0 = aie.buffer(%mem_tile_2_1) {address = 0 : i32, sym_name = "C_L2L3_1_buff_0"} : memref<8192xi32> 
    %C_L2L3_1_buff_1 = aie.buffer(%mem_tile_2_1) {address = 32768 : i32, sym_name = "C_L2L3_1_buff_1"} : memref<8192xi32> 
    %C_L2L3_1_prod_lock_0 = aie.lock(%mem_tile_2_1, 0) {init = 2 : i32, sym_name = "C_L2L3_1_prod_lock_0"}
    %C_L2L3_1_cons_lock_0 = aie.lock(%mem_tile_2_1, 1) {init = 0 : i32, sym_name = "C_L2L3_1_cons_lock_0"}
    %C_L2L3_1_prod_lock_1 = aie.lock(%mem_tile_2_1, 2) {init = 2 : i32, sym_name = "C_L2L3_1_prod_lock_1"}
    %C_L2L3_1_cons_lock_1 = aie.lock(%mem_tile_2_1, 3) {init = 0 : i32, sym_name = "C_L2L3_1_cons_lock_1"}
    %C_L2L3_1_prod_lock_2 = aie.lock(%mem_tile_2_1, 4) {init = 2 : i32, sym_name = "C_L2L3_1_prod_lock_2"}
    %C_L2L3_1_cons_lock_2 = aie.lock(%mem_tile_2_1, 5) {init = 0 : i32, sym_name = "C_L2L3_1_cons_lock_2"}
    %C_L2L3_1_prod_lock_3 = aie.lock(%mem_tile_2_1, 6) {init = 2 : i32, sym_name = "C_L2L3_1_prod_lock_3"}
    %C_L2L3_1_cons_lock_3 = aie.lock(%mem_tile_2_1, 7) {init = 0 : i32, sym_name = "C_L2L3_1_cons_lock_3"}
    %A_L3L2_0_prod_lock_0 = aie.lock(%shim_noc_tile_0_0, 0) {init = 0 : i32, sym_name = "A_L3L2_0_prod_lock_0"}
    %A_L3L2_0_cons_lock_0 = aie.lock(%shim_noc_tile_0_0, 1) {init = 0 : i32, sym_name = "A_L3L2_0_cons_lock_0"}
    %A_L2L1_0_0_cons_buff_0 = aie.buffer(%tile_0_2) {address = 27904 : i32, sym_name = "A_L2L1_0_0_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_0_cons_buff_1 = aie.buffer(%tile_0_2) {address = 29952 : i32, sym_name = "A_L2L1_0_0_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_0_cons_prod_lock_0 = aie.lock(%tile_0_2, 0) {init = 2 : i32, sym_name = "A_L2L1_0_0_cons_prod_lock_0"}
    %A_L2L1_0_0_cons_cons_lock_0 = aie.lock(%tile_0_2, 1) {init = 0 : i32, sym_name = "A_L2L1_0_0_cons_cons_lock_0"}
    %A_L2L1_0_1_cons_buff_0 = aie.buffer(%tile_0_3) {address = 27904 : i32, sym_name = "A_L2L1_0_1_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_1_cons_buff_1 = aie.buffer(%tile_0_3) {address = 29952 : i32, sym_name = "A_L2L1_0_1_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_1_cons_prod_lock_0 = aie.lock(%tile_0_3, 0) {init = 2 : i32, sym_name = "A_L2L1_0_1_cons_prod_lock_0"}
    %A_L2L1_0_1_cons_cons_lock_0 = aie.lock(%tile_0_3, 1) {init = 0 : i32, sym_name = "A_L2L1_0_1_cons_cons_lock_0"}
    %A_L2L1_0_2_cons_buff_0 = aie.buffer(%tile_0_4) {address = 27904 : i32, sym_name = "A_L2L1_0_2_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_2_cons_buff_1 = aie.buffer(%tile_0_4) {address = 29952 : i32, sym_name = "A_L2L1_0_2_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_2_cons_prod_lock_0 = aie.lock(%tile_0_4, 0) {init = 2 : i32, sym_name = "A_L2L1_0_2_cons_prod_lock_0"}
    %A_L2L1_0_2_cons_cons_lock_0 = aie.lock(%tile_0_4, 1) {init = 0 : i32, sym_name = "A_L2L1_0_2_cons_cons_lock_0"}
    %A_L2L1_0_3_cons_buff_0 = aie.buffer(%tile_0_5) {address = 27904 : i32, sym_name = "A_L2L1_0_3_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_3_cons_buff_1 = aie.buffer(%tile_0_5) {address = 29952 : i32, sym_name = "A_L2L1_0_3_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_3_cons_prod_lock_0 = aie.lock(%tile_0_5, 0) {init = 2 : i32, sym_name = "A_L2L1_0_3_cons_prod_lock_0"}
    %A_L2L1_0_3_cons_cons_lock_0 = aie.lock(%tile_0_5, 1) {init = 0 : i32, sym_name = "A_L2L1_0_3_cons_cons_lock_0"}
    %A_L2L1_0_4_cons_buff_0 = aie.buffer(%tile_1_2) {address = 27904 : i32, sym_name = "A_L2L1_0_4_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_4_cons_buff_1 = aie.buffer(%tile_1_2) {address = 29952 : i32, sym_name = "A_L2L1_0_4_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_4_cons_prod_lock_0 = aie.lock(%tile_1_2, 0) {init = 2 : i32, sym_name = "A_L2L1_0_4_cons_prod_lock_0"}
    %A_L2L1_0_4_cons_cons_lock_0 = aie.lock(%tile_1_2, 1) {init = 0 : i32, sym_name = "A_L2L1_0_4_cons_cons_lock_0"}
    %A_L2L1_0_5_cons_buff_0 = aie.buffer(%tile_1_3) {address = 27904 : i32, sym_name = "A_L2L1_0_5_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_5_cons_buff_1 = aie.buffer(%tile_1_3) {address = 29952 : i32, sym_name = "A_L2L1_0_5_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_5_cons_prod_lock_0 = aie.lock(%tile_1_3, 0) {init = 2 : i32, sym_name = "A_L2L1_0_5_cons_prod_lock_0"}
    %A_L2L1_0_5_cons_cons_lock_0 = aie.lock(%tile_1_3, 1) {init = 0 : i32, sym_name = "A_L2L1_0_5_cons_cons_lock_0"}
    %A_L2L1_0_6_cons_buff_0 = aie.buffer(%tile_1_4) {address = 27904 : i32, sym_name = "A_L2L1_0_6_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_6_cons_buff_1 = aie.buffer(%tile_1_4) {address = 29952 : i32, sym_name = "A_L2L1_0_6_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_6_cons_prod_lock_0 = aie.lock(%tile_1_4, 0) {init = 2 : i32, sym_name = "A_L2L1_0_6_cons_prod_lock_0"}
    %A_L2L1_0_6_cons_cons_lock_0 = aie.lock(%tile_1_4, 1) {init = 0 : i32, sym_name = "A_L2L1_0_6_cons_cons_lock_0"}
    %A_L2L1_0_7_cons_buff_0 = aie.buffer(%tile_1_5) {address = 27904 : i32, sym_name = "A_L2L1_0_7_cons_buff_0"} : memref<32x64xi8> 
    %A_L2L1_0_7_cons_buff_1 = aie.buffer(%tile_1_5) {address = 29952 : i32, sym_name = "A_L2L1_0_7_cons_buff_1"} : memref<32x64xi8> 
    %A_L2L1_0_7_cons_prod_lock_0 = aie.lock(%tile_1_5, 0) {init = 2 : i32, sym_name = "A_L2L1_0_7_cons_prod_lock_0"}
    %A_L2L1_0_7_cons_cons_lock_0 = aie.lock(%tile_1_5, 1) {init = 0 : i32, sym_name = "A_L2L1_0_7_cons_cons_lock_0"}
    %C_L2L3_0_buff_0 = aie.buffer(%mem_tile_3_1) {address = 0 : i32, sym_name = "C_L2L3_0_buff_0"} : memref<8192xi32> 
    %C_L2L3_0_buff_1 = aie.buffer(%mem_tile_3_1) {address = 32768 : i32, sym_name = "C_L2L3_0_buff_1"} : memref<8192xi32> 
    %C_L2L3_0_prod_lock_0 = aie.lock(%mem_tile_3_1, 0) {init = 2 : i32, sym_name = "C_L2L3_0_prod_lock_0"}
    %C_L2L3_0_cons_lock_0 = aie.lock(%mem_tile_3_1, 1) {init = 0 : i32, sym_name = "C_L2L3_0_cons_lock_0"}
    %C_L2L3_0_prod_lock_1 = aie.lock(%mem_tile_3_1, 2) {init = 2 : i32, sym_name = "C_L2L3_0_prod_lock_1"}
    %C_L2L3_0_cons_lock_1 = aie.lock(%mem_tile_3_1, 3) {init = 0 : i32, sym_name = "C_L2L3_0_cons_lock_1"}
    %C_L2L3_0_prod_lock_2 = aie.lock(%mem_tile_3_1, 4) {init = 2 : i32, sym_name = "C_L2L3_0_prod_lock_2"}
    %C_L2L3_0_cons_lock_2 = aie.lock(%mem_tile_3_1, 5) {init = 0 : i32, sym_name = "C_L2L3_0_cons_lock_2"}
    %C_L2L3_0_prod_lock_3 = aie.lock(%mem_tile_3_1, 6) {init = 2 : i32, sym_name = "C_L2L3_0_prod_lock_3"}
    %C_L2L3_0_cons_lock_3 = aie.lock(%mem_tile_3_1, 7) {init = 0 : i32, sym_name = "C_L2L3_0_cons_lock_3"}
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_1_5, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_1_4, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_1_3, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_1_2, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_0_5, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_0_4, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_0_3, DMA : 0)
    aie.flow(%mem_tile_0_1, DMA : 0, %tile_0_2, DMA : 0)
    aie.flow(%shim_noc_tile_0_0, DMA : 0, %mem_tile_0_1, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_3_5, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_3_4, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_3_3, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_3_2, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_2_5, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_2_4, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_2_3, DMA : 0)
    aie.flow(%mem_tile_2_1, DMA : 0, %tile_2_2, DMA : 0)
    aie.flow(%shim_noc_tile_2_0, DMA : 0, %mem_tile_2_1, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_5_5, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_5_4, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_5_3, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_5_2, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_4_5, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_4_4, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_4_3, DMA : 0)
    aie.flow(%mem_tile_4_1, DMA : 0, %tile_4_2, DMA : 0)
    aie.flow(%shim_noc_tile_4_0, DMA : 0, %mem_tile_4_1, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_7_5, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_7_4, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_7_3, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_7_2, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_6_5, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_6_4, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_6_3, DMA : 0)
    aie.flow(%mem_tile_6_1, DMA : 0, %tile_6_2, DMA : 0)
    aie.flow(%shim_noc_tile_6_0, DMA : 0, %mem_tile_6_1, DMA : 0)
    aie.flow(%mem_tile_3_1, DMA : 0, %tile_6_2, DMA : 1)
    aie.flow(%mem_tile_3_1, DMA : 0, %tile_4_2, DMA : 1)
    aie.flow(%mem_tile_3_1, DMA : 0, %tile_2_2, DMA : 1)
    aie.flow(%mem_tile_3_1, DMA : 0, %tile_0_2, DMA : 1)
    aie.flow(%shim_noc_tile_3_0, DMA : 0, %mem_tile_3_1, DMA : 0)
    aie.flow(%mem_tile_3_1, DMA : 1, %tile_6_3, DMA : 1)
    aie.flow(%mem_tile_3_1, DMA : 1, %tile_4_3, DMA : 1)
    aie.flow(%mem_tile_3_1, DMA : 1, %tile_2_3, DMA : 1)
    aie.flow(%mem_tile_3_1, DMA : 1, %tile_0_3, DMA : 1)
    aie.flow(%shim_noc_tile_3_0, DMA : 1, %mem_tile_3_1, DMA : 1)
    aie.flow(%mem_tile_2_1, DMA : 1, %tile_6_4, DMA : 1)
    aie.flow(%mem_tile_2_1, DMA : 1, %tile_4_4, DMA : 1)
    aie.flow(%mem_tile_2_1, DMA : 1, %tile_2_4, DMA : 1)
    aie.flow(%mem_tile_2_1, DMA : 1, %tile_0_4, DMA : 1)
    aie.flow(%shim_noc_tile_2_0, DMA : 1, %mem_tile_2_1, DMA : 1)
    aie.flow(%mem_tile_4_1, DMA : 1, %tile_6_5, DMA : 1)
    aie.flow(%mem_tile_4_1, DMA : 1, %tile_4_5, DMA : 1)
    aie.flow(%mem_tile_4_1, DMA : 1, %tile_2_5, DMA : 1)
    aie.flow(%mem_tile_4_1, DMA : 1, %tile_0_5, DMA : 1)
    aie.flow(%shim_noc_tile_4_0, DMA : 1, %mem_tile_4_1, DMA : 1)
    aie.flow(%mem_tile_5_1, DMA : 0, %tile_7_2, DMA : 1)
    aie.flow(%mem_tile_5_1, DMA : 0, %tile_5_2, DMA : 1)
    aie.flow(%mem_tile_5_1, DMA : 0, %tile_3_2, DMA : 1)
    aie.flow(%mem_tile_5_1, DMA : 0, %tile_1_2, DMA : 1)
    aie.flow(%shim_noc_tile_5_0, DMA : 0, %mem_tile_5_1, DMA : 0)
    aie.flow(%mem_tile_5_1, DMA : 1, %tile_7_3, DMA : 1)
    aie.flow(%mem_tile_5_1, DMA : 1, %tile_5_3, DMA : 1)
    aie.flow(%mem_tile_5_1, DMA : 1, %tile_3_3, DMA : 1)
    aie.flow(%mem_tile_5_1, DMA : 1, %tile_1_3, DMA : 1)
    aie.flow(%shim_noc_tile_5_0, DMA : 1, %mem_tile_5_1, DMA : 1)
    aie.flow(%mem_tile_6_1, DMA : 1, %tile_7_4, DMA : 1)
    aie.flow(%mem_tile_6_1, DMA : 1, %tile_5_4, DMA : 1)
    aie.flow(%mem_tile_6_1, DMA : 1, %tile_3_4, DMA : 1)
    aie.flow(%mem_tile_6_1, DMA : 1, %tile_1_4, DMA : 1)
    aie.flow(%shim_noc_tile_6_0, DMA : 1, %mem_tile_6_1, DMA : 1)
    aie.flow(%mem_tile_1_1, DMA : 0, %tile_7_5, DMA : 1)
    aie.flow(%mem_tile_1_1, DMA : 0, %tile_5_5, DMA : 1)
    aie.flow(%mem_tile_1_1, DMA : 0, %tile_3_5, DMA : 1)
    aie.flow(%mem_tile_1_1, DMA : 0, %tile_1_5, DMA : 1)
    aie.flow(%shim_noc_tile_1_0, DMA : 0, %mem_tile_1_1, DMA : 0)
    aie.flow(%tile_0_2, DMA : 0, %mem_tile_3_1, DMA : 2)
    aie.flow(%tile_2_2, DMA : 0, %mem_tile_3_1, DMA : 3)
    aie.flow(%tile_4_2, DMA : 0, %mem_tile_3_1, DMA : 4)
    aie.flow(%tile_6_2, DMA : 0, %mem_tile_3_1, DMA : 5)
    aie.flow(%mem_tile_3_1, DMA : 2, %shim_noc_tile_3_0, DMA : 0)
    aie.flow(%tile_0_3, DMA : 0, %mem_tile_2_1, DMA : 2)
    aie.flow(%tile_2_3, DMA : 0, %mem_tile_2_1, DMA : 3)
    aie.flow(%tile_4_3, DMA : 0, %mem_tile_2_1, DMA : 4)
    aie.flow(%tile_6_3, DMA : 0, %mem_tile_2_1, DMA : 5)
    aie.flow(%mem_tile_2_1, DMA : 2, %shim_noc_tile_3_0, DMA : 1)
    aie.flow(%tile_0_4, DMA : 0, %mem_tile_4_1, DMA : 2)
    aie.flow(%tile_2_4, DMA : 0, %mem_tile_4_1, DMA : 3)
    aie.flow(%tile_4_4, DMA : 0, %mem_tile_4_1, DMA : 4)
    aie.flow(%tile_6_4, DMA : 0, %mem_tile_4_1, DMA : 5)
    aie.flow(%mem_tile_4_1, DMA : 2, %shim_noc_tile_2_0, DMA : 0)
    aie.flow(%tile_0_5, DMA : 0, %mem_tile_1_1, DMA : 1)
    aie.flow(%tile_2_5, DMA : 0, %mem_tile_1_1, DMA : 2)
    aie.flow(%tile_4_5, DMA : 0, %mem_tile_1_1, DMA : 3)
    aie.flow(%tile_6_5, DMA : 0, %mem_tile_1_1, DMA : 4)
    aie.flow(%mem_tile_1_1, DMA : 1, %shim_noc_tile_4_0, DMA : 0)
    aie.flow(%tile_1_2, DMA : 0, %mem_tile_5_1, DMA : 2)
    aie.flow(%tile_3_2, DMA : 0, %mem_tile_5_1, DMA : 3)
    aie.flow(%tile_5_2, DMA : 0, %mem_tile_5_1, DMA : 4)
    aie.flow(%tile_7_2, DMA : 0, %mem_tile_5_1, DMA : 5)
    aie.flow(%mem_tile_5_1, DMA : 2, %shim_noc_tile_4_0, DMA : 1)
    aie.flow(%tile_1_3, DMA : 0, %mem_tile_6_1, DMA : 2)
    aie.flow(%tile_3_3, DMA : 0, %mem_tile_6_1, DMA : 3)
    aie.flow(%tile_5_3, DMA : 0, %mem_tile_6_1, DMA : 4)
    aie.flow(%tile_7_3, DMA : 0, %mem_tile_6_1, DMA : 5)
    aie.flow(%mem_tile_6_1, DMA : 2, %shim_noc_tile_5_0, DMA : 0)
    aie.flow(%tile_1_4, DMA : 0, %mem_tile_7_1, DMA : 0)
    aie.flow(%tile_3_4, DMA : 0, %mem_tile_7_1, DMA : 1)
    aie.flow(%tile_5_4, DMA : 0, %mem_tile_7_1, DMA : 2)
    aie.flow(%tile_7_4, DMA : 0, %mem_tile_7_1, DMA : 3)
    aie.flow(%mem_tile_7_1, DMA : 0, %shim_noc_tile_5_0, DMA : 1)
    aie.flow(%tile_1_5, DMA : 0, %mem_tile_0_1, DMA : 1)
    aie.flow(%tile_3_5, DMA : 0, %mem_tile_0_1, DMA : 2)
    aie.flow(%tile_5_5, DMA : 0, %mem_tile_0_1, DMA : 3)
    aie.flow(%tile_7_5, DMA : 0, %mem_tile_0_1, DMA : 4)
    aie.flow(%mem_tile_0_1, DMA : 1, %shim_noc_tile_6_0, DMA : 0)
    func.func private @zero_i32(memref<2048xi32>) attributes {link_with = "matmul_i8_i32_42bea9df.o"}
    func.func private @"42bea9df_matmul_i8_i32"(memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) attributes {link_with = "matmul_i8_i32_42bea9df.o"}
    %_anonymous0 = aie.buffer(%tile_0_2) {address = 32000 : i32, sym_name = "_anonymous0"} : memref<3xi32> 
    %core_0_2 = aie.core(%tile_0_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
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
      aie.use_lock(%C_L1L2_0_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous0[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous0[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_0_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_0_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_0_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_0_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous0[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_0_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous0[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous0[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_0_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous0[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous0[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_0_0_cons_lock_0, Release, 1)
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
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous1 = aie.buffer(%tile_0_3) {address = 32000 : i32, sym_name = "_anonymous1"} : memref<3xi32> 
    %core_0_3 = aie.core(%tile_0_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous1[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous1[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous1[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_1_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous1[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous1[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_1_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_1_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_1_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_1_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous1[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_1_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous1[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous1[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_1_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous1[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous1[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_1_0_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous1[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous1[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous2 = aie.buffer(%tile_0_4) {address = 32000 : i32, sym_name = "_anonymous2"} : memref<3xi32> 
    %core_0_4 = aie.core(%tile_0_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous2[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous2[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous2[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_2_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous2[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous2[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_2_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_2_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_2_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_2_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous2[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_2_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous2[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous2[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_2_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous2[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous2[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_2_0_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous2[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous2[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous3 = aie.buffer(%tile_0_5) {address = 32000 : i32, sym_name = "_anonymous3"} : memref<3xi32> 
    %core_0_5 = aie.core(%tile_0_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous3[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous3[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous3[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_3_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous3[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous3[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_3_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_3_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_3_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_3_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous3[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_3_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous3[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous3[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_3_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous3[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous3[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_3_0_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous3[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous3[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous4 = aie.buffer(%tile_1_2) {address = 32000 : i32, sym_name = "_anonymous4"} : memref<3xi32> 
    %core_1_2 = aie.core(%tile_1_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous4[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous4[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous4[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_4_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous4[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_4_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous4[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_4_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_4_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_4_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_4_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous4[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_4_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous4[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous4[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_4_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous4[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous4[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_4_0_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous4[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous4[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous5 = aie.buffer(%tile_1_3) {address = 32000 : i32, sym_name = "_anonymous5"} : memref<3xi32> 
    %core_1_3 = aie.core(%tile_1_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous5[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous5[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous5[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_5_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous5[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_5_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous5[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_5_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_5_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_5_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_5_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous5[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_5_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous5[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous5[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_5_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous5[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous5[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_5_0_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous5[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous5[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous6 = aie.buffer(%tile_1_4) {address = 32000 : i32, sym_name = "_anonymous6"} : memref<3xi32> 
    %core_1_4 = aie.core(%tile_1_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous6[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous6[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous6[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_6_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous6[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_6_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous6[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_6_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_6_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_6_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_6_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous6[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_6_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous6[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous6[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_6_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous6[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous6[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_6_0_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous6[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous6[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous7 = aie.buffer(%tile_1_5) {address = 32000 : i32, sym_name = "_anonymous7"} : memref<3xi32> 
    %core_1_5 = aie.core(%tile_1_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous7[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous7[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous7[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_7_0_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous7[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_0_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_0_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_0_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_0_7_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous7[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_7_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_7_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_0_7_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_7_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous7[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_0_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_0_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_0_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_0_7_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous7[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous7[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_7_0_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous7[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous7[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_7_0_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous7[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous7[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous8 = aie.buffer(%tile_2_2) {address = 32000 : i32, sym_name = "_anonymous8"} : memref<3xi32> 
    %core_2_2 = aie.core(%tile_2_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous8[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous8[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous8[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_0_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous8[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous8[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_0_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_0_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_0_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_0_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous8[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_0_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous8[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous8[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_0_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous8[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous8[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_0_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous8[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous8[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous9 = aie.buffer(%tile_2_3) {address = 32000 : i32, sym_name = "_anonymous9"} : memref<3xi32> 
    %core_2_3 = aie.core(%tile_2_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous9[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous9[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous9[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_1_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous9[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous9[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_1_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_1_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_1_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_1_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous9[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_1_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous9[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous9[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_1_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous9[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous9[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_1_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous9[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous9[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous10 = aie.buffer(%tile_2_4) {address = 32000 : i32, sym_name = "_anonymous10"} : memref<3xi32> 
    %core_2_4 = aie.core(%tile_2_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous10[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous10[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous10[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_2_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous10[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous10[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_2_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_2_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_2_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_2_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous10[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_2_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous10[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous10[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_2_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous10[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous10[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_2_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous10[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous10[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous11 = aie.buffer(%tile_2_5) {address = 32000 : i32, sym_name = "_anonymous11"} : memref<3xi32> 
    %core_2_5 = aie.core(%tile_2_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous11[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous11[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous11[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_3_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous11[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous11[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_3_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_3_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_3_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_3_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous11[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_3_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous11[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous11[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_3_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous11[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous11[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_3_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous11[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous11[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous12 = aie.buffer(%tile_3_2) {address = 32000 : i32, sym_name = "_anonymous12"} : memref<3xi32> 
    %core_3_2 = aie.core(%tile_3_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous12[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous12[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous12[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_4_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous12[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_4_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous12[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_4_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_4_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_4_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_4_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous12[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_4_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous12[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous12[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_4_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous12[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous12[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_4_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous12[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous12[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous13 = aie.buffer(%tile_3_3) {address = 32000 : i32, sym_name = "_anonymous13"} : memref<3xi32> 
    %core_3_3 = aie.core(%tile_3_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous13[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous13[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous13[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_5_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous13[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_5_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous13[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_5_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_5_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_5_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_5_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous13[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_5_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous13[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous13[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_5_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous13[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous13[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_5_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous13[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous13[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous14 = aie.buffer(%tile_3_4) {address = 32000 : i32, sym_name = "_anonymous14"} : memref<3xi32> 
    %core_3_4 = aie.core(%tile_3_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous14[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous14[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous14[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_6_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous14[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_6_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous14[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_6_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_6_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_6_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_6_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous14[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_6_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous14[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous14[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_6_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous14[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous14[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_6_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous14[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous14[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous15 = aie.buffer(%tile_3_5) {address = 32000 : i32, sym_name = "_anonymous15"} : memref<3xi32> 
    %core_3_5 = aie.core(%tile_3_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous15[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous15[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous15[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_7_1_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous15[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_1_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_1_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_1_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_1_7_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous15[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_7_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_7_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_1_7_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_7_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous15[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_1_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_1_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_1_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_1_7_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous15[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous15[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_7_1_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous15[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous15[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_7_1_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous15[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous15[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous16 = aie.buffer(%tile_4_2) {address = 32000 : i32, sym_name = "_anonymous16"} : memref<3xi32> 
    %core_4_2 = aie.core(%tile_4_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous16[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous16[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous16[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_0_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous16[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous16[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_0_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_0_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_0_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_0_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous16[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_0_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous16[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous16[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_0_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous16[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous16[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_0_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous16[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous16[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous17 = aie.buffer(%tile_4_3) {address = 32000 : i32, sym_name = "_anonymous17"} : memref<3xi32> 
    %core_4_3 = aie.core(%tile_4_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous17[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous17[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous17[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_1_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous17[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous17[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_1_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_1_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_1_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_1_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous17[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_1_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous17[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous17[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_1_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous17[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous17[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_1_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous17[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous17[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous18 = aie.buffer(%tile_4_4) {address = 32000 : i32, sym_name = "_anonymous18"} : memref<3xi32> 
    %core_4_4 = aie.core(%tile_4_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous18[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous18[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous18[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_2_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous18[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous18[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_2_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_2_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_2_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_2_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous18[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_2_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous18[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous18[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_2_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous18[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous18[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_2_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous18[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous18[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous19 = aie.buffer(%tile_4_5) {address = 32000 : i32, sym_name = "_anonymous19"} : memref<3xi32> 
    %core_4_5 = aie.core(%tile_4_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous19[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous19[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous19[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_3_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous19[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous19[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_3_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_3_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_3_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_3_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous19[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_3_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous19[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous19[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_3_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous19[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous19[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_3_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous19[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous19[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous20 = aie.buffer(%tile_5_2) {address = 32000 : i32, sym_name = "_anonymous20"} : memref<3xi32> 
    %core_5_2 = aie.core(%tile_5_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous20[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous20[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous20[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_4_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous20[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_4_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous20[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_4_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_4_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_4_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_4_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous20[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_4_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous20[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous20[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_4_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous20[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous20[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_4_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous20[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous20[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous21 = aie.buffer(%tile_5_3) {address = 32000 : i32, sym_name = "_anonymous21"} : memref<3xi32> 
    %core_5_3 = aie.core(%tile_5_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous21[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous21[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous21[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_5_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous21[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_5_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous21[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_5_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_5_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_5_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_5_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous21[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_5_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous21[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous21[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_5_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous21[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous21[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_5_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous21[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous21[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous22 = aie.buffer(%tile_5_4) {address = 32000 : i32, sym_name = "_anonymous22"} : memref<3xi32> 
    %core_5_4 = aie.core(%tile_5_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous22[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous22[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous22[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_6_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous22[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_6_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous22[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_6_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_6_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_6_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_6_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous22[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_6_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous22[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous22[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_6_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous22[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous22[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_6_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous22[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous22[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous23 = aie.buffer(%tile_5_5) {address = 32000 : i32, sym_name = "_anonymous23"} : memref<3xi32> 
    %core_5_5 = aie.core(%tile_5_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous23[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous23[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous23[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_7_2_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous23[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_2_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_2_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_2_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_2_7_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous23[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_7_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_7_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_2_7_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_7_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous23[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_2_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_2_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_2_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_2_7_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous23[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous23[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_7_2_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous23[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous23[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_7_2_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous23[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous23[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous24 = aie.buffer(%tile_6_2) {address = 32000 : i32, sym_name = "_anonymous24"} : memref<3xi32> 
    %core_6_2 = aie.core(%tile_6_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous24[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous24[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous24[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_0_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous24[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_0_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous24[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_0_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_0_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_0_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_0_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous24[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_0_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_0_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous24[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous24[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_0_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous24[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous24[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_0_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous24[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous24[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous25 = aie.buffer(%tile_6_3) {address = 32000 : i32, sym_name = "_anonymous25"} : memref<3xi32> 
    %core_6_3 = aie.core(%tile_6_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous25[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous25[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous25[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_1_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous25[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_1_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous25[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_1_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_1_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_1_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_1_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous25[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_1_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_1_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous25[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous25[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_1_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous25[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous25[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_1_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous25[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous25[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous26 = aie.buffer(%tile_6_4) {address = 32000 : i32, sym_name = "_anonymous26"} : memref<3xi32> 
    %core_6_4 = aie.core(%tile_6_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous26[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous26[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous26[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_2_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous26[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_2_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous26[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_2_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_2_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_2_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_2_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous26[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_2_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_2_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous26[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous26[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_2_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous26[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous26[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_2_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous26[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous26[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous27 = aie.buffer(%tile_6_5) {address = 32000 : i32, sym_name = "_anonymous27"} : memref<3xi32> 
    %core_6_5 = aie.core(%tile_6_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous27[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous27[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous27[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_3_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous27[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_3_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous27[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_3_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_3_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_3_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_3_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous27[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_3_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_3_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous27[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous27[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_3_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous27[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous27[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_3_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous27[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous27[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous28 = aie.buffer(%tile_7_2) {address = 32000 : i32, sym_name = "_anonymous28"} : memref<3xi32> 
    %core_7_2 = aie.core(%tile_7_2) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous28[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous28[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous28[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_4_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous28[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_4_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_4_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous28[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_4_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_4_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_4_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_4_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous28[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_4_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_4_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous28[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous28[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_4_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous28[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous28[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_4_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous28[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous28[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous29 = aie.buffer(%tile_7_3) {address = 32000 : i32, sym_name = "_anonymous29"} : memref<3xi32> 
    %core_7_3 = aie.core(%tile_7_3) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous29[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous29[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous29[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_5_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous29[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_5_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_5_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous29[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_5_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_5_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_5_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_5_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous29[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_5_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_5_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous29[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous29[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_5_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous29[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous29[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_5_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous29[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous29[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous30 = aie.buffer(%tile_7_4) {address = 32000 : i32, sym_name = "_anonymous30"} : memref<3xi32> 
    %core_7_4 = aie.core(%tile_7_4) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous30[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous30[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous30[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_6_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous30[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_6_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_6_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous30[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_6_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_6_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_6_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_6_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous30[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_6_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_6_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous30[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous30[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_6_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous30[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous30[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_6_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous30[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous30[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    %_anonymous31 = aie.buffer(%tile_7_5) {address = 32000 : i32, sym_name = "_anonymous31"} : memref<3xi32> 
    %core_7_5 = aie.core(%tile_7_5) {
      %c1_i32 = arith.constant 1 : i32
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c16 = arith.constant 16 : index
      %c128 = arith.constant 128 : index
      %c2 = arith.constant 2 : index
      %c1 = arith.constant 1 : index
      %c0_i32 = arith.constant 0 : i32
      %c0 = arith.constant 0 : index
      %c2_i32 = arith.constant 2 : i32
      memref.store %c0_i32, %_anonymous31[%c0] : memref<3xi32>
      memref.store %c0_i32, %_anonymous31[%c1] : memref<3xi32>
      memref.store %c0_i32, %_anonymous31[%c2] : memref<3xi32>
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
      aie.use_lock(%C_L1L2_7_3_prod_lock_0, AcquireGreaterEqual, 1)
      %4 = memref.load %_anonymous31[%c0] : memref<3xi32>
      %5 = arith.index_cast %4 : i32 to index
      %6 = arith.index_cast %5 : index to i64
      cf.switch %6 : i64, [
        default: ^bb7,
        0: ^bb5,
        1: ^bb6
      ]
    ^bb5:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_3_buff_0 : memref<32x64xi32>)
    ^bb6:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_3_buff_1 : memref<32x64xi32>)
    ^bb7:  // pred: ^bb4
      cf.br ^bb8(%C_L1L2_7_3_buff_0 : memref<32x64xi32>)
    ^bb8(%7: memref<32x64xi32>):  // 3 preds: ^bb5, ^bb6, ^bb7
      %collapse_shape = memref.collapse_shape %7 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
      func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
      cf.br ^bb9(%c0 : index)
    ^bb9(%8: index):  // 2 preds: ^bb8, ^bb18
      %9 = arith.cmpi slt, %8, %c128 : index
      cf.cond_br %9, ^bb10, ^bb19
    ^bb10:  // pred: ^bb9
      aie.use_lock(%A_L2L1_3_7_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %10 = memref.load %_anonymous31[%c1] : memref<3xi32>
      %11 = arith.index_cast %10 : i32 to index
      %12 = arith.index_cast %11 : index to i64
      cf.switch %12 : i64, [
        default: ^bb13,
        0: ^bb11,
        1: ^bb12
      ]
    ^bb11:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_7_cons_buff_0 : memref<32x64xi8>)
    ^bb12:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_7_cons_buff_1 : memref<32x64xi8>)
    ^bb13:  // pred: ^bb10
      cf.br ^bb14(%A_L2L1_3_7_cons_buff_0 : memref<32x64xi8>)
    ^bb14(%13: memref<32x64xi8>):  // 3 preds: ^bb11, ^bb12, ^bb13
      aie.use_lock(%B_L2L1_7_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      %14 = memref.load %_anonymous31[%c2] : memref<3xi32>
      %15 = arith.index_cast %14 : i32 to index
      %16 = arith.index_cast %15 : index to i64
      cf.switch %16 : i64, [
        default: ^bb17,
        0: ^bb15,
        1: ^bb16
      ]
    ^bb15:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_3_cons_buff_0 : memref<64x64xi8>)
    ^bb16:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_3_cons_buff_1 : memref<64x64xi8>)
    ^bb17:  // pred: ^bb14
      cf.br ^bb18(%B_L2L1_7_3_cons_buff_0 : memref<64x64xi8>)
    ^bb18(%17: memref<64x64xi8>):  // 3 preds: ^bb15, ^bb16, ^bb17
      %collapse_shape_0 = memref.collapse_shape %13 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
      %collapse_shape_1 = memref.collapse_shape %17 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
      func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_0, %collapse_shape_1, %collapse_shape) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
      aie.use_lock(%A_L2L1_3_7_cons_prod_lock_0, Release, 1)
      %18 = memref.load %_anonymous31[%c1] : memref<3xi32>
      %19 = arith.addi %18, %c1_i32 : i32
      %20 = arith.cmpi sge, %19, %c2_i32 : i32
      %21 = arith.subi %19, %c2_i32 : i32
      %22 = arith.select %20, %21, %19 : i32
      memref.store %22, %_anonymous31[%c1] : memref<3xi32>
      aie.use_lock(%B_L2L1_7_3_cons_prod_lock_0, Release, 1)
      %23 = memref.load %_anonymous31[%c2] : memref<3xi32>
      %24 = arith.addi %23, %c1_i32 : i32
      %25 = arith.cmpi sge, %24, %c2_i32 : i32
      %26 = arith.subi %24, %c2_i32 : i32
      %27 = arith.select %25, %26, %24 : i32
      memref.store %27, %_anonymous31[%c2] : memref<3xi32>
      %28 = arith.addi %8, %c1 : index
      cf.br ^bb9(%28 : index)
    ^bb19:  // pred: ^bb9
      aie.use_lock(%C_L1L2_7_3_cons_lock_0, Release, 1)
      %29 = memref.load %_anonymous31[%c0] : memref<3xi32>
      %30 = arith.addi %29, %c1_i32 : i32
      %31 = arith.cmpi sge, %30, %c2_i32 : i32
      %32 = arith.subi %30, %c2_i32 : i32
      %33 = arith.select %31, %32, %30 : i32
      memref.store %33, %_anonymous31[%c0] : memref<3xi32>
      %34 = arith.addi %2, %c1 : index
      cf.br ^bb3(%34 : index)
    ^bb20:  // pred: ^bb3
      %35 = arith.addi %0, %c1 : index
      cf.br ^bb1(%35 : index)
    ^bb21:  // pred: ^bb1
      aie.end
    } {link_files = ["matmul_i8_i32_42bea9df.o"], stack_size = 3328 : i32}
    aie.runtime_sequence(%arg0: memref<2097152xi8>, %arg1: memref<33554432xi8>, %arg2: memref<1048576xi32>) {
      %0 = aiex.dma_configure_task_for @C_L2L3_0_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 0, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%0)
      %1 = aiex.dma_configure_task_for @A_L3L2_0_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 0, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%1)
      %2 = aiex.dma_configure_task_for @B_L3L2_0_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 0, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%2)
      %3 = aiex.dma_configure_task_for @A_L3L2_0_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 1048576, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%3)
      %4 = aiex.dma_configure_task_for @B_L3L2_0_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 0, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%4)
      %5 = aiex.dma_configure_task_for @C_L2L3_1_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 64, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%5)
      %6 = aiex.dma_configure_task_for @A_L3L2_1_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 262144, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%6)
      %7 = aiex.dma_configure_task_for @B_L3L2_1_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 524288, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%7)
      %8 = aiex.dma_configure_task_for @A_L3L2_1_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 1310720, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%8)
      %9 = aiex.dma_configure_task_for @B_L3L2_1_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 524288, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%9)
      %10 = aiex.dma_configure_task_for @C_L2L3_2_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 128, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%10)
      %11 = aiex.dma_configure_task_for @A_L3L2_2_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 524288, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%11)
      %12 = aiex.dma_configure_task_for @B_L3L2_2_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 1048576, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%12)
      %13 = aiex.dma_configure_task_for @A_L3L2_2_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 1572864, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%13)
      %14 = aiex.dma_configure_task_for @B_L3L2_2_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 1048576, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%14)
      %15 = aiex.dma_configure_task_for @C_L2L3_3_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 192, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%15)
      %16 = aiex.dma_configure_task_for @A_L3L2_3_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 786432, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%16)
      %17 = aiex.dma_configure_task_for @B_L3L2_3_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 1572864, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%17)
      %18 = aiex.dma_configure_task_for @A_L3L2_3_shim_alloc {
        aie.dma_bd(%arg0 : memref<2097152xi8>, 1835008, 262144, [<size = 8, stride = 0>, <size = 128, stride = 64>, <size = 32, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%18)
      %19 = aiex.dma_configure_task_for @B_L3L2_3_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 1572864, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%19)
      %20 = aiex.dma_configure_task_for @C_L2L3_4_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 256, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%20)
      %21 = aiex.dma_configure_task_for @B_L3L2_4_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 2097152, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%21)
      %22 = aiex.dma_configure_task_for @B_L3L2_4_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 2097152, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%22)
      %23 = aiex.dma_configure_task_for @C_L2L3_5_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 320, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%23)
      %24 = aiex.dma_configure_task_for @B_L3L2_5_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 2621440, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%24)
      %25 = aiex.dma_configure_task_for @B_L3L2_5_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 2621440, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%25)
      %26 = aiex.dma_configure_task_for @C_L2L3_6_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 384, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%26)
      %27 = aiex.dma_configure_task_for @B_L3L2_6_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 3145728, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%27)
      %28 = aiex.dma_configure_task_for @B_L3L2_6_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 3145728, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%28)
      %29 = aiex.dma_configure_task_for @C_L2L3_7_shim_alloc {
        aie.dma_bd(%arg2 : memref<1048576xi32>, 448, 65536, [<size = 2, stride = 524288>, <size = 8, stride = 512>, <size = 128, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%29)
      %30 = aiex.dma_configure_task_for @B_L3L2_7_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 3670016, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%30)
      %31 = aiex.dma_configure_task_for @B_L3L2_7_shim_alloc {
        aie.dma_bd(%arg1 : memref<33554432xi8>, 3670016, 524288, [<size = 8, stride = 4194304>, <size = 128, stride = 64>, <size = 64, stride = 8192>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 7 : i32}
      aiex.dma_start_task(%31)
      aiex.dma_await_task(%0)
      aiex.dma_await_task(%5)
      aiex.dma_await_task(%10)
      aiex.dma_await_task(%15)
      aiex.dma_await_task(%20)
      aiex.dma_await_task(%23)
      aiex.dma_await_task(%26)
      aiex.dma_await_task(%29)
      aiex.dma_free_task(%1)
      aiex.dma_free_task(%2)
      aiex.dma_free_task(%3)
      aiex.dma_free_task(%4)
      aiex.dma_free_task(%6)
      aiex.dma_free_task(%7)
      aiex.dma_free_task(%8)
      aiex.dma_free_task(%9)
      aiex.dma_free_task(%11)
      aiex.dma_free_task(%12)
      aiex.dma_free_task(%13)
      aiex.dma_free_task(%14)
      aiex.dma_free_task(%16)
      aiex.dma_free_task(%17)
      aiex.dma_free_task(%18)
      aiex.dma_free_task(%19)
      aiex.dma_free_task(%21)
      aiex.dma_free_task(%22)
      aiex.dma_free_task(%24)
      aiex.dma_free_task(%25)
      aiex.dma_free_task(%27)
      aiex.dma_free_task(%28)
      aiex.dma_free_task(%30)
      aiex.dma_free_task(%31)
    }
    %memtile_dma_0_1 = aie.memtile_dma(%mem_tile_0_1) {
      %0 = aie.dma_start(MM2S, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L3L2_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_0_cons_buff_0 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L3L2_0_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L3L2_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_0_cons_buff_1 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L3L2_0_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%A_L3L2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_0_cons_buff_0 : memref<2048xi8>, 0, 2048) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%A_L3L2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%A_L3L2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_0_cons_buff_1 : memref<2048xi8>, 0, 2048) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%A_L3L2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(S2MM, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L2L3_7_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L2L3_7_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 2, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%C_L2L3_7_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_1, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%C_L2L3_7_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_1, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 3, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%C_L2L3_7_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_2, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_7_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_2, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(S2MM, 4, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%C_L2L3_7_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_3, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_7_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_7_cons_lock_3, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      %6 = aie.dma_start(MM2S, 1, ^bb19, ^bb27)
    ^bb19:  // 2 preds: ^bb18, ^bb26
      aie.use_lock(%C_L2L3_7_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 28 : i32, next_bd_id = 29 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_0, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_7_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 29 : i32, next_bd_id = 30 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_1, Release, 1)
      aie.next_bd ^bb21
    ^bb21:  // pred: ^bb20
      aie.use_lock(%C_L2L3_7_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 30 : i32, next_bd_id = 31 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_2, Release, 1)
      aie.next_bd ^bb22
    ^bb22:  // pred: ^bb21
      aie.use_lock(%C_L2L3_7_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 31 : i32, next_bd_id = 32 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_3, Release, 1)
      aie.next_bd ^bb23
    ^bb23:  // pred: ^bb22
      aie.use_lock(%C_L2L3_7_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 32 : i32, next_bd_id = 33 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_0, Release, 1)
      aie.next_bd ^bb24
    ^bb24:  // pred: ^bb23
      aie.use_lock(%C_L2L3_7_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 33 : i32, next_bd_id = 34 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_1, Release, 1)
      aie.next_bd ^bb25
    ^bb25:  // pred: ^bb24
      aie.use_lock(%C_L2L3_7_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 34 : i32, next_bd_id = 35 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_2, Release, 1)
      aie.next_bd ^bb26
    ^bb26:  // pred: ^bb25
      aie.use_lock(%C_L2L3_7_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_7_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 35 : i32, next_bd_id = 28 : i32}
      aie.use_lock(%C_L2L3_7_prod_lock_3, Release, 1)
      aie.next_bd ^bb19
    ^bb27:  // pred: ^bb18
      aie.end
    }
    %mem_0_2 = aie.mem(%tile_0_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_0_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_0_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_0_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_0_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_0_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_0_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_0_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_0_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_0_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_0_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_0_3 = aie.mem(%tile_0_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_1_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_1_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_1_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_1_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_1_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_1_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_1_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_1_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_1_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_1_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_0_4 = aie.mem(%tile_0_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_2_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_2_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_2_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_2_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_2_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_2_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_0_5 = aie.mem(%tile_0_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_3_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_3_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_3_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_3_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_3_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_3_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_3_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_3_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_3_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_3_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_1_2 = aie.mem(%tile_1_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_4_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_4_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_4_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_4_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_4_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_4_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_4_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_4_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_4_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_4_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_1_3 = aie.mem(%tile_1_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_5_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_5_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_5_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_5_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_5_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_5_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_5_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_5_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_5_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_5_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_1_4 = aie.mem(%tile_1_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_6_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_6_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_6_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_6_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_6_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_6_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_6_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_6_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_6_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_6_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_1_5 = aie.mem(%tile_1_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_0_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_7_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_0_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_0_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_0_7_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_0_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_7_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_0_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_7_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_7_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_0_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_7_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_7_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_0_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_7_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_7_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_0_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_7_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    aie.shim_dma_allocation @A_L3L2_0_shim_alloc(%shim_noc_tile_0_0, MM2S, 0)
    %memtile_dma_2_1 = aie.memtile_dma(%mem_tile_2_1) {
      %0 = aie.dma_start(MM2S, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L3L2_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_1_cons_buff_0 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L3L2_1_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L3L2_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_1_cons_buff_1 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L3L2_1_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%A_L3L2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_1_cons_buff_0 : memref<2048xi8>, 0, 2048) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%A_L3L2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%A_L3L2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_1_cons_buff_1 : memref<2048xi8>, 0, 2048) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%A_L3L2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%B_L3L2_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_2_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%B_L3L2_2_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%B_L3L2_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_2_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%B_L3L2_2_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 1, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%B_L3L2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_2_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%B_L3L2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%B_L3L2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_2_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%B_L3L2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 2, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%C_L2L3_1_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_0, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_1_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_0, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(S2MM, 3, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%C_L2L3_1_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 28 : i32, next_bd_id = 29 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_1, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_1_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 29 : i32, next_bd_id = 28 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_1, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      %6 = aie.dma_start(S2MM, 4, ^bb19, ^bb21)
    ^bb19:  // 2 preds: ^bb18, ^bb20
      aie.use_lock(%C_L2L3_1_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_2, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_1_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_2, Release, 1)
      aie.next_bd ^bb19
    ^bb21:  // pred: ^bb18
      %7 = aie.dma_start(S2MM, 5, ^bb22, ^bb24)
    ^bb22:  // 2 preds: ^bb21, ^bb23
      aie.use_lock(%C_L2L3_1_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 30 : i32, next_bd_id = 31 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_3, Release, 1)
      aie.next_bd ^bb23
    ^bb23:  // pred: ^bb22
      aie.use_lock(%C_L2L3_1_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 31 : i32, next_bd_id = 30 : i32}
      aie.use_lock(%C_L2L3_1_cons_lock_3, Release, 1)
      aie.next_bd ^bb22
    ^bb24:  // pred: ^bb21
      %8 = aie.dma_start(MM2S, 2, ^bb25, ^bb33)
    ^bb25:  // 2 preds: ^bb24, ^bb32
      aie.use_lock(%C_L2L3_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 8 : i32, next_bd_id = 9 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb26
    ^bb26:  // pred: ^bb25
      aie.use_lock(%C_L2L3_1_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 9 : i32, next_bd_id = 10 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_1, Release, 1)
      aie.next_bd ^bb27
    ^bb27:  // pred: ^bb26
      aie.use_lock(%C_L2L3_1_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 10 : i32, next_bd_id = 11 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_2, Release, 1)
      aie.next_bd ^bb28
    ^bb28:  // pred: ^bb27
      aie.use_lock(%C_L2L3_1_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 11 : i32, next_bd_id = 12 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_3, Release, 1)
      aie.next_bd ^bb29
    ^bb29:  // pred: ^bb28
      aie.use_lock(%C_L2L3_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 12 : i32, next_bd_id = 13 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb30
    ^bb30:  // pred: ^bb29
      aie.use_lock(%C_L2L3_1_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 13 : i32, next_bd_id = 14 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_1, Release, 1)
      aie.next_bd ^bb31
    ^bb31:  // pred: ^bb30
      aie.use_lock(%C_L2L3_1_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 14 : i32, next_bd_id = 15 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_2, Release, 1)
      aie.next_bd ^bb32
    ^bb32:  // pred: ^bb31
      aie.use_lock(%C_L2L3_1_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_1_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 15 : i32, next_bd_id = 8 : i32}
      aie.use_lock(%C_L2L3_1_prod_lock_3, Release, 1)
      aie.next_bd ^bb25
    ^bb33:  // pred: ^bb24
      aie.end
    }
    %mem_2_2 = aie.mem(%tile_2_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_0_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_0_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_0_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_0_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_0_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_0_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_0_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_0_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_0_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_0_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_2_3 = aie.mem(%tile_2_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_1_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_1_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_1_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_1_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_1_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_1_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_1_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_1_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_1_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_1_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_2_4 = aie.mem(%tile_2_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_2_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_2_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_2_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_2_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_2_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_2_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_2_5 = aie.mem(%tile_2_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_3_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_3_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_3_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_3_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_3_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_3_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_3_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_3_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_3_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_3_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_3_2 = aie.mem(%tile_3_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_4_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_4_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_4_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_4_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_4_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_4_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_4_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_4_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_4_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_4_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_3_3 = aie.mem(%tile_3_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_5_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_5_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_5_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_5_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_5_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_5_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_5_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_5_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_5_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_5_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_3_4 = aie.mem(%tile_3_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_6_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_6_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_6_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_6_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_6_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_6_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_6_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_6_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_6_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_6_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_3_5 = aie.mem(%tile_3_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_1_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_7_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_1_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_1_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_1_7_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_1_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_7_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_1_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_7_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_7_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_1_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_7_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_7_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_1_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_7_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_7_1_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_1_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_7_1_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    aie.shim_dma_allocation @A_L3L2_1_shim_alloc(%shim_noc_tile_2_0, MM2S, 0)
    %memtile_dma_4_1 = aie.memtile_dma(%mem_tile_4_1) {
      %0 = aie.dma_start(MM2S, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L3L2_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_2_cons_buff_0 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L3L2_2_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L3L2_2_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_2_cons_buff_1 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L3L2_2_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%A_L3L2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_2_cons_buff_0 : memref<2048xi8>, 0, 2048) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%A_L3L2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%A_L3L2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_2_cons_buff_1 : memref<2048xi8>, 0, 2048) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%A_L3L2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%B_L3L2_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_3_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%B_L3L2_3_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%B_L3L2_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_3_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%B_L3L2_3_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 1, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%B_L3L2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_3_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%B_L3L2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%B_L3L2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_3_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%B_L3L2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 2, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%C_L2L3_2_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_0, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_2_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_0, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(S2MM, 3, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%C_L2L3_2_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 28 : i32, next_bd_id = 29 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_1, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_2_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 29 : i32, next_bd_id = 28 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_1, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      %6 = aie.dma_start(S2MM, 4, ^bb19, ^bb21)
    ^bb19:  // 2 preds: ^bb18, ^bb20
      aie.use_lock(%C_L2L3_2_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_2, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_2_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_2, Release, 1)
      aie.next_bd ^bb19
    ^bb21:  // pred: ^bb18
      %7 = aie.dma_start(S2MM, 5, ^bb22, ^bb24)
    ^bb22:  // 2 preds: ^bb21, ^bb23
      aie.use_lock(%C_L2L3_2_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 30 : i32, next_bd_id = 31 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_3, Release, 1)
      aie.next_bd ^bb23
    ^bb23:  // pred: ^bb22
      aie.use_lock(%C_L2L3_2_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 31 : i32, next_bd_id = 30 : i32}
      aie.use_lock(%C_L2L3_2_cons_lock_3, Release, 1)
      aie.next_bd ^bb22
    ^bb24:  // pred: ^bb21
      %8 = aie.dma_start(MM2S, 2, ^bb25, ^bb33)
    ^bb25:  // 2 preds: ^bb24, ^bb32
      aie.use_lock(%C_L2L3_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 8 : i32, next_bd_id = 9 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb26
    ^bb26:  // pred: ^bb25
      aie.use_lock(%C_L2L3_2_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 9 : i32, next_bd_id = 10 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_1, Release, 1)
      aie.next_bd ^bb27
    ^bb27:  // pred: ^bb26
      aie.use_lock(%C_L2L3_2_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 10 : i32, next_bd_id = 11 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_2, Release, 1)
      aie.next_bd ^bb28
    ^bb28:  // pred: ^bb27
      aie.use_lock(%C_L2L3_2_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 11 : i32, next_bd_id = 12 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_3, Release, 1)
      aie.next_bd ^bb29
    ^bb29:  // pred: ^bb28
      aie.use_lock(%C_L2L3_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 12 : i32, next_bd_id = 13 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb30
    ^bb30:  // pred: ^bb29
      aie.use_lock(%C_L2L3_2_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 13 : i32, next_bd_id = 14 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_1, Release, 1)
      aie.next_bd ^bb31
    ^bb31:  // pred: ^bb30
      aie.use_lock(%C_L2L3_2_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 14 : i32, next_bd_id = 15 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_2, Release, 1)
      aie.next_bd ^bb32
    ^bb32:  // pred: ^bb31
      aie.use_lock(%C_L2L3_2_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_2_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 15 : i32, next_bd_id = 8 : i32}
      aie.use_lock(%C_L2L3_2_prod_lock_3, Release, 1)
      aie.next_bd ^bb25
    ^bb33:  // pred: ^bb24
      aie.end
    }
    %mem_4_2 = aie.mem(%tile_4_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_0_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_0_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_0_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_0_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_0_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_0_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_0_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_0_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_0_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_0_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_4_3 = aie.mem(%tile_4_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_1_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_1_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_1_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_1_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_1_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_1_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_1_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_1_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_1_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_1_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_4_4 = aie.mem(%tile_4_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_2_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_2_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_2_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_2_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_2_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_2_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_2_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_2_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_4_5 = aie.mem(%tile_4_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_3_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_3_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_3_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_3_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_3_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_3_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_3_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_3_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_3_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_3_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_5_2 = aie.mem(%tile_5_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_4_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_4_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_4_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_4_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_4_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_4_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_4_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_4_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_4_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_4_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_5_3 = aie.mem(%tile_5_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_5_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_5_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_5_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_5_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_5_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_5_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_5_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_5_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_5_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_5_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_5_4 = aie.mem(%tile_5_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_6_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_6_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_6_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_6_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_6_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_6_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_6_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_6_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_6_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_6_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_5_5 = aie.mem(%tile_5_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_2_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_7_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_2_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_2_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_2_7_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_2_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_7_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_2_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_7_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_7_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_2_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_7_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_7_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_2_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_7_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_7_2_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_2_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_7_2_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    aie.shim_dma_allocation @A_L3L2_2_shim_alloc(%shim_noc_tile_4_0, MM2S, 0)
    %memtile_dma_6_1 = aie.memtile_dma(%mem_tile_6_1) {
      %0 = aie.dma_start(MM2S, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L3L2_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_3_cons_buff_0 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L3L2_3_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L3L2_3_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_3_cons_buff_1 : memref<2048xi8>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L3L2_3_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%A_L3L2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_3_cons_buff_0 : memref<2048xi8>, 0, 2048) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%A_L3L2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%A_L3L2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L3L2_3_cons_buff_1 : memref<2048xi8>, 0, 2048) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%A_L3L2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%B_L3L2_6_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_6_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%B_L3L2_6_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%B_L3L2_6_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_6_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%B_L3L2_6_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 1, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%B_L3L2_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_6_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%B_L3L2_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%B_L3L2_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_6_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%B_L3L2_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 2, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%C_L2L3_5_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_0, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_5_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_0, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(S2MM, 3, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%C_L2L3_5_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 28 : i32, next_bd_id = 29 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_1, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_5_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 29 : i32, next_bd_id = 28 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_1, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      %6 = aie.dma_start(S2MM, 4, ^bb19, ^bb21)
    ^bb19:  // 2 preds: ^bb18, ^bb20
      aie.use_lock(%C_L2L3_5_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_2, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_5_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_2, Release, 1)
      aie.next_bd ^bb19
    ^bb21:  // pred: ^bb18
      %7 = aie.dma_start(S2MM, 5, ^bb22, ^bb24)
    ^bb22:  // 2 preds: ^bb21, ^bb23
      aie.use_lock(%C_L2L3_5_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 30 : i32, next_bd_id = 31 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_3, Release, 1)
      aie.next_bd ^bb23
    ^bb23:  // pred: ^bb22
      aie.use_lock(%C_L2L3_5_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 31 : i32, next_bd_id = 30 : i32}
      aie.use_lock(%C_L2L3_5_cons_lock_3, Release, 1)
      aie.next_bd ^bb22
    ^bb24:  // pred: ^bb21
      %8 = aie.dma_start(MM2S, 2, ^bb25, ^bb33)
    ^bb25:  // 2 preds: ^bb24, ^bb32
      aie.use_lock(%C_L2L3_5_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 8 : i32, next_bd_id = 9 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_0, Release, 1)
      aie.next_bd ^bb26
    ^bb26:  // pred: ^bb25
      aie.use_lock(%C_L2L3_5_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 9 : i32, next_bd_id = 10 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_1, Release, 1)
      aie.next_bd ^bb27
    ^bb27:  // pred: ^bb26
      aie.use_lock(%C_L2L3_5_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 10 : i32, next_bd_id = 11 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_2, Release, 1)
      aie.next_bd ^bb28
    ^bb28:  // pred: ^bb27
      aie.use_lock(%C_L2L3_5_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 11 : i32, next_bd_id = 12 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_3, Release, 1)
      aie.next_bd ^bb29
    ^bb29:  // pred: ^bb28
      aie.use_lock(%C_L2L3_5_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 12 : i32, next_bd_id = 13 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_0, Release, 1)
      aie.next_bd ^bb30
    ^bb30:  // pred: ^bb29
      aie.use_lock(%C_L2L3_5_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 13 : i32, next_bd_id = 14 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_1, Release, 1)
      aie.next_bd ^bb31
    ^bb31:  // pred: ^bb30
      aie.use_lock(%C_L2L3_5_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 14 : i32, next_bd_id = 15 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_2, Release, 1)
      aie.next_bd ^bb32
    ^bb32:  // pred: ^bb31
      aie.use_lock(%C_L2L3_5_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_5_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 15 : i32, next_bd_id = 8 : i32}
      aie.use_lock(%C_L2L3_5_prod_lock_3, Release, 1)
      aie.next_bd ^bb25
    ^bb33:  // pred: ^bb24
      aie.end
    }
    %mem_6_2 = aie.mem(%tile_6_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_0_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_0_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_0_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_0_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_0_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_0_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_0_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_0_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_0_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_0_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_0_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_0_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_6_3 = aie.mem(%tile_6_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_1_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_1_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_1_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_1_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_1_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_1_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_1_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_1_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_1_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_1_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_1_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_1_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_6_4 = aie.mem(%tile_6_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_2_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_2_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_2_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_2_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_2_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_2_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_2_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_2_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_2_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_2_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_2_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_2_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_6_5 = aie.mem(%tile_6_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_3_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_3_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_3_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_3_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_3_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_3_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_3_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_3_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_3_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_3_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_3_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_3_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_7_2 = aie.mem(%tile_7_2) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_4_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_4_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_4_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_4_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_4_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_4_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_4_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_4_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_4_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_4_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_4_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_4_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_7_3 = aie.mem(%tile_7_3) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_5_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_5_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_5_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_5_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_5_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_5_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_5_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_5_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_5_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_5_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_5_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_5_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_7_4 = aie.mem(%tile_7_4) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_6_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_6_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_6_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_6_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_6_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_6_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_6_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_6_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_6_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_6_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_6_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_6_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_6_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_6_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    %mem_7_5 = aie.mem(%tile_7_5) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%A_L2L1_3_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_7_cons_buff_0 : memref<32x64xi8>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%A_L2L1_3_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%A_L2L1_3_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%A_L2L1_3_7_cons_buff_1 : memref<32x64xi8>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%A_L2L1_3_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L2L1_7_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_3_cons_buff_0 : memref<64x64xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L2L1_7_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L2L1_7_3_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L2L1_7_3_cons_buff_1 : memref<64x64xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L2L1_7_3_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 0, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L1L2_7_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_3_buff_0 : memref<32x64xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L1L2_7_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L1L2_7_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L1L2_7_3_buff_1 : memref<32x64xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L1L2_7_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      aie.end
    }
    aie.shim_dma_allocation @A_L3L2_3_shim_alloc(%shim_noc_tile_6_0, MM2S, 0)
    %memtile_dma_3_1 = aie.memtile_dma(%mem_tile_3_1) {
      %0 = aie.dma_start(MM2S, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%B_L3L2_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_0_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%B_L3L2_0_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%B_L3L2_0_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_0_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%B_L3L2_0_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L3L2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_0_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L3L2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L3L2_0_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_0_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L3L2_0_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%B_L3L2_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_1_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%B_L3L2_1_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%B_L3L2_1_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_1_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%B_L3L2_1_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 1, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%B_L3L2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_1_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%B_L3L2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%B_L3L2_1_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_1_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%B_L3L2_1_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 2, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%C_L2L3_0_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_0, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_0_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_0, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(S2MM, 3, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%C_L2L3_0_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 28 : i32, next_bd_id = 29 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_1, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_0_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 29 : i32, next_bd_id = 28 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_1, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      %6 = aie.dma_start(S2MM, 4, ^bb19, ^bb21)
    ^bb19:  // 2 preds: ^bb18, ^bb20
      aie.use_lock(%C_L2L3_0_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_2, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_0_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_2, Release, 1)
      aie.next_bd ^bb19
    ^bb21:  // pred: ^bb18
      %7 = aie.dma_start(S2MM, 5, ^bb22, ^bb24)
    ^bb22:  // 2 preds: ^bb21, ^bb23
      aie.use_lock(%C_L2L3_0_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 30 : i32, next_bd_id = 31 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_3, Release, 1)
      aie.next_bd ^bb23
    ^bb23:  // pred: ^bb22
      aie.use_lock(%C_L2L3_0_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 31 : i32, next_bd_id = 30 : i32}
      aie.use_lock(%C_L2L3_0_cons_lock_3, Release, 1)
      aie.next_bd ^bb22
    ^bb24:  // pred: ^bb21
      %8 = aie.dma_start(MM2S, 2, ^bb25, ^bb33)
    ^bb25:  // 2 preds: ^bb24, ^bb32
      aie.use_lock(%C_L2L3_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 8 : i32, next_bd_id = 9 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb26
    ^bb26:  // pred: ^bb25
      aie.use_lock(%C_L2L3_0_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 9 : i32, next_bd_id = 10 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_1, Release, 1)
      aie.next_bd ^bb27
    ^bb27:  // pred: ^bb26
      aie.use_lock(%C_L2L3_0_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 10 : i32, next_bd_id = 11 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_2, Release, 1)
      aie.next_bd ^bb28
    ^bb28:  // pred: ^bb27
      aie.use_lock(%C_L2L3_0_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 11 : i32, next_bd_id = 12 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_3, Release, 1)
      aie.next_bd ^bb29
    ^bb29:  // pred: ^bb28
      aie.use_lock(%C_L2L3_0_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 12 : i32, next_bd_id = 13 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_0, Release, 1)
      aie.next_bd ^bb30
    ^bb30:  // pred: ^bb29
      aie.use_lock(%C_L2L3_0_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 13 : i32, next_bd_id = 14 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_1, Release, 1)
      aie.next_bd ^bb31
    ^bb31:  // pred: ^bb30
      aie.use_lock(%C_L2L3_0_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 14 : i32, next_bd_id = 15 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_2, Release, 1)
      aie.next_bd ^bb32
    ^bb32:  // pred: ^bb31
      aie.use_lock(%C_L2L3_0_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_0_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 15 : i32, next_bd_id = 8 : i32}
      aie.use_lock(%C_L2L3_0_prod_lock_3, Release, 1)
      aie.next_bd ^bb25
    ^bb33:  // pred: ^bb24
      aie.end
    }
    aie.shim_dma_allocation @B_L3L2_0_shim_alloc(%shim_noc_tile_3_0, MM2S, 0)
    aie.shim_dma_allocation @B_L3L2_1_shim_alloc(%shim_noc_tile_3_0, MM2S, 1)
    aie.shim_dma_allocation @B_L3L2_2_shim_alloc(%shim_noc_tile_2_0, MM2S, 1)
    aie.shim_dma_allocation @B_L3L2_3_shim_alloc(%shim_noc_tile_4_0, MM2S, 1)
    %memtile_dma_5_1 = aie.memtile_dma(%mem_tile_5_1) {
      %0 = aie.dma_start(MM2S, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%B_L3L2_4_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_4_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%B_L3L2_4_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%B_L3L2_4_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_4_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%B_L3L2_4_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L3L2_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_4_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L3L2_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L3L2_4_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_4_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L3L2_4_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(MM2S, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%B_L3L2_5_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_5_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%B_L3L2_5_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%B_L3L2_5_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_5_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%B_L3L2_5_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 1, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%B_L3L2_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_5_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%B_L3L2_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%B_L3L2_5_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_5_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%B_L3L2_5_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 2, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%C_L2L3_4_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_0, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_4_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_0, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(S2MM, 3, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%C_L2L3_4_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 28 : i32, next_bd_id = 29 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_1, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_4_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 29 : i32, next_bd_id = 28 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_1, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      %6 = aie.dma_start(S2MM, 4, ^bb19, ^bb21)
    ^bb19:  // 2 preds: ^bb18, ^bb20
      aie.use_lock(%C_L2L3_4_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_2, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_4_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_2, Release, 1)
      aie.next_bd ^bb19
    ^bb21:  // pred: ^bb18
      %7 = aie.dma_start(S2MM, 5, ^bb22, ^bb24)
    ^bb22:  // 2 preds: ^bb21, ^bb23
      aie.use_lock(%C_L2L3_4_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 30 : i32, next_bd_id = 31 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_3, Release, 1)
      aie.next_bd ^bb23
    ^bb23:  // pred: ^bb22
      aie.use_lock(%C_L2L3_4_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 31 : i32, next_bd_id = 30 : i32}
      aie.use_lock(%C_L2L3_4_cons_lock_3, Release, 1)
      aie.next_bd ^bb22
    ^bb24:  // pred: ^bb21
      %8 = aie.dma_start(MM2S, 2, ^bb25, ^bb33)
    ^bb25:  // 2 preds: ^bb24, ^bb32
      aie.use_lock(%C_L2L3_4_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 8 : i32, next_bd_id = 9 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_0, Release, 1)
      aie.next_bd ^bb26
    ^bb26:  // pred: ^bb25
      aie.use_lock(%C_L2L3_4_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 9 : i32, next_bd_id = 10 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_1, Release, 1)
      aie.next_bd ^bb27
    ^bb27:  // pred: ^bb26
      aie.use_lock(%C_L2L3_4_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 10 : i32, next_bd_id = 11 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_2, Release, 1)
      aie.next_bd ^bb28
    ^bb28:  // pred: ^bb27
      aie.use_lock(%C_L2L3_4_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 11 : i32, next_bd_id = 12 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_3, Release, 1)
      aie.next_bd ^bb29
    ^bb29:  // pred: ^bb28
      aie.use_lock(%C_L2L3_4_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 12 : i32, next_bd_id = 13 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_0, Release, 1)
      aie.next_bd ^bb30
    ^bb30:  // pred: ^bb29
      aie.use_lock(%C_L2L3_4_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 13 : i32, next_bd_id = 14 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_1, Release, 1)
      aie.next_bd ^bb31
    ^bb31:  // pred: ^bb30
      aie.use_lock(%C_L2L3_4_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 14 : i32, next_bd_id = 15 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_2, Release, 1)
      aie.next_bd ^bb32
    ^bb32:  // pred: ^bb31
      aie.use_lock(%C_L2L3_4_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_4_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 15 : i32, next_bd_id = 8 : i32}
      aie.use_lock(%C_L2L3_4_prod_lock_3, Release, 1)
      aie.next_bd ^bb25
    ^bb33:  // pred: ^bb24
      aie.end
    }
    aie.shim_dma_allocation @B_L3L2_4_shim_alloc(%shim_noc_tile_5_0, MM2S, 0)
    aie.shim_dma_allocation @B_L3L2_5_shim_alloc(%shim_noc_tile_5_0, MM2S, 1)
    aie.shim_dma_allocation @B_L3L2_6_shim_alloc(%shim_noc_tile_6_0, MM2S, 1)
    %memtile_dma_1_1 = aie.memtile_dma(%mem_tile_1_1) {
      %0 = aie.dma_start(MM2S, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%B_L3L2_7_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_7_cons_buff_0 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%B_L3L2_7_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%B_L3L2_7_cons_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_7_cons_buff_1 : memref<4096xi8>, 0, 4096, [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%B_L3L2_7_cons_prod_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 0, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%B_L3L2_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_7_cons_buff_0 : memref<4096xi8>, 0, 4096) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%B_L3L2_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%B_L3L2_7_cons_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%B_L3L2_7_cons_buff_1 : memref<4096xi8>, 0, 4096) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%B_L3L2_7_cons_cons_lock_0, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(S2MM, 1, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L2L3_3_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_0, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L2L3_3_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_0, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 2, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%C_L2L3_3_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_1, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%C_L2L3_3_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 5 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_1, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(S2MM, 3, ^bb13, ^bb15)
    ^bb13:  // 2 preds: ^bb12, ^bb14
      aie.use_lock(%C_L2L3_3_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_2, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_3_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_2, Release, 1)
      aie.next_bd ^bb13
    ^bb15:  // pred: ^bb12
      %5 = aie.dma_start(S2MM, 4, ^bb16, ^bb18)
    ^bb16:  // 2 preds: ^bb15, ^bb17
      aie.use_lock(%C_L2L3_3_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_3, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_3_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 7 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_3_cons_lock_3, Release, 1)
      aie.next_bd ^bb16
    ^bb18:  // pred: ^bb15
      %6 = aie.dma_start(MM2S, 1, ^bb19, ^bb27)
    ^bb19:  // 2 preds: ^bb18, ^bb26
      aie.use_lock(%C_L2L3_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 28 : i32, next_bd_id = 29 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_3_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 29 : i32, next_bd_id = 30 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_1, Release, 1)
      aie.next_bd ^bb21
    ^bb21:  // pred: ^bb20
      aie.use_lock(%C_L2L3_3_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 30 : i32, next_bd_id = 31 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_2, Release, 1)
      aie.next_bd ^bb22
    ^bb22:  // pred: ^bb21
      aie.use_lock(%C_L2L3_3_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 31 : i32, next_bd_id = 32 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_3, Release, 1)
      aie.next_bd ^bb23
    ^bb23:  // pred: ^bb22
      aie.use_lock(%C_L2L3_3_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 32 : i32, next_bd_id = 33 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_0, Release, 1)
      aie.next_bd ^bb24
    ^bb24:  // pred: ^bb23
      aie.use_lock(%C_L2L3_3_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 33 : i32, next_bd_id = 34 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_1, Release, 1)
      aie.next_bd ^bb25
    ^bb25:  // pred: ^bb24
      aie.use_lock(%C_L2L3_3_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 34 : i32, next_bd_id = 35 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_2, Release, 1)
      aie.next_bd ^bb26
    ^bb26:  // pred: ^bb25
      aie.use_lock(%C_L2L3_3_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_3_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 35 : i32, next_bd_id = 28 : i32}
      aie.use_lock(%C_L2L3_3_prod_lock_3, Release, 1)
      aie.next_bd ^bb19
    ^bb27:  // pred: ^bb18
      aie.end
    }
    aie.shim_dma_allocation @B_L3L2_7_shim_alloc(%shim_noc_tile_1_0, MM2S, 0)
    aie.shim_dma_allocation @C_L2L3_0_shim_alloc(%shim_noc_tile_3_0, S2MM, 0)
    aie.shim_dma_allocation @C_L2L3_1_shim_alloc(%shim_noc_tile_3_0, S2MM, 1)
    aie.shim_dma_allocation @C_L2L3_2_shim_alloc(%shim_noc_tile_2_0, S2MM, 0)
    aie.shim_dma_allocation @C_L2L3_3_shim_alloc(%shim_noc_tile_4_0, S2MM, 0)
    aie.shim_dma_allocation @C_L2L3_4_shim_alloc(%shim_noc_tile_4_0, S2MM, 1)
    aie.shim_dma_allocation @C_L2L3_5_shim_alloc(%shim_noc_tile_5_0, S2MM, 0)
    %memtile_dma_7_1 = aie.memtile_dma(%mem_tile_7_1) {
      %0 = aie.dma_start(S2MM, 0, ^bb1, ^bb3)
    ^bb1:  // 2 preds: ^bb0, ^bb2
      aie.use_lock(%C_L2L3_6_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 0, 2048) {bd_id = 0 : i32, next_bd_id = 1 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_0, Release, 1)
      aie.next_bd ^bb2
    ^bb2:  // pred: ^bb1
      aie.use_lock(%C_L2L3_6_prod_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 0, 2048) {bd_id = 1 : i32, next_bd_id = 0 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_0, Release, 1)
      aie.next_bd ^bb1
    ^bb3:  // pred: ^bb0
      %1 = aie.dma_start(S2MM, 1, ^bb4, ^bb6)
    ^bb4:  // 2 preds: ^bb3, ^bb5
      aie.use_lock(%C_L2L3_6_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 2048, 2048) {bd_id = 24 : i32, next_bd_id = 25 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_1, Release, 1)
      aie.next_bd ^bb5
    ^bb5:  // pred: ^bb4
      aie.use_lock(%C_L2L3_6_prod_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 2048, 2048) {bd_id = 25 : i32, next_bd_id = 24 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_1, Release, 1)
      aie.next_bd ^bb4
    ^bb6:  // pred: ^bb3
      %2 = aie.dma_start(S2MM, 2, ^bb7, ^bb9)
    ^bb7:  // 2 preds: ^bb6, ^bb8
      aie.use_lock(%C_L2L3_6_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 4096, 2048) {bd_id = 2 : i32, next_bd_id = 3 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_2, Release, 1)
      aie.next_bd ^bb8
    ^bb8:  // pred: ^bb7
      aie.use_lock(%C_L2L3_6_prod_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 4096, 2048) {bd_id = 3 : i32, next_bd_id = 2 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_2, Release, 1)
      aie.next_bd ^bb7
    ^bb9:  // pred: ^bb6
      %3 = aie.dma_start(S2MM, 3, ^bb10, ^bb12)
    ^bb10:  // 2 preds: ^bb9, ^bb11
      aie.use_lock(%C_L2L3_6_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 6144, 2048) {bd_id = 26 : i32, next_bd_id = 27 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_3, Release, 1)
      aie.next_bd ^bb11
    ^bb11:  // pred: ^bb10
      aie.use_lock(%C_L2L3_6_prod_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 6144, 2048) {bd_id = 27 : i32, next_bd_id = 26 : i32}
      aie.use_lock(%C_L2L3_6_cons_lock_3, Release, 1)
      aie.next_bd ^bb10
    ^bb12:  // pred: ^bb9
      %4 = aie.dma_start(MM2S, 0, ^bb13, ^bb21)
    ^bb13:  // 2 preds: ^bb12, ^bb20
      aie.use_lock(%C_L2L3_6_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 4 : i32, next_bd_id = 5 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_0, Release, 1)
      aie.next_bd ^bb14
    ^bb14:  // pred: ^bb13
      aie.use_lock(%C_L2L3_6_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 5 : i32, next_bd_id = 6 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_1, Release, 1)
      aie.next_bd ^bb15
    ^bb15:  // pred: ^bb14
      aie.use_lock(%C_L2L3_6_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 6 : i32, next_bd_id = 7 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_2, Release, 1)
      aie.next_bd ^bb16
    ^bb16:  // pred: ^bb15
      aie.use_lock(%C_L2L3_6_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_0 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 7 : i32, next_bd_id = 8 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_3, Release, 1)
      aie.next_bd ^bb17
    ^bb17:  // pred: ^bb16
      aie.use_lock(%C_L2L3_6_cons_lock_0, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 0, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 8 : i32, next_bd_id = 9 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_0, Release, 1)
      aie.next_bd ^bb18
    ^bb18:  // pred: ^bb17
      aie.use_lock(%C_L2L3_6_cons_lock_1, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 2048, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 9 : i32, next_bd_id = 10 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_1, Release, 1)
      aie.next_bd ^bb19
    ^bb19:  // pred: ^bb18
      aie.use_lock(%C_L2L3_6_cons_lock_2, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 4096, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 10 : i32, next_bd_id = 11 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_2, Release, 1)
      aie.next_bd ^bb20
    ^bb20:  // pred: ^bb19
      aie.use_lock(%C_L2L3_6_cons_lock_3, AcquireGreaterEqual, 1)
      aie.dma_bd(%C_L2L3_6_buff_1 : memref<8192xi32>, 6144, 2048, [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>]) {bd_id = 11 : i32, next_bd_id = 4 : i32}
      aie.use_lock(%C_L2L3_6_prod_lock_3, Release, 1)
      aie.next_bd ^bb13
    ^bb21:  // pred: ^bb12
      aie.end
    }
    aie.shim_dma_allocation @C_L2L3_6_shim_alloc(%shim_noc_tile_5_0, S2MM, 1)
    aie.shim_dma_allocation @C_L2L3_7_shim_alloc(%shim_noc_tile_6_0, S2MM, 0)
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_0_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_0_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_1_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_1_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_2_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_2_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_3_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_3_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_4_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_4_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_5_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_5_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
    aie.packet_flow(15) {
      aie.packet_source<%shim_noc_tile_6_0, TileControl : 0>
      aie.packet_dest<%shim_noc_tile_6_0, South : 0>
    } {keep_pkt_header = true, priority_route = true}
  }
}
