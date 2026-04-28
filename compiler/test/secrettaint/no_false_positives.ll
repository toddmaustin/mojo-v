; RUN: opt -S -passes=secrettaint %s | FileCheck %s
;
; A function with no secret annotations must produce zero !secret tags.
; This guards against the pass spuriously tainting public computation.

; CHECK-LABEL: define i32 @public_only
; CHECK-NOT: !secret
define i32 @public_only() {
  %a = alloca i32, align 4
  %b = alloca i32, align 4
  %r = alloca i32, align 4
  store i32 10, ptr %a, align 4
  store i32 20, ptr %b, align 4
  %av = load i32, ptr %a, align 4
  %bv = load i32, ptr %b, align 4
  %sum = add i32 %av, %bv
  %prod = mul i32 %av, %bv
  %result = add i32 %sum, %prod
  store i32 %result, ptr %r, align 4
  %rv = load i32, ptr %r, align 4
  ret i32 %rv
}
