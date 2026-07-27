; ModuleID = 'LLVMDialectModule'
source_filename = "LLVMDialectModule"
target triple = "aie2p"

@_anonymous0 = external global [3 x i32]
@inB_cons_buff_1 = external global [64 x [64 x i8]]
@inB_cons_buff_0 = external global [64 x [64 x i8]]
@memA_cons_buff_1 = external global [16 x [64 x i8]]
@memA_cons_buff_0 = external global [16 x [64 x i8]]
@memC_cons_buff_1 = external global [16 x [64 x i32]]
@memC_cons_buff_0 = external global [16 x [64 x i32]]
@memB_cons_buff_1 = external global [64 x [64 x i8]]
@memB_cons_buff_0 = external global [64 x [64 x i8]]
@memC_buff_1 = external global [16 x [64 x i32]]
@memC_buff_0 = external global [16 x [64 x i32]]
@inA_cons_buff_1 = external global [16 x [64 x i8]]
@inA_cons_buff_0 = external global [16 x [64 x i8]]

declare void @debug_i32(i32)

; Unknown intrinsic
declare void @llvm.aie2p.event(i32)

; Unknown intrinsic
declare void @llvm.aie2p.put.ms(i32, i32)

; Unknown intrinsic
declare { i32, i32 } @llvm.aie2p.get.ss()

; Unknown intrinsic
declare void @llvm.aie2p.mcd.write.vec(<16 x i32>, i32)

; Unknown intrinsic
declare <16 x i32> @llvm.aie2p.scd.read.vec(i32)

; Unknown intrinsic
declare void @llvm.aie2p.acquire(i32, i32)

; Unknown intrinsic
declare void @llvm.aie2p.release(i32, i32)

; Unknown intrinsic
declare void @llvm.aie2p.set.ctrl.reg(i32, i32)

declare void @zero_i32(ptr)

declare void @e5b22372_matmul_i8_i32(ptr, ptr, ptr)

define void @core_0_2() {
  store i32 0, ptr @_anonymous0, align 4
  store i32 0, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 4), align 4
  store i32 0, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 8), align 4
  br label %1

1:                                                ; preds = %48, %0
  %2 = phi i64 [ %49, %48 ], [ 0, %0 ]
  %3 = icmp slt i64 %2, 9223372036854775807
  br i1 %3, label %4, label %50

4:                                                ; preds = %41, %1
  %5 = phi i64 [ %47, %41 ], [ 0, %1 ]
  %6 = icmp slt i64 %5, 128
  br i1 %6, label %7, label %48

7:                                                ; preds = %4
  call void @llvm.aie2p.acquire(i32 52, i32 -1)
  %8 = load i32, ptr @_anonymous0, align 4
  %9 = sext i32 %8 to i64
  switch i64 %9, label %10 [
    i64 0, label %51
    i64 1, label %53
  ]

10:                                               ; preds = %51, %53, %7
  %11 = phi ptr [ %54, %53 ], [ %52, %51 ], [ @memC_buff_0, %7 ]
  %12 = getelementptr [16 x [64 x i32]], ptr %11, i32 0, i32 0, i32 0
  br label %13

13:                                               ; preds = %10
  call void @zero_i32(ptr %12)
  br label %14

14:                                               ; preds = %29, %13
  %15 = phi i64 [ %40, %29 ], [ 0, %13 ]
  %16 = icmp slt i64 %15, 128
  br i1 %16, label %17, label %41

17:                                               ; preds = %14
  call void @llvm.aie2p.acquire(i32 49, i32 -1)
  %18 = load i32, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 4), align 4
  %19 = sext i32 %18 to i64
  switch i64 %19, label %20 [
    i64 0, label %55
    i64 1, label %57
  ]

20:                                               ; preds = %55, %57, %17
  %21 = phi ptr [ %58, %57 ], [ %56, %55 ], [ @memA_cons_buff_0, %17 ]
  %22 = getelementptr [16 x [64 x i8]], ptr %21, i32 0, i32 0, i32 0
  br label %23

23:                                               ; preds = %20
  call void @llvm.aie2p.acquire(i32 51, i32 -1)
  %24 = load i32, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 8), align 4
  %25 = sext i32 %24 to i64
  switch i64 %25, label %26 [
    i64 0, label %59
    i64 1, label %61
  ]

26:                                               ; preds = %59, %61, %23
  %27 = phi ptr [ %62, %61 ], [ %60, %59 ], [ @memB_cons_buff_0, %23 ]
  %28 = getelementptr [64 x [64 x i8]], ptr %27, i32 0, i32 0, i32 0
  br label %29

29:                                               ; preds = %26
  call void @e5b22372_matmul_i8_i32(ptr %22, ptr %28, ptr %12)
  call void @llvm.aie2p.release(i32 48, i32 1)
  %30 = load i32, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 4), align 4
  %31 = add i32 %30, 1
  %32 = icmp sge i32 %31, 2
  %33 = add i32 %30, -1
  %34 = select i1 %32, i32 %33, i32 %31
  store i32 %34, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 4), align 4
  call void @llvm.aie2p.release(i32 50, i32 1)
  %35 = load i32, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 8), align 4
  %36 = add i32 %35, 1
  %37 = icmp sge i32 %36, 2
  %38 = add i32 %35, -1
  %39 = select i1 %37, i32 %38, i32 %36
  store i32 %39, ptr getelementptr inbounds nuw (i8, ptr @_anonymous0, i64 8), align 4
  %40 = add i64 %15, 1
  br label %14

41:                                               ; preds = %14
  call void @llvm.aie2p.release(i32 53, i32 1)
  %42 = load i32, ptr @_anonymous0, align 4
  %43 = add i32 %42, 1
  %44 = icmp sge i32 %43, 2
  %45 = add i32 %42, -1
  %46 = select i1 %44, i32 %45, i32 %43
  store i32 %46, ptr @_anonymous0, align 4
  %47 = add i64 %5, 1
  br label %4

48:                                               ; preds = %4
  %49 = add i64 %2, 1
  br label %1

50:                                               ; preds = %1
  ret void

51:                                               ; preds = %7
  %52 = phi ptr [ @memC_buff_0, %7 ]
  br label %10

53:                                               ; preds = %7
  %54 = phi ptr [ @memC_buff_1, %7 ]
  br label %10

55:                                               ; preds = %17
  %56 = phi ptr [ @memA_cons_buff_0, %17 ]
  br label %20

57:                                               ; preds = %17
  %58 = phi ptr [ @memA_cons_buff_1, %17 ]
  br label %20

59:                                               ; preds = %23
  %60 = phi ptr [ @memB_cons_buff_0, %23 ]
  br label %26

61:                                               ; preds = %23
  %62 = phi ptr [ @memB_cons_buff_1, %23 ]
  br label %26
}

!llvm.module.flags = !{!0}

!0 = !{i32 2, !"Debug Info Version", i32 3}
