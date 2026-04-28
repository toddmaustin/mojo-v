; RUN: opt -S -passes=secretbranchelim %s | FileCheck %s
;
; A secret-dependent branch over a pure diamond must be replaced with a
; select. After transformation:
;   - no conditional branch survives in the function
;   - the phi node is gone, replaced by a select
;   - instructions from the side blocks are hoisted into the header

; CHECK-LABEL: define i32 @simple_diamond
define i32 @simple_diamond(i32 %x) {
entry:
  %cond = icmp eq i32 %x, 0, !secret !0
  ; CHECK-NOT: br i1
  ; CHECK: %tv = add i32 %x, 1
  ; CHECK: %fv = add i32 %x, 2
  ; CHECK: %result.sel = select i1 %cond, i32 %tv, i32 %fv
  ; CHECK: br label %merge
  br i1 %cond, label %true_bb, label %false_bb, !secret !0

true_bb:
  %tv = add i32 %x, 1
  br label %merge

false_bb:
  %fv = add i32 %x, 2
  br label %merge

merge:
  ; CHECK-NOT: phi
  %result = phi i32 [ %tv, %true_bb ], [ %fv, %false_bb ]
  ret i32 %result
}

!0 = !{}
