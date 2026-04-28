; RUN: not opt -S -passes=secretbranchelim %s 2>&1 | FileCheck %s
;
; A store inside a branch body cannot be speculated — executing it on the
; wrong path would corrupt memory. This must be a hard compiler error.

; CHECK: error: secret-dependent branch in 'store_in_branch' cannot be converted to select

define void @store_in_branch(i32 %x, ptr %p) {
entry:
  %cond = icmp eq i32 %x, 0, !secret !0
  br i1 %cond, label %true_bb, label %false_bb, !secret !0

true_bb:
  store i32 1, ptr %p, align 4
  br label %merge

false_bb:
  store i32 2, ptr %p, align 4
  br label %merge

merge:
  ret void
}

!0 = !{}
