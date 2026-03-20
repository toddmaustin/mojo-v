#ifndef MOJOV_INTRINSICS_H
#define MOJOV_INTRINSICS_H

#include <stdint.h>
#include "mojov-utils.h"

/* Callers provide _u64e_t and _fp64e_t before including this header. */

extern inline void _store(_u64e_t *dst, uint64_t src);
extern inline void _fstore(_fp64e_t *dst, double src);

extern inline _u64e_t _add(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _addi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _sub(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _subi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _mul(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _muli(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _div(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _divi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _mod(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _modi(_u64e_t src1, uint64_t src2);

extern inline _fp64e_t _fadd(_fp64e_t src1, _fp64e_t src2);
extern inline _fp64e_t _faddi(_fp64e_t src1, double src2);
extern inline _fp64e_t _fsub(_fp64e_t src1, _fp64e_t src2);
extern inline _fp64e_t _fsubi(_fp64e_t src1, double src2);
extern inline _fp64e_t _fmul(_fp64e_t src1, _fp64e_t src2);
extern inline _fp64e_t _fmuli(_fp64e_t src1, double src2);
extern inline _fp64e_t _fdiv(_fp64e_t src1, _fp64e_t src2);
extern inline _fp64e_t _fdivi(_fp64e_t src1, double src2);

extern inline _u64e_t _seq(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _seqi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _sne(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _snei(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _slt(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _slti(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _sle(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _slei(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _sgt(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _sgti(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _sge(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _sgei(_u64e_t src1, uint64_t src2);

extern inline _u64e_t _fseq(_fp64e_t src1, _fp64e_t src2);
extern inline _u64e_t _fseqi(_fp64e_t src1, double src2);
extern inline _u64e_t _fsne(_fp64e_t src1, _fp64e_t src2);
extern inline _u64e_t _fsnei(_fp64e_t src1, double src2);
extern inline _u64e_t _fslt(_fp64e_t src1, _fp64e_t src2);
extern inline _u64e_t _fslti(_fp64e_t src1, double src2);
extern inline _u64e_t _fsle(_fp64e_t src1, _fp64e_t src2);
extern inline _u64e_t _fslei(_fp64e_t src1, double src2);
extern inline _u64e_t _fsgt(_fp64e_t src1, _fp64e_t src2);
extern inline _u64e_t _fsgti(_fp64e_t src1, double src2);
extern inline _u64e_t _fsge(_fp64e_t src1, _fp64e_t src2);
extern inline _u64e_t _fsgei(_fp64e_t src1, double src2);

extern inline _u64e_t _land(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _landi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _lor(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _lori(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _lnot(_u64e_t src);

extern inline _u64e_t _and(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _andi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _or(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _ori(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _xor(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _xori(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _lsh(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _lshi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _rsh(_u64e_t src1, _u64e_t src2);
extern inline _u64e_t _rshi(_u64e_t src1, uint64_t src2);
extern inline _u64e_t _neg(_u64e_t src);
extern inline _u64e_t _negi(uint64_t src);

#define _MOJOV_DEF_BIN_U64(name, insn) \
extern inline _u64e_t name(_u64e_t src1, _u64e_t src2) { \
  _u64e_t dst; \
  __asm__ volatile ( \
    LDE(x28, %1, 0) \
    LDE(x29, %2, 0) \
    insn " x30, x28, x29\n\t" \
    SDE(x30, %0, 0) \
    : : "r"(&dst), "r"(&src1), "r"(&src2) \
    : "x28", "x29", "x30", "memory"); \
  return dst; \
}

#define _MOJOV_DEF_BINI_U64(name, insn) \
extern inline _u64e_t name(_u64e_t src1, uint64_t src2) { \
  _u64e_t dst; \
  __asm__ volatile ( \
    LDE(x28, %1, 0) \
    "ld x29, (%2)\n\t" \
    insn " x30, x28, x29\n\t" \
    SDE(x30, %0, 0) \
    : : "r"(&dst), "r"(&src1), "r"(&src2) \
    : "x28", "x29", "x30", "memory"); \
  return dst; \
}

#define _MOJOV_DEF_BIN_FP64(name, insn) \
extern inline _fp64e_t name(_fp64e_t src1, _fp64e_t src2) { \
  _fp64e_t dst; \
  __asm__ volatile ( \
    FLDE(f28, %1, 0) \
    FLDE(f29, %2, 0) \
    insn " f30, f28, f29\n\t" \
    FSDE(f30, %0, 0) \
    : : "r"(&dst), "r"(&src1), "r"(&src2) \
    : "f28", "f29", "f30", "memory"); \
  return dst; \
}

#define _MOJOV_DEF_BINI_FP64(name, insn) \
extern inline _fp64e_t name(_fp64e_t src1, double src2) { \
  _fp64e_t dst; \
  __asm__ volatile ( \
    FLDE(f28, %1, 0) \
    "fld f29, (%2)\n\t" \
    insn " f30, f28, f29\n\t" \
    FSDE(f30, %0, 0) \
    : : "r"(&dst), "r"(&src1), "r"(&src2) \
    : "f28", "f29", "f30", "memory"); \
  return dst; \
}

#define _MOJOV_DEF_REL_U64(name, insn) _MOJOV_DEF_BIN_U64(name, insn)
#define _MOJOV_DEF_RELI_U64(name, insn) _MOJOV_DEF_BINI_U64(name, insn)

#define _MOJOV_DEF_REL_FP64(name, insn) \
extern inline _u64e_t name(_fp64e_t src1, _fp64e_t src2) { \
  _u64e_t dst; \
  __asm__ volatile ( \
    FLDE(f28, %1, 0) \
    FLDE(f29, %2, 0) \
    insn " x28, f28, f29\n\t" \
    SDE(x28, %0, 0) \
    : : "r"(&dst), "r"(&src1), "r"(&src2) \
    : "x28", "f28", "f29", "memory"); \
  return dst; \
}

#define _MOJOV_DEF_RELI_FP64(name, insn) \
extern inline _u64e_t name(_fp64e_t src1, double src2) { \
  _u64e_t dst; \
  __asm__ volatile ( \
    FLDE(f28, %1, 0) \
    "fld f29, (%2)\n\t" \
    insn " x28, f28, f29\n\t" \
    SDE(x28, %0, 0) \
    : : "r"(&dst), "r"(&src1), "r"(&src2) \
    : "x28", "f28", "f29", "memory"); \
  return dst; \
}

extern inline void _store(_u64e_t *dst, uint64_t src) {
  __asm__ volatile ("ld x28, (%1)\n\t" SDE(x28, %0, 0) : : "r"(dst), "r"(&src) : "x28", "memory");
}

extern inline void _fstore(_fp64e_t *dst, double src) {
  __asm__ volatile ("fld f28, (%1)\n\t" FSDE(f28, %0, 0) : : "r"(dst), "r"(&src) : "f28", "memory");
}

_MOJOV_DEF_BIN_U64(_add, "add")
_MOJOV_DEF_BINI_U64(_addi, "add")
_MOJOV_DEF_BIN_U64(_sub, "sub")
_MOJOV_DEF_BINI_U64(_subi, "sub")
_MOJOV_DEF_BIN_U64(_mul, "mul")
_MOJOV_DEF_BINI_U64(_muli, "mul")
_MOJOV_DEF_BIN_U64(_div, "divu")
_MOJOV_DEF_BINI_U64(_divi, "divu")
_MOJOV_DEF_BIN_U64(_mod, "remu")
_MOJOV_DEF_BINI_U64(_modi, "remu")
_MOJOV_DEF_BIN_FP64(_fadd, "fadd.d")
_MOJOV_DEF_BINI_FP64(_faddi, "fadd.d")
_MOJOV_DEF_BIN_FP64(_fsub, "fsub.d")
_MOJOV_DEF_BINI_FP64(_fsubi, "fsub.d")
_MOJOV_DEF_BIN_FP64(_fmul, "fmul.d")
_MOJOV_DEF_BINI_FP64(_fmuli, "fmul.d")
_MOJOV_DEF_BIN_FP64(_fdiv, "fdiv.d")
_MOJOV_DEF_BINI_FP64(_fdivi, "fdiv.d")
_MOJOV_DEF_REL_U64(_slt, "sltu")
_MOJOV_DEF_RELI_U64(_slti, "sltu")
_MOJOV_DEF_REL_FP64(_fseq, "feq.d")
_MOJOV_DEF_RELI_FP64(_fseqi, "feq.d")
_MOJOV_DEF_REL_FP64(_fslt, "flt.d")
_MOJOV_DEF_RELI_FP64(_fslti, "flt.d")
_MOJOV_DEF_REL_FP64(_fsle, "fle.d")
_MOJOV_DEF_RELI_FP64(_fslei, "fle.d")
_MOJOV_DEF_BIN_U64(_and, "and")
_MOJOV_DEF_BINI_U64(_andi, "and")
_MOJOV_DEF_BIN_U64(_or, "or")
_MOJOV_DEF_BINI_U64(_ori, "or")
_MOJOV_DEF_BIN_U64(_xor, "xor")
_MOJOV_DEF_BINI_U64(_xori, "xor")
_MOJOV_DEF_BIN_U64(_lsh, "sll")
_MOJOV_DEF_BINI_U64(_lshi, "sll")
_MOJOV_DEF_BIN_U64(_rsh, "srl")
_MOJOV_DEF_BINI_U64(_rshi, "srl")

extern inline _u64e_t _lnot(_u64e_t src) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) "sltiu x30, x28, 1\n\t" SDE(x30, %0, 0) : : "r"(&dst), "r"(&src) : "x28", "x30", "memory"); return dst; }
extern inline _u64e_t _sgt(_u64e_t src1, _u64e_t src2) { return _slt(src2, src1); }
extern inline _u64e_t _sgti(_u64e_t src1, uint64_t src2) { _u64e_t tmp; _store(&tmp, src2); return _slt(tmp, src1); }
extern inline _u64e_t _sge(_u64e_t src1, _u64e_t src2) { return _lnot(_slt(src1, src2)); }
extern inline _u64e_t _sgei(_u64e_t src1, uint64_t src2) { return _lnot(_slti(src1, src2)); }
extern inline _u64e_t _sle(_u64e_t src1, _u64e_t src2) { return _lnot(_slt(src2, src1)); }
extern inline _u64e_t _slei(_u64e_t src1, uint64_t src2) { return _lnot(_sgti(src1, src2)); }
extern inline _u64e_t _seq(_u64e_t src1, _u64e_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) LDE(x29, %2, 0) "xor x30, x28, x29\n\t" "seqz x31, x30\n\t" SDE(x31, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _seqi(_u64e_t src1, uint64_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) "ld x29, (%2)\n\t" "xor x30, x28, x29\n\t" "seqz x31, x30\n\t" SDE(x31, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _sne(_u64e_t src1, _u64e_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) LDE(x29, %2, 0) "xor x30, x28, x29\n\t" "snez x31, x30\n\t" SDE(x31, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _snei(_u64e_t src1, uint64_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) "ld x29, (%2)\n\t" "xor x30, x28, x29\n\t" "snez x31, x30\n\t" SDE(x31, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _land(_u64e_t src1, _u64e_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) LDE(x29, %2, 0) "snez x30, x28\n\t" "snez x31, x29\n\t" "and x30, x30, x31\n\t" SDE(x30, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _landi(_u64e_t src1, uint64_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) "ld x29, (%2)\n\t" "snez x30, x28\n\t" "snez x31, x29\n\t" "and x30, x30, x31\n\t" SDE(x30, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _lor(_u64e_t src1, _u64e_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) LDE(x29, %2, 0) "snez x30, x28\n\t" "snez x31, x29\n\t" "or x30, x30, x31\n\t" SDE(x30, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _lori(_u64e_t src1, uint64_t src2) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) "ld x29, (%2)\n\t" "snez x30, x28\n\t" "snez x31, x29\n\t" "or x30, x30, x31\n\t" SDE(x30, %0, 0) : : "r"(&dst), "r"(&src1), "r"(&src2) : "x28", "x29", "x30", "x31", "memory"); return dst; }
extern inline _u64e_t _neg(_u64e_t src) { _u64e_t dst; __asm__ volatile ( LDE(x28, %1, 0) "xori x30, x28, -1\n\t" SDE(x30, %0, 0) : : "r"(&dst), "r"(&src) : "x28", "x30", "memory"); return dst; }
extern inline _u64e_t _negi(uint64_t src) { _u64e_t dst; __asm__ volatile ( "ld x28, (%1)\n\t" "xori x30, x28, -1\n\t" SDE(x30, %0, 0) : : "r"(&dst), "r"(&src) : "x28", "x30", "memory"); return dst; }

extern inline _u64e_t _fsgt(_fp64e_t src1, _fp64e_t src2) { return _fslt(src2, src1); }
extern inline _u64e_t _fsgti(_fp64e_t src1, double src2) { _fp64e_t tmp; _fstore(&tmp, src2); return _fslt(tmp, src1); }
extern inline _u64e_t _fsge(_fp64e_t src1, _fp64e_t src2) { return _fsle(src2, src1); }
extern inline _u64e_t _fsgei(_fp64e_t src1, double src2) { _fp64e_t tmp; _fstore(&tmp, src2); return _fsle(tmp, src1); }
extern inline _u64e_t _fsne(_fp64e_t src1, _fp64e_t src2) { return _lnot(_fseq(src1, src2)); }
extern inline _u64e_t _fsnei(_fp64e_t src1, double src2) { return _lnot(_fseqi(src1, src2)); }

extern inline _fp64e_t _fabs(_fp64e_t src);
extern inline _u64e_t _cmov(_u64e_t predicate, _u64e_t if_true, _u64e_t if_false);
extern inline _fp64e_t _fcmov(_u64e_t predicate, _fp64e_t if_true, _fp64e_t if_false);

extern inline _fp64e_t _fabs(_fp64e_t src) {
  _fp64e_t dst;
  __asm__ volatile (
    FLDE(f28, %1, 0)
    "fmv.d.x f29, x0\n\t"
    "flt.d x30, f28, f29\n\t"
    "fneg.d f30, f28\n\t"
    "fmv.x.d x28, f28\n\t"
    "fmv.x.d x29, f30\n\t"
    "czero.eqz x31, x29, x30\n\t"
    "czero.nez x30, x28, x30\n\t"
    "or x31, x30, x31\n\t"
    "fmv.d.x f31, x31\n\t"
    FSDE(f31, %0, 0)
    : : "r"(&dst), "r"(&src)
    : "x28", "x29", "x30", "x31", "f28", "f29", "f30", "f31", "memory");
  return dst;
}

extern inline _u64e_t _cmov(_u64e_t predicate, _u64e_t if_true, _u64e_t if_false) {
  _u64e_t dst;
  __asm__ volatile (
    LDE(x30, %1, 0)
    LDE(x28, %2, 0)
    LDE(x29, %3, 0)
    "czero.eqz x31, x28, x30\n\t"
    "czero.nez x30, x29, x30\n\t"
    "or x31, x30, x31\n\t"
    SDE(x31, %0, 0)
    : : "r"(&dst), "r"(&predicate), "r"(&if_true), "r"(&if_false)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}

extern inline _fp64e_t _fcmov(_u64e_t predicate, _fp64e_t if_true, _fp64e_t if_false) {
  _fp64e_t dst;
  __asm__ volatile (
    LDE(x30, %1, 0)
    FLDE(f28, %2, 0)
    FLDE(f29, %3, 0)
    "fmv.x.d x28, f28\n\t"
    "fmv.x.d x29, f29\n\t"
    "czero.eqz x31, x28, x30\n\t"
    "czero.nez x30, x29, x30\n\t"
    "or x31, x30, x31\n\t"
    "fmv.d.x f30, x31\n\t"
    FSDE(f30, %0, 0)
    : : "r"(&dst), "r"(&predicate), "r"(&if_true), "r"(&if_false)
    : "x28", "x29", "x30", "x31", "f28", "f29", "f30", "memory");
  return dst;
}

#endif
