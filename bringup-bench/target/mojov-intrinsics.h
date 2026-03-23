#ifndef MOJOV_INTRINSICS_H
#define MOJOV_INTRINSICS_H

#include <stdint.h>
#include "mojov-utils.h"

/* Callers provide _u64e_t and _fp64e_t before including this header. */


/*
 *  Mojo-V intrinsic helper functions...
 */

/* Encodes scalar uint64_t value into an engine integer operand. */
static inline _u64e_t
_enc(uint64_t src)
{
  _u64e_t dst;
  __asm__ volatile (
    "ld x28, (%1)\n\t"
    SDE(x28, %0, 0)
    : : "r"(&dst), "r"(&src)
    : "x28", "memory");
  return dst;
}

/* Encodes scalar double value into an engine floating-point operand. */
static inline _fp64e_t
_fenc(double src)
{
  _fp64e_t dst;
  __asm__ volatile (
    "fld f28, (%1)\n\t"
    FSDE(f28, %0, 0)
    : : "r"(&dst), "r"(&src)
    : "f28", "memory");
  return dst;
}

#define __MOJOV_DEF_BIN_U64(name, insn) \
{ \
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

#define __MOJOV_DEF_BINI_U64(name, insn) \
{ \
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

#define __MOJOV_DEF_BIN_FP64(name, insn) \
{ \
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

#define __MOJOV_DEF_BINI_FP64(name, insn) \
{ \
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

#define __MOJOV_DEF_REL_FP64(name, insn) \
{ \
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

#define __MOJOV_DEF_RELI_FP64(name, insn) \
{ \
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


/*
 *  Mojo-V integer arithmetic intrinsics...
 */

/* Computes integer addition. Example: dst = src1 + src2; */
static inline _u64e_t _add(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_add, "add")
/* Computes integer addition with an immediate. Example: dst = src1 + 42u; */
static inline _u64e_t _addi(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_addi, "add")
/* Computes integer subtraction. Example: dst = src1 - src2; */
static inline _u64e_t _sub(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_sub, "sub")
/* Computes integer subtraction with an immediate. Example: dst = src1 - 42u; */
static inline _u64e_t _subi(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_subi, "sub")
/* Computes signed integer multiplication. Example: dst = src1 * src2; */
static inline _u64e_t _mul(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_mul, "mul")
/* Computes signed integer multiplication with an immediate. Example: dst = src1 * 42u; */
static inline _u64e_t _muli(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_muli, "mul")
/* Computes unsigned integer multiplication. Example: dst = src1 * src2; */
static inline _u64e_t _mulu(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_mulu, "mul")
/* Computes unsigned integer multiplication with an immediate. Example: dst = src1 * 42u; */
static inline _u64e_t _mului(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_mului, "mul")
/* Computes signed integer division. Example: dst = src1 / src2; */
static inline _u64e_t _div(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_div, "div")
/* Computes signed integer division with an immediate. Example: dst = src1 / 42; */
static inline _u64e_t _divi(_u64e_t src1, int64_t src2) __MOJOV_DEF_BINI_U64(_divi, "div")
/* Computes unsigned integer division. Example: dst = src1 / src2; */
static inline _u64e_t _divu(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_divu, "divu")
/* Computes unsigned integer division with an immediate. Example: dst = src1 / 42u; */
static inline _u64e_t _divui(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_divui, "divu")
/* Computes signed integer remainder. Example: dst = src1 % src2; */
static inline _u64e_t _mod(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_mod, "rem")
/* Computes signed integer remainder with an immediate. Example: dst = src1 % 42u; */
static inline _u64e_t _modi(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_modi, "rem")
/* Computes unsigned integer remainder. Example: dst = src1 % src2; */
static inline _u64e_t _modu(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_modu, "remu")
/* Computes unsigned integer remainder with an immediate. Example: dst = src1 % 42; */
static inline _u64e_t _modui(_u64e_t src1, int64_t src2) __MOJOV_DEF_BINI_U64(_modui, "remu")


/*
 *  Mojo-V integer logical operation intrinsics...
 */

/* Computes logical AND after booleanizing both operands. Example: dst = src1 && src2; */
static inline _u64e_t _land(_u64e_t src1, _u64e_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    LDE(x29, %2, 0)
    "snez x30, x28\n\t"
    "snez x31, x29\n\t"
    "and x30, x30, x31\n\t"
    SDE(x30, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes logical AND with an immediate after booleanizing both operands. Example: dst = src1 && 42u; */
static inline _u64e_t _landi(_u64e_t src1, uint64_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    "ld x29, (%2)\n\t"
    "snez x30, x28\n\t"
    "snez x31, x29\n\t"
    "and x30, x30, x31\n\t"
    SDE(x30, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes logical OR after booleanizing both operands. Example: dst = src1 || src2; */
static inline _u64e_t _lor(_u64e_t src1, _u64e_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    LDE(x29, %2, 0)
    "snez x30, x28\n\t"
    "snez x31, x29\n\t"
    "or x30, x30, x31\n\t"
    SDE(x30, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes logical OR with an immediate after booleanizing both operands. Example: dst = src1 || 42u; */
static inline _u64e_t _lori(_u64e_t src1, uint64_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    "ld x29, (%2)\n\t"
    "snez x30, x28\n\t"
    "snez x31, x29\n\t"
    "or x30, x30, x31\n\t"
    SDE(x30, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes logical NOT after booleanizing the operand. Example: dst = !src; */
static inline _u64e_t _lnot(_u64e_t src)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    "sltiu x30, x28, 1\n\t"
    SDE(x30, %0, 0)
    : : "r"(&dst), "r"(&src)
    : "x28", "x30", "memory");
  return dst;
}


/*
 *  Mojo-V bitwise logical operation intrinsics...
 */

/* Computes bitwise AND. Example: dst = src1 & src2; */
static inline _u64e_t _and(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_add, "and")
/* Computes bitwise AND with an immediate. Example: dst = src1 & 42u; */
static inline _u64e_t _andi(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_addi, "and")
/* Computes bitwise OR. Example: dst = src1 | src2; */
static inline _u64e_t _or(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_add, "or")
/* Computes bitwise OR with an immediate. Example: dst = src1 | 42u; */
static inline _u64e_t _ori(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_addi, "or")
/* Computes bitwise XOR. Example: dst = src1 ^ src2; */
static inline _u64e_t _xor(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_add, "xor")
/* Computes bitwise XOR with an immediate. Example: dst = src1 ^ 42u; */
static inline _u64e_t _xori(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_addi, "xor")
/* Computes shift left logical. Example: dst = src1 << src2; */
static inline _u64e_t _sll(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_add, "sll")
/* Computes shift left logical with an immediate. Example: dst = src1 << 3; */
static inline _u64e_t _slli(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_addi, "sll")
/* Computes unsigned shift right logical. Example: dst = (uint64_t)src1 >> (uint64_t)src2; */
static inline _u64e_t _srl(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_add, "srl")
/* Computes unsigned shift right logical with an immediate. Example: (uint64_t)dst = (uint64_t)src1 >> 3; */
static inline _u64e_t _srli(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_addi, "srl")
/* Computes signed shift right arithmetic. Example: dst = (int64_t)src1 >> (int64_t)src2; */
static inline _u64e_t _sra(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_add, "sra")
/* Computes signed shift right arithmetic with an immediate. Example: (int64_t)dst = (int64_t)src1 >> 3; */
static inline _u64e_t _srai(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_addi, "sra")
/* Computes bitwise complement. Example: dst = ~src; */
static inline _u64e_t _neg(_u64e_t src)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    "xori x30, x28, -1\n\t"
    SDE(x30, %0, 0)
    : : "r"(&dst), "r"(&src)
    : "x28", "x30", "memory");
  return dst;
}
/* Computes bitwise complement of an immediate. Example: dst = ~42u; */
static inline _u64e_t _negi(uint64_t src)
{
  _u64e_t dst;
  __asm__ volatile (
    "ld x28, (%1)\n\t"
    "xori x30, x28, -1\n\t"
    SDE(x30, %0, 0)
    : : "r"(&dst), "r"(&src)
    : "x28", "x30", "memory");
  return dst;
}


/*
 *  Mojo-V integer relational operation intrinsics...
 */

/* Computes integer equality. Example: dst = src1 == src2; */
static inline _u64e_t _seq(_u64e_t src1, _u64e_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    LDE(x29, %2, 0)
    "xor x30, x28, x29\n\t"
    "seqz x31, x30\n\t"
    SDE(x31, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes integer equality with an immediate. Example: dst = src1 == 42u; */
static inline _u64e_t _seqi(_u64e_t src1, uint64_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    "ld x29, (%2)\n\t"
    "xor x30, x28, x29\n\t"
    "seqz x31, x30\n\t"
    SDE(x31, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes integer inequality. Example: dst = src1 != src2; */
static inline _u64e_t _sne(_u64e_t src1, _u64e_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    LDE(x29, %2, 0)
    "xor x30, x28, x29\n\t"
    "snez x31, x30\n\t"
    SDE(x31, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes integer inequality with an immediate. Example: dst = src1 != 42u; */
static inline _u64e_t _snei(_u64e_t src1, uint64_t src2)
{
  _u64e_t dst;
  __asm__ volatile (
    LDE(x28, %1, 0)
    "ld x29, (%2)\n\t"
    "xor x30, x28, x29\n\t"
    "snez x31, x30\n\t"
    SDE(x31, %0, 0)
    : : "r"(&dst), "r"(&src1), "r"(&src2)
    : "x28", "x29", "x30", "x31", "memory");
  return dst;
}
/* Computes signed src1 < src2. Example: dst = (int64_t)src1 < (int64_t)src2; */
static inline _u64e_t _slt(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_slti, "slt")
/* Computes signed src1 < an immediate. Example: dst = (int64_t)src1 < 42; */
static inline _u64e_t _slti(_u64e_t src1, int64_t src2) __MOJOV_DEF_BINI_U64(_slt, "slt")
/* Computes unsigned src1 < src2. Example: dst = (uint64_t)src1 < (uint64_t)src2; */
static inline _u64e_t _sltu(_u64e_t src1, _u64e_t src2) __MOJOV_DEF_BIN_U64(_sltu, "sltu")
/* Computes unsigned src1 < an immediate. Example: dst = (uint64_t)src1 < 42u; */
static inline _u64e_t _sltui(_u64e_t src1, uint64_t src2) __MOJOV_DEF_BINI_U64(_sltui, "sltu")
/* Computes signed src1 > src2. Example: dst = (int64_t)src1 > (int64_t)src2; */
static inline _u64e_t _sgt(_u64e_t src1, _u64e_t src2) { return _slt(src2, src1); }
/* Computes signed src1 > an immediate. Example: dst = (int64_t)src1 > 42; */
static inline _u64e_t _sgti(_u64e_t src1, int64_t src2) { _u64e_t tmp = _enc(src2); return _slt(tmp, src1); }
/* Computes unsigned src1 > src2. Example: dst = (uint64_t)src1 > (uint64_t)src2; */
static inline _u64e_t _sgtu(_u64e_t src1, _u64e_t src2) { return _sltu(src2, src1); }
/* Computes unsigned src1 > an immediate. Example: dst = (uint64_t)src1 > 42u; */
static inline _u64e_t _sgtui(_u64e_t src1, uint64_t src2) { _u64e_t tmp = _enc(src2); return _sltu(tmp, src1); }
/* Computes signed src1 <= src2. Example: dst = (int64_t)src1 <= (int64_t)src2; */
static inline _u64e_t _sle(_u64e_t src1, _u64e_t src2) { return _lnot(_slt(src2, src1)); }
/* Computes signed src1 <= an immediate. Example: dst = (int64_t)src1 <= 42; */
static inline _u64e_t _slei(_u64e_t src1, int64_t src2) { return _lnot(_sgti(src1, src2)); }
/* Computes unsigned src1 <= src2. Example: dst = (uint64_t)src1 <= (uint64_t)src2; */
static inline _u64e_t _sleu(_u64e_t src1, _u64e_t src2) { return _lnot(_sltu(src2, src1)); }
/* Computes unsigned src1 <= an immediate. Example: dst = (uint64_t)src1 <= 42u; */
static inline _u64e_t _sleui(_u64e_t src1, uint64_t src2) { return _lnot(_sgtui(src1, src2)); }
/* Computes signed src1 >= src2. Example: dst = (int64_t)src1 >= (int64_t)src2; */
static inline _u64e_t _sge(_u64e_t src1, _u64e_t src2) { return _lnot(_slt(src1, src2)); }
/* Computes signed src1 >= an immediate. Example: dst = (int64_t)src1 >= 42; */
static inline _u64e_t _sgei(_u64e_t src1, int64_t src2) { return _lnot(_slti(src1, src2)); }
/* Computes unsigned src1 >= src2. Example: dst = (uint64_t)src1 >= (uint64_t)src2; */
static inline _u64e_t _sgeu(_u64e_t src1, _u64e_t src2) { return _lnot(_sltu(src1, src2)); }
/* Computes unsigned src1 >= an immediate. Example: dst = (uint64_t)src1 >= 42u; */
static inline _u64e_t _sgeui(_u64e_t src1, uint64_t src2) { return _lnot(_sltui(src1, src2)); }


/*
 *  Mojo-V floating-point arithmetic intrinsics...
 */

/* Computes floating-point addition. Example: dst = src1 + src2; */
static inline _fp64e_t _fadd(_fp64e_t src1, _fp64e_t src2) __MOJOV_DEF_BIN_FP64(_fadd, "fadd.d")
/* Computes floating-point addition with an immediate. Example: dst = src1 + 42.0; */
static inline _fp64e_t _faddi(_fp64e_t src1, double src2) __MOJOV_DEF_BINI_FP64(_faddi, "fadd.d")
/* Computes floating-point subtraction. Example: dst = src1 - src2; */
static inline _fp64e_t _fsub(_fp64e_t src1, _fp64e_t src2) __MOJOV_DEF_BIN_FP64(_fsub, "fsub.d")
/* Computes floating-point subtraction with an immediate. Example: dst = src1 - 42.0; */
static inline _fp64e_t _fsubi(_fp64e_t src1, double src2) __MOJOV_DEF_BINI_FP64(_fsubi, "fsub.d")
/* Computes floating-point multiplication. Example: dst = src1 * src2; */
static inline _fp64e_t _fmul(_fp64e_t src1, _fp64e_t src2) __MOJOV_DEF_BIN_FP64(_fmul, "fmul.d")
/* Computes floating-point multiplication with an immediate. Example: dst = src1 * 42.0; */
static inline _fp64e_t _fmuli(_fp64e_t src1, double src2) __MOJOV_DEF_BINI_FP64(_fmuli, "fmul.d")
/* Computes floating-point division. Example: dst = src1 / src2; */
static inline _fp64e_t _fdiv(_fp64e_t src1, _fp64e_t src2) __MOJOV_DEF_BIN_FP64(_fdiv, "fdiv.d")
/* Computes floating-point division with an immediate. Example: dst = src1 / 42.0; */
static inline _fp64e_t _fdivi(_fp64e_t src1, double src2) __MOJOV_DEF_BINI_FP64(_fdivi, "fdiv.d")
/* Computes the floating-point absolute value. Example: dst = fabs(src); */
static inline _fp64e_t _fabs(_fp64e_t src)
{
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


/*
 *  Mojo-V floating-point relational operation intrinsics...
 */

/* Computes floating-point equality. Example: dst = src1 == src2; */
static inline _u64e_t _fseq(_fp64e_t src1, _fp64e_t src2) __MOJOV_DEF_REL_FP64(_fseq, "feq.d")
/* Computes floating-point equality with an immediate. Example: dst = src1 == 42.0; */
static inline _u64e_t _fseqi(_fp64e_t src1, double src2) __MOJOV_DEF_RELI_FP64(_fseqi, "feq.d")
/* Computes floating-point inequality. Example: dst = src1 != src2; */
static inline _u64e_t _fsne(_fp64e_t src1, _fp64e_t src2) { return _lnot(_fseq(src1, src2)); }
/* Computes floating-point inequality with an immediate. Example: dst = src1 != 42.0; */
static inline _u64e_t _fsnei(_fp64e_t src1, double src2) { return _lnot(_fseqi(src1, src2)); }
/* Computes floating-point less-than. Example: dst = src1 < src2; */
static inline _u64e_t _fslt(_fp64e_t src1, _fp64e_t src2) __MOJOV_DEF_REL_FP64(_fslt, "flt.d")
/* Computes floating-point less-than with an immediate. Example: dst = src1 < 42.0; */
static inline _u64e_t _fslti(_fp64e_t src1, double src2) __MOJOV_DEF_RELI_FP64(_fslti, "flt.d")
/* Computes floating-point less-than-or-equal. Example: dst = src1 <= src2; */
static inline _u64e_t _fsle(_fp64e_t src1, _fp64e_t src2) __MOJOV_DEF_REL_FP64(_fsle, "fle.d")
/* Computes floating-point less-than-or-equal with an immediate. Example: dst = src1 <= 42.0; */
static inline _u64e_t _fslei(_fp64e_t src1, double src2) __MOJOV_DEF_RELI_FP64(_fslei, "fle.d")
/* Computes floating-point greater-than. Example: dst = src1 > src2; */
static inline _u64e_t _fsgt(_fp64e_t src1, _fp64e_t src2) { return _fslt(src2, src1); }
/* Computes floating-point greater-than with an immediate. Example: dst = src1 > 42.0; */
static inline _u64e_t _fsgti(_fp64e_t src1, double src2) { _fp64e_t tmp = _fenc(src2); return _fslt(tmp, src1); }
/* Computes floating-point greater-than-or-equal. Example: dst = src1 >= src2; */
static inline _u64e_t _fsge(_fp64e_t src1, _fp64e_t src2) { return _fsle(src2, src1); }
/* Computes floating-point greater-than-or-equal with an immediate. Example: dst = src1 >= 42.0; */
static inline _u64e_t _fsgei(_fp64e_t src1, double src2) { _fp64e_t tmp = _fenc(src2); return _fsle(tmp, src1); }


/*
 *  Mojo-V conditional move intrinsics...
 */

/* Selects between integer operands using a predicate. Example: dst = predicate ? if_true : if_false; */
static inline _u64e_t _cmov(_u64e_t predicate, _u64e_t if_true, _u64e_t if_false)
{
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
/* Selects between floating-point operands using a predicate. Example: dst = predicate ? if_true : if_false; */
static inline _fp64e_t _fcmov(_u64e_t predicate, _fp64e_t if_true, _fp64e_t if_false)
{
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

#endif /* MOJOV_INTRINSICS_H */

