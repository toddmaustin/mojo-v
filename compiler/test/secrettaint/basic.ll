; RUN: opt -S -passes=secrettaint %s | FileCheck %s
;
; Secret annotation seeds taint on the annotated pointer. Arithmetic
; that consumes a secret value produces a secret value. Pure-public
; operations must NOT be tagged.

@ann = private unnamed_addr constant [7 x i8] c"secret\00", section "llvm.metadata"
@src = private unnamed_addr constant [7 x i8] c"test.c\00", section "llvm.metadata"

; CHECK-LABEL: define i32 @basic
define i32 @basic() {
  %s = alloca i32, align 4
  %p = alloca i32, align 4
  %r = alloca i32, align 4
  call void @llvm.var.annotation.p0.p0(ptr %s, ptr @ann, ptr @src, i32 1, ptr null)

  ; Pointer was annotated — store to it is secret.
  ; CHECK: store i32 42, ptr %s{{.*}}!secret
  store i32 42, ptr %s, align 4

  ; Public store — no metadata at end of line.
  ; CHECK: store i32 7, ptr %p{{[^!]*$}}
  store i32 7, ptr %p, align 4

  ; Load from secret pointer is secret.
  ; CHECK: %sv = load i32, ptr %s{{.*}}!secret
  %sv = load i32, ptr %s, align 4

  ; Load from public pointer is NOT secret.
  ; CHECK: %pv = load i32, ptr %p{{[^!]*$}}
  %pv = load i32, ptr %p, align 4

  ; Arithmetic with one secret operand is secret.
  ; CHECK: %sum = add i32 %sv, %pv{{.*}}!secret
  %sum = add i32 %sv, %pv

  ; Store of secret value taints the destination pointer.
  ; CHECK: store i32 %sum, ptr %r{{.*}}!secret
  store i32 %sum, ptr %r, align 4

  ; Subsequent load from now-tainted pointer is secret.
  ; CHECK: %rv = load i32, ptr %r{{.*}}!secret
  %rv = load i32, ptr %r, align 4
  ret i32 %rv
}

declare void @llvm.var.annotation.p0.p0(ptr, ptr, ptr, i32, ptr) #0
attributes #0 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
