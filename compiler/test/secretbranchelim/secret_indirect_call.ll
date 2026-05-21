; RUN: not opt -S -passes=secretbranchelim %s 2>&1 | FileCheck %s
;
; An indirect call through a !secret-tagged function pointer must be a hard
; compiler error. The call target leaks via branch target prediction.
; Using inttoptr of a secret integer avoids conflating this with the
; secret-memory-address check.
; Direct calls to named functions must NOT be flagged.

; CHECK: error: secret-dependent indirect control flow in 'secret_indirect_call' is not transformable

define void @secret_indirect_call(i64 %fn_as_int) {
entry:
  %tainted = add i64 %fn_as_int, 0, !secret !0
  %fn_ptr = inttoptr i64 %tainted to ptr, !secret !0
  call void %fn_ptr()
  ret void
}

!0 = !{}
