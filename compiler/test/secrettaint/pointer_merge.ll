; RUN: opt -S -passes=secrettaint %s | FileCheck %s
;
; Conservative pointer model: taint attaches to the alloca, not to a specific
; store. The fixed-point loop re-processes the whole function body each
; iteration. Once %result enters TaintedPtrs (because the "then" block stores
; a secret value into it), the NEXT iteration re-visits every instruction —
; including the initial public store in "entry" — and tags those too.
;
; Consequence: if an alloca ever holds secret data on any path, ALL stores and
; loads to it are tagged, including ones that precede the secret store in
; program order. This is intentionally over-approximate.

@ann = private unnamed_addr constant [7 x i8] c"secret\00", section "llvm.metadata"
@src = private unnamed_addr constant [7 x i8] c"test.c\00", section "llvm.metadata"

; CHECK-LABEL: define i32 @pointer_merge
define i32 @pointer_merge(i1 %cond) {
entry:
  %result     = alloca i32, align 4
  %secret_var = alloca i32, align 4
  call void @llvm.var.annotation.p0.p0(ptr %secret_var, ptr @ann, ptr @src, i32 1, ptr null)
  ; %result will enter TaintedPtrs on a later fixed-point iteration, so this
  ; public-looking store is retroactively tagged on the next pass over the body.
  ; CHECK: store i32 0, ptr %result{{.*}}!secret
  store i32 0, ptr %result, align 4
  store i32 42, ptr %secret_var, align 4
  br i1 %cond, label %then, label %merge

then:
  %sv = load i32, ptr %secret_var, align 4
  ; This store causes %result to enter TaintedPtrs.
  ; CHECK: store i32 %sv, ptr %result{{.*}}!secret
  store i32 %sv, ptr %result, align 4
  br label %merge

merge:
  ; CHECK: %rv = load i32, ptr %result{{.*}}!secret
  %rv = load i32, ptr %result, align 4
  ret i32 %rv
}

declare void @llvm.var.annotation.p0.p0(ptr, ptr, ptr, i32, ptr) #0
attributes #0 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
