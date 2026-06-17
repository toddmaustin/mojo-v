; RUN: llc -mtriple=riscv64 -mattr=+m,+f,+d %s -o - | FileCheck %s
;
; After the backend lowers @llvm.riscv.mojov.secret, the result must be
; allocated to a register in the secret range x24-x31, and remain there
; through the store. Using `store volatile` prevents round-trip elimination.

define void @secret_in_secret_reg(i64 %x, i64 %y, ptr %out) {
  %sum = add i64 %x, %y
  ; The wrapper forces the result into SecretGPR (x24-x31).
  %sec = call i64 @llvm.riscv.mojov.secret.i64(i64 %sum)
  store volatile i64 %sec, ptr %out
  ret void
}

declare i64 @llvm.riscv.mojov.secret.i64(i64)

; CHECK-LABEL: secret_in_secret_reg:
; CHECK: add
; The sde source register must be in the secret range (x24-x31).
; CHECK: sde {{s8|s9|s10|s11|t3|t4|t5|t6}},
