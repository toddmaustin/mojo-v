; RUN: not opt -S -passes=secretbranchelim %s 2>&1 | FileCheck %s
;
; A load whose pointer operand is !secret-tagged must be a hard compiler error.
; No safe transformation exists — the address itself leaks via cache timing.

; CHECK: error: secret-dependent memory address in 'secret_load_addr' leaks via cache timing

define i8 @secret_load_addr(ptr %table, i64 %idx) {
entry:
  %tainted_idx = add i64 %idx, 0, !secret !0
  %secret_ptr = getelementptr i8, ptr %table, i64 %tainted_idx, !secret !0
  %val = load i8, ptr %secret_ptr, align 1
  ret i8 %val
}

!0 = !{}
