; RUN: opt -S -passes=secretbranchelim %s | FileCheck %s
;
; A branch on a public (non-secret) condition must be left completely alone.

; CHECK-LABEL: define i32 @public_branch
; CHECK: br i1 %cond, label %true_bb, label %false_bb
; CHECK-NOT: select
define i32 @public_branch(i32 %x) {
entry:
  %cond = icmp eq i32 %x, 0
  br i1 %cond, label %true_bb, label %false_bb

true_bb:
  br label %merge

false_bb:
  br label %merge

merge:
  %result = phi i32 [ 1, %true_bb ], [ 2, %false_bb ]
  ret i32 %result
}
