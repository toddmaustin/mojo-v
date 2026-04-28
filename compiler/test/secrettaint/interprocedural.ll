; RUN: opt -S -passes=secrettaint %s | FileCheck %s
;
; Interprocedural taint propagation — two directions:
;
; 1. Intraprocedural (same-function): a call whose argument is secret taints
;    the call result immediately, without needing to inspect the callee.
;
; 2. Interprocedural (cross-function): after the module-level fixed-point
;    converges, the callee's parameter is marked secret and its derived
;    values are tagged inside the callee body.

@ann = private unnamed_addr constant [7 x i8] c"secret\00", section "llvm.metadata"
@src = private unnamed_addr constant [7 x i8] c"test.c\00", section "llvm.metadata"

; Callee receives its argument as secret on the second fixed-point iteration
; (once the caller has been analysed and %sv is known to be secret).
; CHECK-LABEL: define i32 @double_val
; CHECK: %doubled = mul i32 %x, 2{{.*}}!secret
define i32 @double_val(i32 %x) {
  %doubled = mul i32 %x, 2
  ret i32 %doubled
}

; Caller annotates a local variable as secret, loads it, and passes it to
; double_val. The call result must be tainted.
; CHECK-LABEL: define i32 @caller
define i32 @caller() {
  %s = alloca i32, align 4
  call void @llvm.var.annotation.p0.p0(ptr %s, ptr @ann, ptr @src, i32 1, ptr null)
  store i32 21, ptr %s, align 4
  ; CHECK: %sv = load i32, ptr %s{{.*}}!secret
  %sv = load i32, ptr %s, align 4
  ; Tainted argument makes the call result tainted.
  ; CHECK: %result = call i32 @double_val{{.*}}!secret
  %result = call i32 @double_val(i32 %sv)
  ret i32 %result
}

declare void @llvm.var.annotation.p0.p0(ptr, ptr, ptr, i32, ptr) #0
attributes #0 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
