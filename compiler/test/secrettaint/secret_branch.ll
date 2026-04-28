; RUN: opt -S -passes=secrettaint %s | FileCheck %s
;
; A branch whose condition is derived from a secret value must be tagged
; !secret. This is the signal that a future branch-elimination pass will
; use to detect and replace secret-dependent control flow with select
; (cmov) sequences.
;
; Note: the two branch targets themselves are irrelevant — we tag the br
; regardless of what happens in the true/false successors.

@ann = private unnamed_addr constant [7 x i8] c"secret\00", section "llvm.metadata"
@src = private unnamed_addr constant [7 x i8] c"test.c\00", section "llvm.metadata"

; CHECK-LABEL: define i32 @secret_branch
define i32 @secret_branch() {
  %s = alloca i32, align 4
  call void @llvm.var.annotation.p0.p0(ptr %s, ptr @ann, ptr @src, i32 1, ptr null)
  store i32 42, ptr %s, align 4
  %sv = load i32, ptr %s, align 4

  ; icmp on a secret value: condition itself is secret.
  ; CHECK: %cond = icmp eq i32 %sv, 0{{.*}}!secret
  %cond = icmp eq i32 %sv, 0

  ; br on a tainted condition: must be tagged !secret.
  ; CHECK: br i1 %cond{{.*}}!secret
  br i1 %cond, label %true_br, label %false_br

true_br:
  ret i32 1

false_br:
  ret i32 0
}

; Negative case: a branch on a purely public condition must NOT be tagged.
; CHECK-LABEL: define i32 @public_branch
define i32 @public_branch(i32 %x) {
  %cond = icmp eq i32 %x, 0
  ; CHECK: br i1 %cond{{[^!]*$}}
  br i1 %cond, label %true_br, label %false_br

true_br:
  ret i32 1

false_br:
  ret i32 0
}

declare void @llvm.var.annotation.p0.p0(ptr, ptr, ptr, i32, ptr) #0
attributes #0 = { nocallback nofree nosync nounwind willreturn memory(inaccessiblemem: readwrite) }
