; ModuleID = 'ir/test.tainted.ll'
source_filename = "src/test.c"
target datalayout = "e-m:o-p270:32:32-p271:32:32-p272:64:64-i64:64-i128:128-n32:64-S128-Fn32"
target triple = "arm64-apple-macosx26.0.0"

@.str = private unnamed_addr constant [7 x i8] c"secret\00", section "llvm.metadata"
@.str.1 = private unnamed_addr constant [11 x i8] c"src/test.c\00", section "llvm.metadata"
@.str.2 = private unnamed_addr constant [15 x i8] c"Hello, world!\0A\00", align 1

; Function Attrs: noinline nounwind ssp uwtable(sync)
define zeroext i8 @foo() #0 {
  %1 = alloca i8, align 1
  %2 = alloca i8, align 1
  %3 = alloca i8, align 1
  %4 = alloca i8, align 1
  %5 = alloca i8, align 1
  call void @llvm.var.annotation.p0.p0(ptr %2, ptr @.str, ptr @.str.1, i32 8, ptr null)
  store i8 55, ptr %2, align 1, !secret !6
  store i8 -127, ptr %3, align 1
  %6 = load i8, ptr %2, align 1, !secret !6
  %7 = zext i8 %6 to i32, !secret !6
  %8 = load i8, ptr %3, align 1
  %9 = zext i8 %8 to i32
  %10 = xor i32 %7, %9, !secret !6
  %11 = trunc i32 %10 to i8, !secret !6
  store i8 %11, ptr %4, align 1, !secret !6
  %12 = load i8, ptr %3, align 1
  %13 = zext i8 %12 to i32
  %14 = add nsw i32 %13, 16
  %15 = trunc i32 %14 to i8
  store i8 %15, ptr %5, align 1
  %16 = load i8, ptr %5, align 1
  %17 = zext i8 %16 to i32
  %18 = icmp eq i32 %17, 113
  br i1 %18, label %19, label %21

19:                                               ; preds = %0
  %20 = load i8, ptr %5, align 1
  store i8 %20, ptr %1, align 1, !secret !6
  br label %23

21:                                               ; preds = %0
  %22 = load i8, ptr %4, align 1, !secret !6
  store i8 %22, ptr %1, align 1, !secret !6
  br label %23

23:                                               ; preds = %21, %19
  %24 = load i8, ptr %1, align 1, !secret !6
  ret i8 %24, !secret !6
}

; Function Attrs: nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite)
declare void @llvm.var.annotation.p0.p0(ptr, ptr, ptr, i32, ptr) #1

; Function Attrs: noinline nounwind ssp uwtable(sync)
define i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca i8, align 1
  store i32 0, ptr %1, align 4
  %3 = call i32 (ptr, ...) @printf(ptr noundef @.str.2)
  %4 = call zeroext i8 @foo(), !secret !6
  store i8 %4, ptr %2, align 1
  ret i32 0
}

declare i32 @printf(ptr noundef, ...) #2

attributes #0 = { noinline nounwind ssp uwtable(sync) "frame-pointer"="non-leaf" "no-trapping-math"="true" "probe-stack"="__chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+bti,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a" }
attributes #1 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
attributes #2 = { "frame-pointer"="non-leaf" "no-trapping-math"="true" "probe-stack"="__chkstk_darwin" "stack-protector-buffer-size"="8" "target-cpu"="apple-m1" "target-features"="+aes,+altnzcv,+bti,+ccdp,+ccidx,+ccpp,+complxnum,+crc,+dit,+dotprod,+flagm,+fp-armv8,+fp16fml,+fptoint,+fullfp16,+jsconv,+lse,+neon,+pauth,+perfmon,+predres,+ras,+rcpc,+rdm,+sb,+sha2,+sha3,+specrestrict,+ssbs,+v8.1a,+v8.2a,+v8.3a,+v8.4a,+v8.5a,+v8a" }

!llvm.module.flags = !{!0, !1, !2, !3, !4}
!llvm.ident = !{!5}

!0 = !{i32 2, !"SDK Version", [2 x i32] [i32 26, i32 4]}
!1 = !{i32 1, !"wchar_size", i32 4}
!2 = !{i32 8, !"PIC Level", i32 2}
!3 = !{i32 7, !"uwtable", i32 1}
!4 = !{i32 7, !"frame-pointer", i32 1}
!5 = !{!"Apple clang version 21.0.0 (clang-2100.0.123.102)"}
!6 = !{}
