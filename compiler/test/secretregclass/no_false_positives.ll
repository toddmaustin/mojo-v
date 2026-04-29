; RUN: opt -S -passes=secretregclass %s | FileCheck %s
;
; Public instructions and void-typed instructions must never be wrapped.

; CHECK-LABEL: define void @no_false_positives
; CHECK-NOT: mojov.secret
define void @no_false_positives(i64 %x, ptr %p) {
  %sum = add i64 %x, 1
  store i64 %sum, ptr %p, align 8
  ret void
}
