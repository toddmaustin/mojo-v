; RUN: not opt -S -passes=secretbranchelim %s 2>&1 | FileCheck %s
;
; An indirectbr whose address is !secret-tagged must be a hard compiler error.
; The jump target leaks secret information via branch target prediction.
; Using inttoptr of a secret integer avoids conflating this with the
; secret-memory-address check.

; CHECK: error: secret-dependent indirect control flow in 'secret_indirect_br' is not transformable

define void @secret_indirect_br(i64 %addr) {
entry:
  %tainted = add i64 %addr, 0, !secret !0
  %secret_ptr = inttoptr i64 %tainted to ptr, !secret !0
  indirectbr ptr %secret_ptr, [label %bb1, label %bb2]

bb1:
  ret void
bb2:
  ret void
}

!0 = !{}
