module {
  aie.device(npu2) {
    %logical_core = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_0 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_1 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_2 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_3 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_4 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_5 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_6 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_7 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_8 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_9 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_10 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_11 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_12 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_13 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_14 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_15 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_16 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_17 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_18 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_19 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_20 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_21 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_22 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_23 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_24 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_25 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_26 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_27 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_28 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_29 = aie.logical_tile<CoreTile>(?, ?)
    %logical_core_30 = aie.logical_tile<CoreTile>(?, ?)
    %logical_mem = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_31 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_32 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_33 = aie.logical_tile<MemTile>(?, ?)
    %logical_shim_noc = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_34 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_35 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_36 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_mem_37 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_38 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_39 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_40 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_41 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_42 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_43 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_44 = aie.logical_tile<MemTile>(?, ?)
    %logical_shim_noc_45 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_46 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_47 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_48 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_49 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_50 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_51 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_52 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_mem_53 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_54 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_55 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_56 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_57 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_58 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_59 = aie.logical_tile<MemTile>(?, ?)
    %logical_mem_60 = aie.logical_tile<MemTile>(?, ?)
    %logical_shim_noc_61 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_62 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_63 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_64 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_65 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_66 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_67 = aie.logical_tile<ShimNOCTile>(?, ?)
    %logical_shim_noc_68 = aie.logical_tile<ShimNOCTile>(?, ?)
    aie.objectfifo @A_L2L1_0(%logical_mem dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core, %logical_core_0, %logical_core_1, %logical_core_2, %logical_core_3, %logical_core_4, %logical_core_5, %logical_core_6}, 2 : i32) : !aie.objectfifo<memref<32x64xi8>>  
    aie.objectfifo @A_L3L2_0(%logical_shim_noc, {%logical_mem}, 2 : i32) : !aie.objectfifo<memref<2048xi8>>  
    aie.objectfifo.link [@A_L3L2_0] -> [@A_L2L1_0]([] [0])
    aie.objectfifo @A_L2L1_1(%logical_mem_31 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_7, %logical_core_8, %logical_core_9, %logical_core_10, %logical_core_11, %logical_core_12, %logical_core_13, %logical_core_14}, 2 : i32) : !aie.objectfifo<memref<32x64xi8>>  
    aie.objectfifo @A_L3L2_1(%logical_shim_noc_34, {%logical_mem_31}, 2 : i32) : !aie.objectfifo<memref<2048xi8>>  
    aie.objectfifo.link [@A_L3L2_1] -> [@A_L2L1_1]([] [0])
    aie.objectfifo @A_L2L1_2(%logical_mem_32 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_15, %logical_core_16, %logical_core_17, %logical_core_18, %logical_core_19, %logical_core_20, %logical_core_21, %logical_core_22}, 2 : i32) : !aie.objectfifo<memref<32x64xi8>>  
    aie.objectfifo @A_L3L2_2(%logical_shim_noc_35, {%logical_mem_32}, 2 : i32) : !aie.objectfifo<memref<2048xi8>>  
    aie.objectfifo.link [@A_L3L2_2] -> [@A_L2L1_2]([] [0])
    aie.objectfifo @A_L2L1_3(%logical_mem_33 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_23, %logical_core_24, %logical_core_25, %logical_core_26, %logical_core_27, %logical_core_28, %logical_core_29, %logical_core_30}, 2 : i32) : !aie.objectfifo<memref<32x64xi8>>  
    aie.objectfifo @A_L3L2_3(%logical_shim_noc_36, {%logical_mem_33}, 2 : i32) : !aie.objectfifo<memref<2048xi8>>  
    aie.objectfifo.link [@A_L3L2_3] -> [@A_L2L1_3]([] [0])
    aie.objectfifo @B_L2L1_0(%logical_mem_37 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core, %logical_core_7, %logical_core_15, %logical_core_23}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_0(%logical_shim_noc_45, {%logical_mem_37}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_0] -> [@B_L2L1_0]([] [0])
    aie.objectfifo @B_L2L1_1(%logical_mem_38 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_0, %logical_core_8, %logical_core_16, %logical_core_24}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_1(%logical_shim_noc_46, {%logical_mem_38}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_1] -> [@B_L2L1_1]([] [0])
    aie.objectfifo @B_L2L1_2(%logical_mem_39 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_1, %logical_core_9, %logical_core_17, %logical_core_25}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_2(%logical_shim_noc_47, {%logical_mem_39}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_2] -> [@B_L2L1_2]([] [0])
    aie.objectfifo @B_L2L1_3(%logical_mem_40 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_2, %logical_core_10, %logical_core_18, %logical_core_26}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_3(%logical_shim_noc_48, {%logical_mem_40}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_3] -> [@B_L2L1_3]([] [0])
    aie.objectfifo @B_L2L1_4(%logical_mem_41 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_3, %logical_core_11, %logical_core_19, %logical_core_27}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_4(%logical_shim_noc_49, {%logical_mem_41}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_4] -> [@B_L2L1_4]([] [0])
    aie.objectfifo @B_L2L1_5(%logical_mem_42 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_4, %logical_core_12, %logical_core_20, %logical_core_28}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_5(%logical_shim_noc_50, {%logical_mem_42}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_5] -> [@B_L2L1_5]([] [0])
    aie.objectfifo @B_L2L1_6(%logical_mem_43 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_5, %logical_core_13, %logical_core_21, %logical_core_29}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_6(%logical_shim_noc_51, {%logical_mem_43}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_6] -> [@B_L2L1_6]([] [0])
    aie.objectfifo @B_L2L1_7(%logical_mem_44 dimensionsToStream [<size = 8, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_core_6, %logical_core_14, %logical_core_22, %logical_core_30}, 2 : i32) : !aie.objectfifo<memref<64x64xi8>>  
    aie.objectfifo @B_L3L2_7(%logical_shim_noc_52, {%logical_mem_44}, 2 : i32) : !aie.objectfifo<memref<4096xi8>>  
    aie.objectfifo.link [@B_L3L2_7] -> [@B_L2L1_7]([] [0])
    aie.objectfifo @C_L1L2_0_0(%logical_core, {%logical_mem_53}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_0_1(%logical_core_7, {%logical_mem_53}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_0_2(%logical_core_15, {%logical_mem_53}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_0_3(%logical_core_23, {%logical_mem_53}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_0(%logical_mem_53 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_61}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_0_0, @C_L1L2_0_1, @C_L1L2_0_2, @C_L1L2_0_3] -> [@C_L2L3_0]([0, 2048, 4096, 6144] [])
    aie.objectfifo @C_L1L2_1_0(%logical_core_0, {%logical_mem_54}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_1_1(%logical_core_8, {%logical_mem_54}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_1_2(%logical_core_16, {%logical_mem_54}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_1_3(%logical_core_24, {%logical_mem_54}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_1(%logical_mem_54 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_62}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_1_0, @C_L1L2_1_1, @C_L1L2_1_2, @C_L1L2_1_3] -> [@C_L2L3_1]([0, 2048, 4096, 6144] [])
    aie.objectfifo @C_L1L2_2_0(%logical_core_1, {%logical_mem_55}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_2_1(%logical_core_9, {%logical_mem_55}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_2_2(%logical_core_17, {%logical_mem_55}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_2_3(%logical_core_25, {%logical_mem_55}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_2(%logical_mem_55 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_63}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_2_0, @C_L1L2_2_1, @C_L1L2_2_2, @C_L1L2_2_3] -> [@C_L2L3_2]([0, 2048, 4096, 6144] [])
    aie.objectfifo @C_L1L2_3_0(%logical_core_2, {%logical_mem_56}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_3_1(%logical_core_10, {%logical_mem_56}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_3_2(%logical_core_18, {%logical_mem_56}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_3_3(%logical_core_26, {%logical_mem_56}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_3(%logical_mem_56 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_64}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_3_0, @C_L1L2_3_1, @C_L1L2_3_2, @C_L1L2_3_3] -> [@C_L2L3_3]([0, 2048, 4096, 6144] [])
    aie.objectfifo @C_L1L2_4_0(%logical_core_3, {%logical_mem_57}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_4_1(%logical_core_11, {%logical_mem_57}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_4_2(%logical_core_19, {%logical_mem_57}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_4_3(%logical_core_27, {%logical_mem_57}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_4(%logical_mem_57 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_65}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_4_0, @C_L1L2_4_1, @C_L1L2_4_2, @C_L1L2_4_3] -> [@C_L2L3_4]([0, 2048, 4096, 6144] [])
    aie.objectfifo @C_L1L2_5_0(%logical_core_4, {%logical_mem_58}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_5_1(%logical_core_12, {%logical_mem_58}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_5_2(%logical_core_20, {%logical_mem_58}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_5_3(%logical_core_28, {%logical_mem_58}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_5(%logical_mem_58 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_66}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_5_0, @C_L1L2_5_1, @C_L1L2_5_2, @C_L1L2_5_3] -> [@C_L2L3_5]([0, 2048, 4096, 6144] [])
    aie.objectfifo @C_L1L2_6_0(%logical_core_5, {%logical_mem_59}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_6_1(%logical_core_13, {%logical_mem_59}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_6_2(%logical_core_21, {%logical_mem_59}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_6_3(%logical_core_29, {%logical_mem_59}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_6(%logical_mem_59 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_67}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_6_0, @C_L1L2_6_1, @C_L1L2_6_2, @C_L1L2_6_3] -> [@C_L2L3_6]([0, 2048, 4096, 6144] [])
    aie.objectfifo @C_L1L2_7_0(%logical_core_6, {%logical_mem_60}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_7_1(%logical_core_14, {%logical_mem_60}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_7_2(%logical_core_22, {%logical_mem_60}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L1L2_7_3(%logical_core_30, {%logical_mem_60}, 2 : i32) : !aie.objectfifo<memref<32x64xi32>>  
    aie.objectfifo @C_L2L3_7(%logical_mem_60 dimensionsToStream [<size = 4, stride = 512>, <size = 8, stride = 8>, <size = 8, stride = 64>, <size = 8, stride = 1>], {%logical_shim_noc_68}, 2 : i32) : !aie.objectfifo<memref<8192xi32>>  
    aie.objectfifo.link [@C_L1L2_7_0, @C_L1L2_7_1, @C_L1L2_7_2, @C_L1L2_7_3] -> [@C_L2L3_7]([0, 2048, 4096, 6144] [])
    func.func private @zero_i32(memref<2048xi32>) attributes {link_with = "matmul_i8_i32_42bea9df.o"}
    func.func private @"42bea9df_matmul_i8_i32"(memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) attributes {link_with = "matmul_i8_i32_42bea9df.o"}
    %0 = aie.core(%logical_core) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_0_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_0(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_0_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %1 = aie.core(%logical_core_0) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_1_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_1(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_1_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %2 = aie.core(%logical_core_1) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_2_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_2(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_2_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %3 = aie.core(%logical_core_2) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_3_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_3(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_3_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %4 = aie.core(%logical_core_3) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_4_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_4(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_4(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_4_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %5 = aie.core(%logical_core_4) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_5_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_5(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_5(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_5_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %6 = aie.core(%logical_core_5) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_6_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_6(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_6(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_6_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %7 = aie.core(%logical_core_6) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_7_0(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_7(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_0(Consume, 1)
            aie.objectfifo.release @B_L2L1_7(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_7_0(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %8 = aie.core(%logical_core_7) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_0_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_0(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_0_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %9 = aie.core(%logical_core_8) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_1_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_1(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_1_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %10 = aie.core(%logical_core_9) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_2_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_2(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_2_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %11 = aie.core(%logical_core_10) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_3_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_3(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_3_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %12 = aie.core(%logical_core_11) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_4_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_4(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_4(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_4_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %13 = aie.core(%logical_core_12) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_5_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_5(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_5(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_5_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %14 = aie.core(%logical_core_13) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_6_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_6(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_6(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_6_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %15 = aie.core(%logical_core_14) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_7_1(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_7(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_1(Consume, 1)
            aie.objectfifo.release @B_L2L1_7(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_7_1(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %16 = aie.core(%logical_core_15) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_0_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_0(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_0_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %17 = aie.core(%logical_core_16) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_1_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_1(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_1_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %18 = aie.core(%logical_core_17) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_2_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_2(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_2_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %19 = aie.core(%logical_core_18) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_3_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_3(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_3_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %20 = aie.core(%logical_core_19) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_4_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_4(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_4(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_4_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %21 = aie.core(%logical_core_20) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_5_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_5(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_5(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_5_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %22 = aie.core(%logical_core_21) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_6_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_6(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_6(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_6_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %23 = aie.core(%logical_core_22) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_7_2(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_7(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_2(Consume, 1)
            aie.objectfifo.release @B_L2L1_7(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_7_2(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %24 = aie.core(%logical_core_23) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_0_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_0(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_0(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_0_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %25 = aie.core(%logical_core_24) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_1_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_1(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_1(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_1_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %26 = aie.core(%logical_core_25) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_2_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_2(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_2(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_2_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %27 = aie.core(%logical_core_26) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_3_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_3(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_3_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %28 = aie.core(%logical_core_27) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_4_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_4(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_4(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_4_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %29 = aie.core(%logical_core_28) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_5_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_5(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_5(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_5_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %30 = aie.core(%logical_core_29) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_6_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_6(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_6(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_6_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    %31 = aie.core(%logical_core_30) {
      %c0 = arith.constant 0 : index
      %c9223372036854775807 = arith.constant 9223372036854775807 : index
      %c1 = arith.constant 1 : index
      scf.for %arg0 = %c0 to %c9223372036854775807 step %c1 {
        %c0_69 = arith.constant 0 : index
        %c4 = arith.constant 4 : index
        %c1_70 = arith.constant 1 : index
        scf.for %arg1 = %c0_69 to %c4 step %c1_70 {
          %32 = aie.objectfifo.acquire @C_L1L2_7_3(Produce, 1) : !aie.objectfifosubview<memref<32x64xi32>>
          %33 = aie.objectfifo.subview.access %32[0] : !aie.objectfifosubview<memref<32x64xi32>> -> memref<32x64xi32>
          %collapse_shape = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
          func.call @zero_i32(%collapse_shape) : (memref<2048xi32>) -> ()
          %c0_71 = arith.constant 0 : index
          %c64 = arith.constant 64 : index
          %c1_72 = arith.constant 1 : index
          scf.for %arg2 = %c0_71 to %c64 step %c1_72 {
            %34 = aie.objectfifo.acquire @A_L2L1_3(Consume, 1) : !aie.objectfifosubview<memref<32x64xi8>>
            %35 = aie.objectfifo.subview.access %34[0] : !aie.objectfifosubview<memref<32x64xi8>> -> memref<32x64xi8>
            %36 = aie.objectfifo.acquire @B_L2L1_7(Consume, 1) : !aie.objectfifosubview<memref<64x64xi8>>
            %37 = aie.objectfifo.subview.access %36[0] : !aie.objectfifosubview<memref<64x64xi8>> -> memref<64x64xi8>
            %collapse_shape_73 = memref.collapse_shape %35 [[0, 1]] : memref<32x64xi8> into memref<2048xi8>
            %collapse_shape_74 = memref.collapse_shape %37 [[0, 1]] : memref<64x64xi8> into memref<4096xi8>
            %collapse_shape_75 = memref.collapse_shape %33 [[0, 1]] : memref<32x64xi32> into memref<2048xi32>
            func.call @"42bea9df_matmul_i8_i32"(%collapse_shape_73, %collapse_shape_74, %collapse_shape_75) : (memref<2048xi8>, memref<4096xi8>, memref<2048xi32>) -> ()
            aie.objectfifo.release @A_L2L1_3(Consume, 1)
            aie.objectfifo.release @B_L2L1_7(Consume, 1)
          }
          aie.objectfifo.release @C_L1L2_7_3(Produce, 1)
        }
      }
      aie.end
    } {stack_size = 3328 : i32}
    aie.runtime_sequence(%arg0: memref<1048576xi8>, %arg1: memref<4194304xi8>, %arg2: memref<262144xi32>) {
      %32 = aiex.dma_configure_task_for @C_L2L3_0 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 0, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%32)
      %33 = aiex.dma_configure_task_for @A_L3L2_0 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 0, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%33)
      %34 = aiex.dma_configure_task_for @B_L3L2_0 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 0, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%34)
      %35 = aiex.dma_configure_task_for @A_L3L2_0 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 524288, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%35)
      %36 = aiex.dma_configure_task_for @B_L3L2_0 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 0, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%36)
      %37 = aiex.dma_configure_task_for @C_L2L3_1 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 64, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%37)
      %38 = aiex.dma_configure_task_for @A_L3L2_1 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 131072, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%38)
      %39 = aiex.dma_configure_task_for @B_L3L2_1 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 262144, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%39)
      %40 = aiex.dma_configure_task_for @A_L3L2_1 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 655360, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%40)
      %41 = aiex.dma_configure_task_for @B_L3L2_1 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 262144, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%41)
      %42 = aiex.dma_configure_task_for @C_L2L3_2 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 128, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%42)
      %43 = aiex.dma_configure_task_for @A_L3L2_2 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 262144, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%43)
      %44 = aiex.dma_configure_task_for @B_L3L2_2 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 524288, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%44)
      %45 = aiex.dma_configure_task_for @A_L3L2_2 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 786432, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%45)
      %46 = aiex.dma_configure_task_for @B_L3L2_2 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 524288, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%46)
      %47 = aiex.dma_configure_task_for @C_L2L3_3 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 192, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%47)
      %48 = aiex.dma_configure_task_for @A_L3L2_3 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 393216, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%48)
      %49 = aiex.dma_configure_task_for @B_L3L2_3 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 786432, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%49)
      %50 = aiex.dma_configure_task_for @A_L3L2_3 {
        aie.dma_bd(%arg0 : memref<1048576xi8>, 917504, 131072, [<size = 2, stride = 0>, <size = 64, stride = 64>, <size = 32, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%50)
      %51 = aiex.dma_configure_task_for @B_L3L2_3 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 786432, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%51)
      %52 = aiex.dma_configure_task_for @C_L2L3_4 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 256, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%52)
      %53 = aiex.dma_configure_task_for @B_L3L2_4 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1048576, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%53)
      %54 = aiex.dma_configure_task_for @B_L3L2_4 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1048576, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%54)
      %55 = aiex.dma_configure_task_for @C_L2L3_5 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 320, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%55)
      %56 = aiex.dma_configure_task_for @B_L3L2_5 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1310720, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%56)
      %57 = aiex.dma_configure_task_for @B_L3L2_5 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1310720, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%57)
      %58 = aiex.dma_configure_task_for @C_L2L3_6 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 384, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%58)
      %59 = aiex.dma_configure_task_for @B_L3L2_6 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1572864, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%59)
      %60 = aiex.dma_configure_task_for @B_L3L2_6 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1572864, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%60)
      %61 = aiex.dma_configure_task_for @C_L2L3_7 {
        aie.dma_bd(%arg2 : memref<262144xi32>, 448, 16384, [<size = 2, stride = 131072>, <size = 2, stride = 512>, <size = 128, stride = 1024>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {issue_token = true, repeat_count = 1 : i32}
      aiex.dma_start_task(%61)
      %62 = aiex.dma_configure_task_for @B_L3L2_7 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1835008, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%62)
      %63 = aiex.dma_configure_task_for @B_L3L2_7 {
        aie.dma_bd(%arg1 : memref<4194304xi8>, 1835008, 262144, [<size = 2, stride = 2097152>, <size = 64, stride = 64>, <size = 64, stride = 4096>, <size = 64, stride = 1>]) {burst_length = 0 : i32}
        aie.end
      } {repeat_count = 1 : i32}
      aiex.dma_start_task(%63)
      aiex.dma_await_task(%32)
      aiex.dma_await_task(%37)
      aiex.dma_await_task(%42)
      aiex.dma_await_task(%47)
      aiex.dma_await_task(%52)
      aiex.dma_await_task(%55)
      aiex.dma_await_task(%58)
      aiex.dma_await_task(%61)
      aiex.dma_free_task(%33)
      aiex.dma_free_task(%34)
      aiex.dma_free_task(%35)
      aiex.dma_free_task(%36)
      aiex.dma_free_task(%38)
      aiex.dma_free_task(%39)
      aiex.dma_free_task(%40)
      aiex.dma_free_task(%41)
      aiex.dma_free_task(%43)
      aiex.dma_free_task(%44)
      aiex.dma_free_task(%45)
      aiex.dma_free_task(%46)
      aiex.dma_free_task(%48)
      aiex.dma_free_task(%49)
      aiex.dma_free_task(%50)
      aiex.dma_free_task(%51)
      aiex.dma_free_task(%53)
      aiex.dma_free_task(%54)
      aiex.dma_free_task(%56)
      aiex.dma_free_task(%57)
      aiex.dma_free_task(%59)
      aiex.dma_free_task(%60)
      aiex.dma_free_task(%62)
      aiex.dma_free_task(%63)
    }
  }
}
