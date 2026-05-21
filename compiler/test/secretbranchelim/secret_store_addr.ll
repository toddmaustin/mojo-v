; RUN: not opt -S -passes=secretbranchelim %s 2>&1 | FileCheck %s
;
; A store whose pointer operand is !secret-tagged must be a hard compiler error.
; Storing a public value to a secret-derived address still leaks the address
; via cache timing — the stored value being public does not make it safe.

; CHECK: error: secret-dependent memory address in 'secret_store_addr' leaks via cache timing

define void @secret_store_addr(ptr %table, i8 %public_val, i64 %idx) {
entry:
  %tainted_idx = add i64 %idx, 0, !secret !0
  %secret_ptr = getelementptr i8, ptr %table, i64 %tainted_idx, !secret !0
  store i8 %public_val, ptr %secret_ptr, align 1
  ret void
}

!0 = !{}
