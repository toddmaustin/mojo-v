; RUN: opt -S -passes=secretregclass %s | FileCheck %s
;
; Every !secret integer instruction must be wrapped with @llvm.riscv.mojov.secret.
; Void instructions and non-integer types must not be wrapped.
; Already-wrapped values must not be double-wrapped.

; CHECK-LABEL: define i64 @wraps_secret_values
define i64 @wraps_secret_values(i64 %x, i64 %y) {
  ; Secret arithmetic: result must be wrapped.
  ; CHECK: %sum = add i64 %x, %y, !secret
  ; CHECK-NEXT: %sum.secret = call i64 @llvm.riscv.mojov.secret.i64(i64 %sum)
  %sum = add i64 %x, %y, !secret !0

  ; Public arithmetic: must NOT be wrapped.
  ; CHECK: %pub = add i64 %x, 1{{$}}
  %pub = add i64 %x, 1

  ; Secret using secret input: wrapped, and uses the wrapped value.
  ; CHECK: %mul = mul i64 %sum.secret, %pub, !secret
  ; CHECK-NEXT: %mul.secret = call i64 @llvm.riscv.mojov.secret.i64(i64 %mul)
  %mul = mul i64 %sum, %pub, !secret !0

  ret i64 %mul
}

!0 = !{}
