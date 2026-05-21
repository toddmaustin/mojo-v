; RUN: opt -S -passes=secretbranchelim %s | FileCheck %s
;
; Loads and stores through public (non-secret) pointers must pass through
; without error. Regression guard against false positives.

; CHECK-LABEL: define i8 @public_load
; CHECK: load i8
define i8 @public_load(ptr %p) {
entry:
  %v = load i8, ptr %p, align 1
  ret i8 %v
}

; CHECK-LABEL: define void @public_store
; CHECK: store i8
define void @public_store(ptr %p, i8 %v) {
entry:
  store i8 %v, ptr %p, align 1
  ret void
}

; CHECK-LABEL: define void @direct_call_not_flagged
; CHECK: call void @some_func
define void @direct_call_not_flagged() {
entry:
  call void @some_func()
  ret void
}

declare void @some_func()
