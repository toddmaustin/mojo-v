; RUN: llc -mtriple=riscv64 -mattr=+m,+f,+d %s -o - | FileCheck %s
;
; SecretConstraint pre-RA pass: an instruction that reads from a
; SecretGPR-constrained virtual register must have its def tightened to
; SecretGPR before register allocation runs.
;
; Without SecretConstraint the add below could be allocated to a public
; register (a0-a7, s0-s11 etc.) even though its source is in x24-x31.
; That is a hardware fault: any instruction reading a secret register must
; also write its result to a secret register.
;
; With SecretConstraint the add destination is constrained to SecretGPR
; before RA, so RA assigns it to x24-x31 and the store becomes SDE.

define void @constraint_propagation(i64 %x, i64 %y, ptr %out) {
  ; %sec_x is forced into SecretGPR (x24-x31) by the mojov.secret intrinsic.
  %sec_x = call i64 @llvm.riscv.mojov.secret.i64(i64 %x)
  ; This add reads %sec_x from a secret register.
  ; SecretConstraint must tighten %sum to SecretGPR before RA.
  ; Without it %sum could land in a public register — hardware fault.
  %sum = add i64 %sec_x, %y
  store volatile i64 %sum, ptr %out
  ret void
}

declare i64 @llvm.riscv.mojov.secret.i64(i64)

; CHECK-LABEL: constraint_propagation:
; The add destination must be in x24-x31 (t3-t6 or s8-s11).
; CHECK: add {{s8|s9|s10|s11|t3|t4|t5|t6}},
; The store must be SDE because the add result is in a secret register.
; CHECK: sde {{s8|s9|s10|s11|t3|t4|t5|t6}},
