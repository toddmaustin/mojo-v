; RUN: not opt -S -passes=secretbranchelim %s 2>&1 | FileCheck %s
;
; A branch where one side returns early never reconverges — there is no
; merge point, so no select can be constructed. Hard compiler error.

; CHECK: error: secret-dependent branch in 'no_reconvergence' cannot be converted to select

define i32 @no_reconvergence(i32 %x) {
entry:
  %cond = icmp eq i32 %x, 0, !secret !0
  br i1 %cond, label %true_bb, label %false_bb, !secret !0

true_bb:
  ret i32 1

false_bb:
  ret i32 2
}

!0 = !{}
