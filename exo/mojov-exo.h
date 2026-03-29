#ifndef EXO_H
#define EXO_H

#if !defined(__cplusplus)
#include "mojov-intrinsics.h"
#else

// #include <cstdint>
#include <type_traits>

#include "../bringup-bench/target/mojov-utils.h"

#ifndef EXO_UINT64E_STORAGE_TYPE
#define EXO_UINT64E_STORAGE_TYPE mojov_mem_fast_u64_t
#endif

#ifndef EXO_FP64E_STORAGE_TYPE
#define EXO_FP64E_STORAGE_TYPE mojov_mem_fast_fp64_t
#endif

#ifndef EXO_HAS_RAW_TYPES
typedef EXO_UINT64E_STORAGE_TYPE _uint64e_t;
typedef EXO_FP64E_STORAGE_TYPE _fp64e_t;
#endif

#include "mojov-intrinsics.h"

namespace exo {

class uint64e_t;
class fp64e_t;

namespace detail {

using uint_storage_t = _uint64e_t;
using fp_storage_t = _fp64e_t;

static_assert(std::is_trivially_copyable<uint_storage_t>::value,
              "_uint64e_t must be trivially copyable");
static_assert(std::is_trivially_copyable<fp_storage_t>::value,
              "_fp64e_t must be trivially copyable");

inline uint_storage_t zero_uint64e() { return _enc(0u); }
inline fp_storage_t zero_fp64e() { return _fenc(0.0); }

}  // namespace detail

/* Wraps a Mojo-V encrypted 64-bit integer so C++ expressions map to intrinsics.
 * Example: uint64e_t total = 4u; */
class uint64e_t {
public:
  using value_type = uint64_t;
  using storage_type = detail::uint_storage_t;

  /* Constructs an encrypted integer initialized to encrypted zero.
   * Example: uint64e_t counter; */
  uint64e_t() : value_(detail::zero_uint64e()) {}
  /* Destroys the wrapper without additional cleanup because storage is POD-like.
   * Example: { uint64e_t tmp; } // destructor runs at scope exit */
  ~uint64e_t() = default;

  /* Copies an encrypted integer wrapper without decrypting the payload.
   * Example: uint64e_t b(a); */
  uint64e_t(const uint64e_t&) = default;
  /* Moves an encrypted integer wrapper.
   * Example: uint64e_t b(make_value()); */
  uint64e_t(uint64e_t&&) = default;
  /* Replaces this encrypted value with another encrypted wrapper.
   * Example: dst = src; */
  uint64e_t& operator=(const uint64e_t&) = default;
  /* Move-assigns another encrypted wrapper into this object.
   * Example: dst = make_value(); */
  uint64e_t& operator=(uint64e_t&&) = default;

  /* Wraps a pre-existing raw Mojo-V encrypted integer object.
   * Example: _uint64e_t raw = _enc(7u); uint64e_t v(raw); */
  uint64e_t(storage_type encrypted) : value_(encrypted) {}
  /* Encrypts a plain uint64_t into a uint64e_t wrapper.
   * Example: uint64e_t v(42u); */
  uint64e_t(value_type plain) : value_(_enc(plain)) {}
  /* Encrypts a plain int after promoting it to uint64_t.
   * Example: uint64e_t v(5); */
  uint64e_t(int plain) : value_(_enc(static_cast<value_type>(plain))) {}
  /* Converts an encrypted FP64 wrapper to encrypted uint64_t.
   * Example: uint64e_t bits(fp); */
  uint64e_t(const fp64e_t& plain);

  /* Replaces this wrapper with a raw encrypted integer payload.
   * Example: v = _enc(9u); */
  uint64e_t& operator=(storage_type encrypted) {
    value_ = encrypted;
    return *this;
  }

  /* Replaces this wrapper with the encrypted form of a plain uint64_t.
   * Example: v = 9u; */
  uint64e_t& operator=(value_type plain) {
    value_ = _enc(plain);
    return *this;
  }

  /* Returns the underlying encrypted payload as a const reference.
   * Example: const _uint64e_t &raw = v.encrypted(); */
  const storage_type& encrypted() const { return value_; }
  /* Returns the underlying encrypted payload as a mutable reference.
   * Example: v.encrypted() = _add(v.encrypted(), _enc(1u)); */
  storage_type& encrypted() { return value_; }

  /* Implicitly exposes the wrapped encrypted payload for intrinsic calls.
   * Example: _uint64e_t raw = static_cast<const _uint64e_t&>(v); */
  operator const storage_type&() const { return value_; }

  /* Returns the current encrypted value unchanged.
   * Example: uint64e_t same = +v; */
  uint64e_t operator+() const { return *this; }
  /* Returns the encrypted two's-complement negation of this value.
   * Example: uint64e_t neg = -v; */
  uint64e_t operator-() const { return uint64e_t(_neg(value_)); }
  /* Returns the encrypted bitwise complement of this value.
   * Example: uint64e_t flipped = ~mask; */
  uint64e_t operator~() const { return uint64e_t(_comp(value_)); }
  /* Returns the encrypted logical negation of this value.
   * Example: uint64e_t is_zero = !flag; */
  uint64e_t operator!() const { return uint64e_t(_lnot(value_)); }

  /* Adds another encrypted integer into this one.
   * Example: total += subtotal; */
  uint64e_t& operator+=(const uint64e_t& rhs) { value_ = _add(value_, rhs.value_); return *this; }
  /* Adds a plain integer into this encrypted one.
   * Example: total += 8u; */
  uint64e_t& operator+=(value_type rhs) { value_ = _addi(value_, rhs); return *this; }
  /* Subtracts another encrypted integer from this one.
   * Example: total -= subtotal; */
  uint64e_t& operator-=(const uint64e_t& rhs) { value_ = _sub(value_, rhs.value_); return *this; }
  /* Subtracts a plain integer from this encrypted one.
   * Example: total -= 8u; */
  uint64e_t& operator-=(value_type rhs) { value_ = _subi(value_, rhs); return *this; }
  /* Multiplies this encrypted integer by another encrypted integer.
   * Example: area *= width; */
  uint64e_t& operator*=(const uint64e_t& rhs) { value_ = _mulu(value_, rhs.value_); return *this; }
  /* Multiplies this encrypted integer by a plain integer.
   * Example: area *= 4u; */
  uint64e_t& operator*=(value_type rhs) { value_ = _mului(value_, rhs); return *this; }
  /* Divides this encrypted integer by another encrypted integer.
   * Example: quotient /= divisor; */
  uint64e_t& operator/=(const uint64e_t& rhs) { value_ = _divu(value_, rhs.value_); return *this; }
  /* Divides this encrypted integer by a plain integer.
   * Example: quotient /= 2u; */
  uint64e_t& operator/=(value_type rhs) { value_ = _divui(value_, rhs); return *this; }
  /* Computes encrypted remainder with another encrypted integer.
   * Example: residue %= modulus; */
  uint64e_t& operator%=(const uint64e_t& rhs) { value_ = _modu(value_, rhs.value_); return *this; }
  /* Computes encrypted remainder with a plain integer.
   * Example: residue %= 16u; */
  uint64e_t& operator%=(value_type rhs) { value_ = _modui(value_, rhs); return *this; }
  /* Applies encrypted bitwise AND with another encrypted integer.
   * Example: bits &= mask; */
  uint64e_t& operator&=(const uint64e_t& rhs) { value_ = _and(value_, rhs.value_); return *this; }
  /* Applies encrypted bitwise AND with a plain integer.
   * Example: bits &= 0xffu; */
  uint64e_t& operator&=(value_type rhs) { value_ = _andi(value_, rhs); return *this; }
  /* Applies encrypted bitwise OR with another encrypted integer.
   * Example: bits |= mask; */
  uint64e_t& operator|=(const uint64e_t& rhs) { value_ = _or(value_, rhs.value_); return *this; }
  /* Applies encrypted bitwise OR with a plain integer.
   * Example: bits |= 0x10u; */
  uint64e_t& operator|=(value_type rhs) { value_ = _ori(value_, rhs); return *this; }
  /* Applies encrypted bitwise XOR with another encrypted integer.
   * Example: bits ^= mask; */
  uint64e_t& operator^=(const uint64e_t& rhs) { value_ = _xor(value_, rhs.value_); return *this; }
  /* Applies encrypted bitwise XOR with a plain integer.
   * Example: bits ^= 0x10u; */
  uint64e_t& operator^=(value_type rhs) { value_ = _xori(value_, rhs); return *this; }
  /* Left-shifts this encrypted integer by another encrypted shift amount.
   * Example: bits <<= shift; */
  uint64e_t& operator<<=(const uint64e_t& rhs) { value_ = _sll(value_, rhs.value_); return *this; }
  /* Left-shifts this encrypted integer by a plain shift amount.
   * Example: bits <<= 3u; */
  uint64e_t& operator<<=(value_type rhs) { value_ = _slli(value_, rhs); return *this; }
  /* Right-shifts this encrypted integer by another encrypted shift amount.
   * Example: bits >>= shift; */
  uint64e_t& operator>>=(const uint64e_t& rhs) { value_ = _srl(value_, rhs.value_); return *this; }
  /* Right-shifts this encrypted integer by a plain shift amount.
   * Example: bits >>= 3u; */
  uint64e_t& operator>>=(value_type rhs) { value_ = _srli(value_, rhs); return *this; }

  /* Pre-increments this encrypted integer by one.
   * Example: ++counter; */
  uint64e_t& operator++() { value_ = _addi(value_, 1u); return *this; }
  /* Post-increments this encrypted integer by one and returns the prior value.
   * Example: counter++; */
  uint64e_t operator++(int) { uint64e_t tmp(*this); ++(*this); return tmp; }
  /* Pre-decrements this encrypted integer by one.
   * Example: --counter; */
  uint64e_t& operator--() { value_ = _subi(value_, 1u); return *this; }
  /* Post-decrements this encrypted integer by one and returns the prior value.
   * Example: counter--; */
  uint64e_t operator--(int) { uint64e_t tmp(*this); --(*this); return tmp; }

private:
  storage_type value_;
};

/* Wraps a Mojo-V encrypted FP64 value so C++ floating-point expressions map to intrinsics.
 * Example: fp64e_t score = 1.5; */
class fp64e_t {
public:
  using value_type = double;
  using storage_type = detail::fp_storage_t;

  /* Constructs an encrypted floating-point value initialized to 0.0.
   * Example: fp64e_t x; */
  fp64e_t() : value_(detail::zero_fp64e()) {}
  /* Destroys the wrapper without additional cleanup.
   * Example: { fp64e_t tmp; } */
  ~fp64e_t() = default;

  /* Copies an encrypted floating-point wrapper.
   * Example: fp64e_t b(a); */
  fp64e_t(const fp64e_t&) = default;
  /* Moves an encrypted floating-point wrapper.
   * Example: fp64e_t b(make_fp()); */
  fp64e_t(fp64e_t&&) = default;
  /* Copy-assigns another encrypted floating-point wrapper.
   * Example: dst = src; */
  fp64e_t& operator=(const fp64e_t&) = default;
  /* Move-assigns another encrypted floating-point wrapper.
   * Example: dst = make_fp(); */
  fp64e_t& operator=(fp64e_t&&) = default;

  /* Wraps a pre-existing raw encrypted FP64 payload.
   * Example: _fp64e_t raw = _fenc(2.5); fp64e_t v(raw); */
  fp64e_t(storage_type encrypted) : value_(encrypted) {}
  /* Encrypts a plain double into an fp64e_t wrapper.
   * Example: fp64e_t v(2.5); */
  fp64e_t(value_type plain) : value_(_fenc(plain)) {}
  /* Converts an encrypted uint64_t wrapper to encrypted FP64.
   * Example: fp64e_t fp(count); */
  fp64e_t(const uint64e_t& plain);

  /* Replaces this wrapper with a raw encrypted FP64 payload.
   * Example: v = _fenc(3.5); */
  fp64e_t& operator=(storage_type encrypted) {
    value_ = encrypted;
    return *this;
  }

  /* Replaces this wrapper with the encrypted form of a plain double.
   * Example: v = 3.5; */
  fp64e_t& operator=(value_type plain) {
    value_ = _fenc(plain);
    return *this;
  }

  /* Returns the underlying encrypted FP64 payload as a const reference.
   * Example: const _fp64e_t &raw = v.encrypted(); */
  const storage_type& encrypted() const { return value_; }
  /* Returns the underlying encrypted FP64 payload as a mutable reference.
   * Example: v.encrypted() = _fadd(v.encrypted(), _fenc(1.0)); */
  storage_type& encrypted() { return value_; }

  /* Implicitly exposes the wrapped encrypted FP64 payload for intrinsic calls.
   * Example: _fp64e_t raw = static_cast<const _fp64e_t&>(v); */
  operator const storage_type&() const { return value_; }

  /* Returns the current encrypted floating-point value unchanged.
   * Example: fp64e_t same = +v; */
  fp64e_t operator+() const { return *this; }
  /* Returns the encrypted arithmetic negation of this floating-point value.
   * Example: fp64e_t neg = -v; */
  fp64e_t operator-() const { return fp64e_t(_fneg(value_)); }

  /* Adds another encrypted FP64 into this one.
   * Example: total += delta; */
  fp64e_t& operator+=(const fp64e_t& rhs) { value_ = _fadd(value_, rhs.value_); return *this; }
  /* Adds a plain double into this encrypted one.
   * Example: total += 0.5; */
  fp64e_t& operator+=(value_type rhs) { value_ = _faddi(value_, rhs); return *this; }
  /* Subtracts another encrypted FP64 from this one.
   * Example: total -= delta; */
  fp64e_t& operator-=(const fp64e_t& rhs) { value_ = _fsub(value_, rhs.value_); return *this; }
  /* Subtracts a plain double from this encrypted one.
   * Example: total -= 0.5; */
  fp64e_t& operator-=(value_type rhs) { value_ = _fsubi(value_, rhs); return *this; }
  /* Multiplies this encrypted FP64 by another encrypted FP64.
   * Example: total *= scale; */
  fp64e_t& operator*=(const fp64e_t& rhs) { value_ = _fmul(value_, rhs.value_); return *this; }
  /* Multiplies this encrypted FP64 by a plain double.
   * Example: total *= 1.5; */
  fp64e_t& operator*=(value_type rhs) { value_ = _fmuli(value_, rhs); return *this; }
  /* Divides this encrypted FP64 by another encrypted FP64.
   * Example: total /= divisor; */
  fp64e_t& operator/=(const fp64e_t& rhs) { value_ = _fdiv(value_, rhs.value_); return *this; }
  /* Divides this encrypted FP64 by a plain double.
   * Example: total /= 2.0; */
  fp64e_t& operator/=(value_type rhs) { value_ = _fdivi(value_, rhs); return *this; }

private:
  storage_type value_;
};

/* Converts encrypted FP64 to encrypted uint64_t using Mojo-V conversion. */
inline uint64e_t::uint64e_t(const fp64e_t& plain) : value_(_fcvt_lu_d(plain.encrypted())) {}
/* Converts encrypted uint64_t to encrypted FP64 using Mojo-V conversion. */
inline fp64e_t::fp64e_t(const uint64e_t& plain) : value_(_fcvt_du(plain.encrypted())) {}

/* Returns the encrypted sum of two encrypted integers.
 * Example: uint64e_t total = a + b; */
inline uint64e_t operator+(uint64e_t lhs, const uint64e_t& rhs) { lhs += rhs; return lhs; }
/* Returns the encrypted sum of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t total = a + 4u; */
inline uint64e_t operator+(uint64e_t lhs, uint64_t rhs) { lhs += rhs; return lhs; }
/* Returns the encrypted sum of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t total = 4u + a; */
inline uint64e_t operator+(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) + rhs; }
/* Returns the encrypted difference of two encrypted integers.
 * Example: uint64e_t diff = a - b; */
inline uint64e_t operator-(uint64e_t lhs, const uint64e_t& rhs) { lhs -= rhs; return lhs; }
/* Returns the encrypted difference of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t diff = a - 4u; */
inline uint64e_t operator-(uint64e_t lhs, uint64_t rhs) { lhs -= rhs; return lhs; }
/* Returns the encrypted difference of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t diff = 9u - a; */
inline uint64e_t operator-(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) - rhs; }
/* Returns the encrypted product of two encrypted integers.
 * Example: uint64e_t prod = a * b; */
inline uint64e_t operator*(uint64e_t lhs, const uint64e_t& rhs) { lhs *= rhs; return lhs; }
/* Returns the encrypted product of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t prod = a * 8u; */
inline uint64e_t operator*(uint64e_t lhs, uint64_t rhs) { lhs *= rhs; return lhs; }
/* Returns the encrypted product of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t prod = 8u * a; */
inline uint64e_t operator*(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) * rhs; }
/* Returns the encrypted quotient of two encrypted integers.
 * Example: uint64e_t q = a / b; */
inline uint64e_t operator/(uint64e_t lhs, const uint64e_t& rhs) { lhs /= rhs; return lhs; }
/* Returns the encrypted quotient of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t q = a / 2u; */
inline uint64e_t operator/(uint64e_t lhs, uint64_t rhs) { lhs /= rhs; return lhs; }
/* Returns the encrypted quotient of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t q = 16u / a; */
inline uint64e_t operator/(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) / rhs; }
/* Returns the encrypted remainder of two encrypted integers.
 * Example: uint64e_t r = a % b; */
inline uint64e_t operator%(uint64e_t lhs, const uint64e_t& rhs) { lhs %= rhs; return lhs; }
/* Returns the encrypted remainder of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t r = a % 16u; */
inline uint64e_t operator%(uint64e_t lhs, uint64_t rhs) { lhs %= rhs; return lhs; }
/* Returns the encrypted remainder of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t r = 16u % a; */
inline uint64e_t operator%(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) % rhs; }
/* Returns the encrypted bitwise AND of two encrypted integers.
 * Example: uint64e_t both = a & b; */
inline uint64e_t operator&(uint64e_t lhs, const uint64e_t& rhs) { lhs &= rhs; return lhs; }
/* Returns the encrypted bitwise AND of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t masked = a & 0xffu; */
inline uint64e_t operator&(uint64e_t lhs, uint64_t rhs) { lhs &= rhs; return lhs; }
/* Returns the encrypted bitwise AND of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t masked = 0xffu & a; */
inline uint64e_t operator&(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) & rhs; }
/* Returns the encrypted bitwise OR of two encrypted integers.
 * Example: uint64e_t bits = a | b; */
inline uint64e_t operator|(uint64e_t lhs, const uint64e_t& rhs) { lhs |= rhs; return lhs; }
/* Returns the encrypted bitwise OR of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t bits = a | 0x10u; */
inline uint64e_t operator|(uint64e_t lhs, uint64_t rhs) { lhs |= rhs; return lhs; }
/* Returns the encrypted bitwise OR of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t bits = 0x10u | a; */
inline uint64e_t operator|(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) | rhs; }
/* Returns the encrypted bitwise XOR of two encrypted integers.
 * Example: uint64e_t bits = a ^ b; */
inline uint64e_t operator^(uint64e_t lhs, const uint64e_t& rhs) { lhs ^= rhs; return lhs; }
/* Returns the encrypted bitwise XOR of an encrypted integer and a plain uint64_t.
 * Example: uint64e_t bits = a ^ 0x10u; */
inline uint64e_t operator^(uint64e_t lhs, uint64_t rhs) { lhs ^= rhs; return lhs; }
/* Returns the encrypted bitwise XOR of a plain uint64_t and an encrypted integer.
 * Example: uint64e_t bits = 0x10u ^ a; */
inline uint64e_t operator^(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) ^ rhs; }
/* Returns the encrypted left shift of an integer by another encrypted amount.
 * Example: uint64e_t shifted = a << b; */
inline uint64e_t operator<<(uint64e_t lhs, const uint64e_t& rhs) { lhs <<= rhs; return lhs; }
/* Returns the encrypted left shift of an integer by a plain amount.
 * Example: uint64e_t shifted = a << 3u; */
inline uint64e_t operator<<(uint64e_t lhs, uint64_t rhs) { lhs <<= rhs; return lhs; }
/* Returns the encrypted left shift of a plain value by an encrypted amount.
 * Example: uint64e_t shifted = 1u << a; */
inline uint64e_t operator<<(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) << rhs; }
/* Returns the encrypted right shift of an integer by another encrypted amount.
 * Example: uint64e_t shifted = a >> b; */
inline uint64e_t operator>>(uint64e_t lhs, const uint64e_t& rhs) { lhs >>= rhs; return lhs; }
/* Returns the encrypted right shift of an integer by a plain amount.
 * Example: uint64e_t shifted = a >> 3u; */
inline uint64e_t operator>>(uint64e_t lhs, uint64_t rhs) { lhs >>= rhs; return lhs; }
/* Returns the encrypted right shift of a plain value by an encrypted amount.
 * Example: uint64e_t shifted = 8u >> a; */
inline uint64e_t operator>>(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(lhs) >> rhs; }
/* Computes encrypted logical AND without C++ short-circuiting.
 * Example: uint64e_t both = a && b; */
inline uint64e_t operator&&(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_land(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted logical AND between an encrypted integer and a plain integer.
 * Example: uint64e_t both = a && 1u; */
inline uint64e_t operator&&(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_landi(lhs.encrypted(), rhs)); }
/* Computes encrypted logical AND between a plain integer and an encrypted integer.
 * Example: uint64e_t both = 1u && a; */
inline uint64e_t operator&&(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(_landi(rhs.encrypted(), lhs)); }
/* Computes encrypted logical OR without C++ short-circuiting.
 * Example: uint64e_t either = a || b; */
inline uint64e_t operator||(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_lor(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted logical OR between an encrypted integer and a plain integer.
 * Example: uint64e_t either = a || 0u; */
inline uint64e_t operator||(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_lori(lhs.encrypted(), rhs)); }
/* Computes encrypted logical OR between a plain integer and an encrypted integer.
 * Example: uint64e_t either = 0u || a; */
inline uint64e_t operator||(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(_lori(rhs.encrypted(), lhs)); }

/* Computes encrypted equality for two encrypted integers.
 * Example: uint64e_t eq = (a == b); */
inline uint64e_t operator==(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_seq(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted equality for an encrypted integer and a plain integer.
 * Example: uint64e_t eq = (a == 7u); */
inline uint64e_t operator==(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_seqi(lhs.encrypted(), rhs)); }
/* Computes encrypted equality for a plain integer and an encrypted integer.
 * Example: uint64e_t eq = (7u == a); */
inline uint64e_t operator==(uint64_t lhs, const uint64e_t& rhs) { return rhs == lhs; }
/* Computes encrypted inequality for two encrypted integers.
 * Example: uint64e_t ne = (a != b); */
inline uint64e_t operator!=(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_sne(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted inequality for an encrypted integer and a plain integer.
 * Example: uint64e_t ne = (a != 7u); */
inline uint64e_t operator!=(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_snei(lhs.encrypted(), rhs)); }
/* Computes encrypted inequality for a plain integer and an encrypted integer.
 * Example: uint64e_t ne = (7u != a); */
inline uint64e_t operator!=(uint64_t lhs, const uint64e_t& rhs) { return rhs != lhs; }
/* Computes encrypted unsigned less-than for two encrypted integers.
 * Example: uint64e_t lt = (a < b); */
inline uint64e_t operator<(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_sltu(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted unsigned less-than for an encrypted integer and a plain integer.
 * Example: uint64e_t lt = (a < 7u); */
inline uint64e_t operator<(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_sltui(lhs.encrypted(), rhs)); }
/* Computes encrypted unsigned less-than for a plain integer and an encrypted integer.
 * Example: uint64e_t lt = (7u < a); */
inline uint64e_t operator<(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(_sgtui(rhs.encrypted(), lhs)); }
/* Computes encrypted unsigned less-than-or-equal for two encrypted integers.
 * Example: uint64e_t le = (a <= b); */
inline uint64e_t operator<=(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_sleu(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted unsigned less-than-or-equal for an encrypted integer and a plain integer.
 * Example: uint64e_t le = (a <= 7u); */
inline uint64e_t operator<=(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_sleui(lhs.encrypted(), rhs)); }
/* Computes encrypted unsigned less-than-or-equal for a plain integer and an encrypted integer.
 * Example: uint64e_t le = (7u <= a); */
inline uint64e_t operator<=(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(_sgeui(rhs.encrypted(), lhs)); }
/* Computes encrypted unsigned greater-than for two encrypted integers.
 * Example: uint64e_t gt = (a > b); */
inline uint64e_t operator>(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_sgtu(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted unsigned greater-than for an encrypted integer and a plain integer.
 * Example: uint64e_t gt = (a > 7u); */
inline uint64e_t operator>(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_sgtui(lhs.encrypted(), rhs)); }
/* Computes encrypted unsigned greater-than for a plain integer and an encrypted integer.
 * Example: uint64e_t gt = (7u > a); */
inline uint64e_t operator>(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(_sltui(rhs.encrypted(), lhs)); }
/* Computes encrypted unsigned greater-than-or-equal for two encrypted integers.
 * Example: uint64e_t ge = (a >= b); */
inline uint64e_t operator>=(const uint64e_t& lhs, const uint64e_t& rhs) { return uint64e_t(_sgeu(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted unsigned greater-than-or-equal for an encrypted integer and a plain integer.
 * Example: uint64e_t ge = (a >= 7u); */
inline uint64e_t operator>=(const uint64e_t& lhs, uint64_t rhs) { return uint64e_t(_sgeui(lhs.encrypted(), rhs)); }
/* Computes encrypted unsigned greater-than-or-equal for a plain integer and an encrypted integer.
 * Example: uint64e_t ge = (7u >= a); */
inline uint64e_t operator>=(uint64_t lhs, const uint64e_t& rhs) { return uint64e_t(_sleui(rhs.encrypted(), lhs)); }

/* Returns the encrypted sum of two encrypted FP64 values.
 * Example: fp64e_t total = a + b; */
inline fp64e_t operator+(fp64e_t lhs, const fp64e_t& rhs) { lhs += rhs; return lhs; }
/* Returns the encrypted sum of an encrypted FP64 and a plain double.
 * Example: fp64e_t total = a + 0.5; */
inline fp64e_t operator+(fp64e_t lhs, double rhs) { lhs += rhs; return lhs; }
/* Returns the encrypted sum of a plain double and an encrypted FP64.
 * Example: fp64e_t total = 0.5 + a; */
inline fp64e_t operator+(double lhs, const fp64e_t& rhs) { return fp64e_t(lhs) + rhs; }
/* Returns the encrypted difference of two encrypted FP64 values.
 * Example: fp64e_t diff = a - b; */
inline fp64e_t operator-(fp64e_t lhs, const fp64e_t& rhs) { lhs -= rhs; return lhs; }
/* Returns the encrypted difference of an encrypted FP64 and a plain double.
 * Example: fp64e_t diff = a - 0.5; */
inline fp64e_t operator-(fp64e_t lhs, double rhs) { lhs -= rhs; return lhs; }
/* Returns the encrypted difference of a plain double and an encrypted FP64.
 * Example: fp64e_t diff = 3.0 - a; */
inline fp64e_t operator-(double lhs, const fp64e_t& rhs) { return fp64e_t(lhs) - rhs; }
/* Returns the encrypted product of two encrypted FP64 values.
 * Example: fp64e_t prod = a * b; */
inline fp64e_t operator*(fp64e_t lhs, const fp64e_t& rhs) { lhs *= rhs; return lhs; }
/* Returns the encrypted product of an encrypted FP64 and a plain double.
 * Example: fp64e_t prod = a * 2.0; */
inline fp64e_t operator*(fp64e_t lhs, double rhs) { lhs *= rhs; return lhs; }
/* Returns the encrypted product of a plain double and an encrypted FP64.
 * Example: fp64e_t prod = 2.0 * a; */
inline fp64e_t operator*(double lhs, const fp64e_t& rhs) { return fp64e_t(lhs) * rhs; }
/* Returns the encrypted quotient of two encrypted FP64 values.
 * Example: fp64e_t q = a / b; */
inline fp64e_t operator/(fp64e_t lhs, const fp64e_t& rhs) { lhs /= rhs; return lhs; }
/* Returns the encrypted quotient of an encrypted FP64 and a plain double.
 * Example: fp64e_t q = a / 2.0; */
inline fp64e_t operator/(fp64e_t lhs, double rhs) { lhs /= rhs; return lhs; }
/* Returns the encrypted quotient of a plain double and an encrypted FP64.
 * Example: fp64e_t q = 8.0 / a; */
inline fp64e_t operator/(double lhs, const fp64e_t& rhs) { return fp64e_t(lhs) / rhs; }

/* Computes encrypted floating-point equality for two encrypted FP64 values.
 * Example: uint64e_t eq = (a == b); */
inline uint64e_t operator==(const fp64e_t& lhs, const fp64e_t& rhs) { return uint64e_t(_fseq(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted floating-point equality for an encrypted FP64 and a plain double.
 * Example: uint64e_t eq = (a == 1.0); */
inline uint64e_t operator==(const fp64e_t& lhs, double rhs) { return uint64e_t(_fseqi(lhs.encrypted(), rhs)); }
/* Computes encrypted floating-point equality for a plain double and an encrypted FP64.
 * Example: uint64e_t eq = (1.0 == a); */
inline uint64e_t operator==(double lhs, const fp64e_t& rhs) { return rhs == lhs; }
/* Computes encrypted floating-point inequality for two encrypted FP64 values.
 * Example: uint64e_t ne = (a != b); */
inline uint64e_t operator!=(const fp64e_t& lhs, const fp64e_t& rhs) { return uint64e_t(_fsne(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted floating-point inequality for an encrypted FP64 and a plain double.
 * Example: uint64e_t ne = (a != 1.0); */
inline uint64e_t operator!=(const fp64e_t& lhs, double rhs) { return uint64e_t(_fsnei(lhs.encrypted(), rhs)); }
/* Computes encrypted floating-point inequality for a plain double and an encrypted FP64.
 * Example: uint64e_t ne = (1.0 != a); */
inline uint64e_t operator!=(double lhs, const fp64e_t& rhs) { return rhs != lhs; }
/* Computes encrypted floating-point less-than for two encrypted FP64 values.
 * Example: uint64e_t lt = (a < b); */
inline uint64e_t operator<(const fp64e_t& lhs, const fp64e_t& rhs) { return uint64e_t(_fslt(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted floating-point less-than for an encrypted FP64 and a plain double.
 * Example: uint64e_t lt = (a < 1.0); */
inline uint64e_t operator<(const fp64e_t& lhs, double rhs) { return uint64e_t(_fslti(lhs.encrypted(), rhs)); }
/* Computes encrypted floating-point less-than for a plain double and an encrypted FP64.
 * Example: uint64e_t lt = (1.0 < a); */
inline uint64e_t operator<(double lhs, const fp64e_t& rhs) { return uint64e_t(_fsgti(rhs.encrypted(), lhs)); }
/* Computes encrypted floating-point less-than-or-equal for two encrypted FP64 values.
 * Example: uint64e_t le = (a <= b); */
inline uint64e_t operator<=(const fp64e_t& lhs, const fp64e_t& rhs) { return uint64e_t(_fsle(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted floating-point less-than-or-equal for an encrypted FP64 and a plain double.
 * Example: uint64e_t le = (a <= 1.0); */
inline uint64e_t operator<=(const fp64e_t& lhs, double rhs) { return uint64e_t(_fslei(lhs.encrypted(), rhs)); }
/* Computes encrypted floating-point less-than-or-equal for a plain double and an encrypted FP64.
 * Example: uint64e_t le = (1.0 <= a); */
inline uint64e_t operator<=(double lhs, const fp64e_t& rhs) { return uint64e_t(_fsgei(rhs.encrypted(), lhs)); }
/* Computes encrypted floating-point greater-than for two encrypted FP64 values.
 * Example: uint64e_t gt = (a > b); */
inline uint64e_t operator>(const fp64e_t& lhs, const fp64e_t& rhs) { return uint64e_t(_fsgt(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted floating-point greater-than for an encrypted FP64 and a plain double.
 * Example: uint64e_t gt = (a > 1.0); */
inline uint64e_t operator>(const fp64e_t& lhs, double rhs) { return uint64e_t(_fsgti(lhs.encrypted(), rhs)); }
/* Computes encrypted floating-point greater-than for a plain double and an encrypted FP64.
 * Example: uint64e_t gt = (1.0 > a); */
inline uint64e_t operator>(double lhs, const fp64e_t& rhs) { return uint64e_t(_fslti(rhs.encrypted(), lhs)); }
/* Computes encrypted floating-point greater-than-or-equal for two encrypted FP64 values.
 * Example: uint64e_t ge = (a >= b); */
inline uint64e_t operator>=(const fp64e_t& lhs, const fp64e_t& rhs) { return uint64e_t(_fsge(lhs.encrypted(), rhs.encrypted())); }
/* Computes encrypted floating-point greater-than-or-equal for an encrypted FP64 and a plain double.
 * Example: uint64e_t ge = (a >= 1.0); */
inline uint64e_t operator>=(const fp64e_t& lhs, double rhs) { return uint64e_t(_fsgei(lhs.encrypted(), rhs)); }
/* Computes encrypted floating-point greater-than-or-equal for a plain double and an encrypted FP64.
 * Example: uint64e_t ge = (1.0 >= a); */
inline uint64e_t operator>=(double lhs, const fp64e_t& rhs) { return uint64e_t(_fslei(rhs.encrypted(), lhs)); }

/* Selects one encrypted integer or another based on an encrypted predicate.
 * Example: uint64e_t chosen = cmov(pred, on_true, on_false); */
inline uint64e_t cmov(const uint64e_t& predicate, const uint64e_t& if_true, const uint64e_t& if_false) {
  return uint64e_t(_cmov(predicate.encrypted(), if_true.encrypted(), if_false.encrypted()));
}
/* Selects one encrypted integer or another based on a plain predicate. */
inline uint64e_t cmov(uint64_t predicate, const uint64e_t& if_true, const uint64e_t& if_false) {
  return cmov(uint64e_t(predicate), if_true, if_false);
}
inline uint64e_t cmov(const uint64e_t& predicate, uint64_t if_true, const uint64e_t& if_false) {
  return cmov(predicate, uint64e_t(if_true), if_false);
}
inline uint64e_t cmov(const uint64e_t& predicate, const uint64e_t& if_true, uint64_t if_false) {
  return cmov(predicate, if_true, uint64e_t(if_false));
}
inline uint64e_t cmov(uint64_t predicate, uint64_t if_true, const uint64e_t& if_false) {
  return cmov(uint64e_t(predicate), uint64e_t(if_true), if_false);
}
inline uint64e_t cmov(uint64_t predicate, const uint64e_t& if_true, uint64_t if_false) {
  return cmov(uint64e_t(predicate), if_true, uint64e_t(if_false));
}
inline uint64e_t cmov(const uint64e_t& predicate, uint64_t if_true, uint64_t if_false) {
  return cmov(predicate, uint64e_t(if_true), uint64e_t(if_false));
}
inline uint64e_t cmov(uint64_t predicate, uint64_t if_true, uint64_t if_false) {
  return cmov(uint64e_t(predicate), uint64e_t(if_true), uint64e_t(if_false));
}

/* Selects one encrypted FP64 or another based on encrypted/plain predicate. */
inline fp64e_t cmov(const uint64e_t& predicate, const fp64e_t& if_true, const fp64e_t& if_false) {
  return fp64e_t(_fcmov(predicate.encrypted(), if_true.encrypted(), if_false.encrypted()));
}
inline fp64e_t cmov(uint64_t predicate, const fp64e_t& if_true, const fp64e_t& if_false) {
  return cmov(uint64e_t(predicate), if_true, if_false);
}
inline fp64e_t cmov(const uint64e_t& predicate, double if_true, const fp64e_t& if_false) {
  return cmov(predicate, fp64e_t(if_true), if_false);
}
inline fp64e_t cmov(const uint64e_t& predicate, const fp64e_t& if_true, double if_false) {
  return cmov(predicate, if_true, fp64e_t(if_false));
}
inline fp64e_t cmov(uint64_t predicate, double if_true, const fp64e_t& if_false) {
  return cmov(uint64e_t(predicate), fp64e_t(if_true), if_false);
}
inline fp64e_t cmov(uint64_t predicate, const fp64e_t& if_true, double if_false) {
  return cmov(uint64e_t(predicate), if_true, fp64e_t(if_false));
}
inline fp64e_t cmov(const uint64e_t& predicate, double if_true, double if_false) {
  return cmov(predicate, fp64e_t(if_true), fp64e_t(if_false));
}
inline fp64e_t cmov(uint64_t predicate, double if_true, double if_false) {
  return cmov(uint64e_t(predicate), fp64e_t(if_true), fp64e_t(if_false));
}

/* Returns the encrypted absolute value of an encrypted FP64.
 * Example: fp64e_t mag = abs(delta); */
inline fp64e_t fabs(const fp64e_t& value) {
  return fp64e_t(_fabs(value.encrypted()));
}

}  // namespace exo

using exo::cmov;
using exo::fp64e_t;
using exo::uint64e_t;

#endif  // __cplusplus

#endif  // EXO_H
