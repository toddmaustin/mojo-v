#ifndef MOJOV_EXO_H
#define MOJOV_EXO_H

#if !defined(__cplusplus)
#include "mojov-intrinsics.h"
#else

#include <cstdint>
#include <cstddef>
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

namespace detail {

using uint_storage_t = _uint64e_t;
using fp_storage_t = _fp64e_t;

struct datagrant_plaintext_t {
  uint64_t dfhash;
  uint64_t salt;
  uint64_t sig;
  uint64_t metadata;
};

static_assert(std::is_trivially_copyable<uint_storage_t>::value,
              "_uint64e_t must be trivially copyable");
static_assert(std::is_trivially_copyable<fp_storage_t>::value,
              "_fp64e_t must be trivially copyable");

inline uint_storage_t zero_uint64e() { return _enc(0u); }
inline fp_storage_t zero_fp64e() { return _fenc(0.0); }

inline simon_state_t& debug_simon_state() {
  static simon_state_t state;
  return state;
}

inline uint64_t& debug_contract_sig() {
  static uint64_t sig = 0;
  return sig;
}

inline bool& debug_context_ready() {
  static bool ready = false;
  return ready;
}

inline void debug_context_or_die() {
  if (!debug_context_ready()) {
    libmin_printf("ERROR: exo::debug_context() must be initialized before calling decrypt().\n");
    libmin_fail(-1);
  }
}

inline uint64_t decrypt_storage(const mojov_mem_fast_u64_t& value) {
  return mojov_decrypt_fast_u64(&debug_simon_state(), value, debug_contract_sig());
}

inline uint64_t decrypt_storage(const mojov_mem_strong_u64_t& value) {
  return mojov_decrypt_strong_u64(&debug_simon_state(), value, debug_contract_sig());
}

inline uint64_t decrypt_storage(const mojov_mem_proofcarrying_u64_t& value) {
  return mojov_decrypt_proofcarrying_u64(&debug_simon_state(), value, debug_contract_sig());
}

inline double decrypt_storage(const mojov_mem_fast_fp64_t& value) {
  return mojov_decrypt_fast_fp64(&debug_simon_state(), value, debug_contract_sig());
}

inline double decrypt_storage(const mojov_mem_strong_fp64_t& value) {
  return mojov_decrypt_strong_fp64(&debug_simon_state(), value, debug_contract_sig());
}

inline double decrypt_storage(const mojov_mem_proofcarrying_fp64_t& value) {
  return mojov_decrypt_proofcarrying_fp64(&debug_simon_state(), value, debug_contract_sig());
}

inline datagrant_plaintext_t decrypt_storage(const mojov_mem_datagrant_t& value) {
  mojov_mem_datagrant_t plaintext;
  simon_128_128_decrypt(&debug_simon_state(), value.ct.ct_lo, &plaintext.ct.ct_lo);
  simon_128_128_decrypt(&debug_simon_state(), value.ct.ct_hi, &plaintext.ct.ct_hi);
  plaintext.ct.ct_hi ^= value.ct.ct_lo;
  return {plaintext.pt.dfhash, plaintext.pt.salt, plaintext.pt.sig, plaintext.pt.metadata};
}

}  // namespace detail

/* Initializes decrypt() support used by encrypted wrappers during debug.
 * Returns 0 when key expansion succeeds and 1 otherwise.
 * Example: int ok = debug_context(SIMON128_KEY, contract_sig); */
inline int debug_context(uint128_t simon128_key, uint64_t contract_sig) {
  const bool expanded = simon_128_128_keyexpand(&detail::debug_simon_state(), simon128_key, 68);
  detail::debug_contract_sig() = contract_sig;
  detail::debug_context_ready() = expanded;
  return expanded ? 0 : 1;
}

class datagrant_t {
public:
  using storage_type = mojov_mem_datagrant_t;
  using plaintext_type = detail::datagrant_plaintext_t;

  datagrant_t() = default;
  datagrant_t(const storage_type& encrypted) : value_(encrypted) {}

  const storage_type& encrypted() const { return value_; }
  storage_type& encrypted() { return value_; }
  operator const storage_type&() const { return value_; }

  datagrant_t& operator=(storage_type encrypted) { value_ = encrypted; return *this; }

  plaintext_type decrypt() const {
    detail::debug_context_or_die();
    return detail::decrypt_storage(value_);
  }

  bool is_valid() const {
    detail::debug_context_or_die();
    const plaintext_type plaintext = detail::decrypt_storage(value_);
    return plaintext.sig == detail::debug_contract_sig() + 1u;
  }

private:
  storage_type value_{};
};

/* Unified encrypted scalar templates.
 *
 * uint64e_t/int64e_t/fp64e_t are now aliases of these generic templates.
 */

template <std::size_t Bits, bool IsSigned>
class inte_t;

template <std::size_t Bits>
class fpe_t;

template <std::size_t Bits, bool IsSigned>
class inte_t {
  static_assert(Bits > 0 && Bits <= 64, "Bits must be in [1, 64]");
public:
  using storage_type = detail::uint_storage_t;
  using value_type = typename std::conditional<IsSigned, int64_t, uint64_t>::type;

  inte_t() : value_(detail::zero_uint64e()) {}
  inte_t(storage_type encrypted) : value_(normalize(encrypted)) {}
  inte_t(value_type plain) : value_(normalize(_enc(static_cast<uint64_t>(plain)))) {}
  inte_t(int plain) : value_(normalize(_enc(static_cast<uint64_t>(plain)))) {}

  template <std::size_t OBits, bool OSigned>
  inte_t(const inte_t<OBits, OSigned>& other) : value_(normalize(other.encrypted())) {}

  template <std::size_t FBits>
  inte_t(const fpe_t<FBits>& plain)
      : value_(normalize(IsSigned ? _fcvt_l_d(plain.encrypted()) : _fcvt_lu_d(plain.encrypted()))) {}

  const storage_type& encrypted() const { return value_; }
  storage_type& encrypted() { return value_; }
  operator const storage_type&() const { return value_; }

  value_type decrypt() const {
    detail::debug_context_or_die();
    const uint64_t raw = detail::decrypt_storage(value_);
    if constexpr (IsSigned) return static_cast<value_type>(sign_extend(raw));
    return static_cast<value_type>(truncate(raw));
  }

  // Integer: assignments (plain and encrypted payloads).
  inte_t& operator=(storage_type encrypted) { value_ = normalize(encrypted); return *this; }
  inte_t& operator=(value_type plain) { value_ = normalize(_enc(static_cast<uint64_t>(plain))); return *this; }

  // Integer: arithmetic / bitwise unary operators.
  inte_t operator+() const { return *this; }
  inte_t operator-() const { return inte_t(normalize(_neg(value_))); }
  inte_t operator~() const { return inte_t(normalize(_comp(value_))); }
  inte_t operator!() const { return inte_t(_lnot(value_)); }

  // Integer: assignments + arithmetic / bitwise compound operators.
  inte_t& operator+=(const inte_t& rhs) { value_ = normalize(_add(value_, rhs.value_)); return *this; }
  inte_t& operator+=(value_type rhs) { value_ = normalize(_addi(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator-=(const inte_t& rhs) { value_ = normalize(_sub(value_, rhs.value_)); return *this; }
  inte_t& operator-=(value_type rhs) { value_ = normalize(_subi(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator*=(const inte_t& rhs) { value_ = normalize(IsSigned ? _mul(value_, rhs.value_) : _mulu(value_, rhs.value_)); return *this; }
  inte_t& operator*=(value_type rhs) { value_ = normalize(IsSigned ? _muli(value_, static_cast<int64_t>(rhs)) : _mului(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator/=(const inte_t& rhs) { value_ = normalize(IsSigned ? _div(value_, rhs.value_) : _divu(value_, rhs.value_)); return *this; }
  inte_t& operator/=(value_type rhs) { value_ = normalize(IsSigned ? _divi(value_, static_cast<int64_t>(rhs)) : _divui(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator%=(const inte_t& rhs) { value_ = normalize(IsSigned ? _mod(value_, rhs.value_) : _modu(value_, rhs.value_)); return *this; }
  inte_t& operator%=(value_type rhs) { value_ = normalize(IsSigned ? _modi(value_, static_cast<int64_t>(rhs)) : _modui(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator&=(const inte_t& rhs) { value_ = normalize(_and(value_, rhs.value_)); return *this; }
  inte_t& operator&=(value_type rhs) { value_ = normalize(_andi(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator|=(const inte_t& rhs) { value_ = normalize(_or(value_, rhs.value_)); return *this; }
  inte_t& operator|=(value_type rhs) { value_ = normalize(_ori(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator^=(const inte_t& rhs) { value_ = normalize(_xor(value_, rhs.value_)); return *this; }
  inte_t& operator^=(value_type rhs) { value_ = normalize(_xori(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator<<=(const inte_t& rhs) { value_ = normalize(_sll(value_, rhs.value_)); return *this; }
  inte_t& operator<<=(value_type rhs) { value_ = normalize(_slli(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator>>=(const inte_t& rhs) { value_ = normalize(IsSigned ? _sra(value_, rhs.value_) : _srl(value_, rhs.value_)); return *this; }
  inte_t& operator>>=(value_type rhs) { value_ = normalize(IsSigned ? _srai(value_, static_cast<uint64_t>(rhs)) : _srli(value_, static_cast<uint64_t>(rhs))); return *this; }
  inte_t& operator++() { return *this += 1; }
  inte_t operator++(int) { inte_t tmp(*this); ++(*this); return tmp; }
  inte_t& operator--() { return *this -= 1; }
  inte_t operator--(int) { inte_t tmp(*this); --(*this); return tmp; }

private:
  static constexpr uint64_t kMask = (Bits == 64) ? ~uint64_t(0) : ((uint64_t(1) << Bits) - 1u);
  static constexpr uint64_t truncate(uint64_t x) { return x & kMask; }
  static constexpr int64_t sign_extend(uint64_t x) {
    if constexpr (Bits == 64) return static_cast<int64_t>(x);
    const uint64_t bit = uint64_t(1) << (Bits - 1);
    const uint64_t masked = x & kMask;
    return static_cast<int64_t>((masked ^ bit) - bit);
  }
  static storage_type normalize(storage_type raw) {
    if constexpr (Bits == 64) return raw;
    if constexpr (IsSigned) { constexpr uint64_t sh = 64u - Bits; return _srai(_slli(raw, sh), sh); }
    return _andi(raw, kMask);
  }

  storage_type value_;
};

template <std::size_t Bits>
class fpe_t {
  static_assert(Bits == 32 || Bits == 64, "Only 32-bit and 64-bit encrypted FP are supported");
public:
  using storage_type = detail::fp_storage_t;
  using value_type = typename std::conditional<Bits == 32, float, double>::type;

  fpe_t() : value_(detail::zero_fp64e()) {}
  fpe_t(storage_type encrypted) : value_(encrypted) {}
  fpe_t(value_type plain) : value_(_fenc(static_cast<double>(plain))) {}

  template <std::size_t IBits, bool ISigned>
  fpe_t(const inte_t<IBits, ISigned>& plain)
      : value_(IBits == 64 ? (ISigned ? _fcvt_d_l(plain.encrypted()) : _fcvt_du(plain.encrypted()))
                           : (ISigned ? _fcvt_d_l(inte_t<64, true>(plain).encrypted())
                                      : _fcvt_du(inte_t<64, false>(plain).encrypted()))) {}

  const storage_type& encrypted() const { return value_; }
  storage_type& encrypted() { return value_; }
  operator const storage_type&() const { return value_; }

  value_type decrypt() const {
    detail::debug_context_or_die();
    return static_cast<value_type>(detail::decrypt_storage(value_));
  }

  // Floating point: arithmetic unary operators.
  fpe_t operator+() const { return *this; }
  fpe_t operator-() const { return fpe_t(_fneg(value_)); }

  // Floating point: assignments + arithmetic compound operators.
  fpe_t& operator+=(const fpe_t& rhs) { value_ = _fadd(value_, rhs.value_); return *this; }
  fpe_t& operator+=(value_type rhs) { value_ = _faddi(value_, static_cast<double>(rhs)); return *this; }
  fpe_t& operator-=(const fpe_t& rhs) { value_ = _fsub(value_, rhs.value_); return *this; }
  fpe_t& operator-=(value_type rhs) { value_ = _fsubi(value_, static_cast<double>(rhs)); return *this; }
  fpe_t& operator*=(const fpe_t& rhs) { value_ = _fmul(value_, rhs.value_); return *this; }
  fpe_t& operator*=(value_type rhs) { value_ = _fmuli(value_, static_cast<double>(rhs)); return *this; }
  fpe_t& operator/=(const fpe_t& rhs) { value_ = _fdiv(value_, rhs.value_); return *this; }
  fpe_t& operator/=(value_type rhs) { value_ = _fdivi(value_, static_cast<double>(rhs)); return *this; }

private:
  storage_type value_;
};

// ============================================================================
// Integer groups
// ============================================================================

// Integer / arithmetic operators.
template <std::size_t B, bool S> inline inte_t<B,S> operator+(inte_t<B,S> l, const inte_t<B,S>& r){ l+=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator-(inte_t<B,S> l, const inte_t<B,S>& r){ l-=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator*(inte_t<B,S> l, const inte_t<B,S>& r){ l*=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator/(inte_t<B,S> l, const inte_t<B,S>& r){ l/=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator%(inte_t<B,S> l, const inte_t<B,S>& r){ l%=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator&(inte_t<B,S> l, const inte_t<B,S>& r){ l&=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator|(inte_t<B,S> l, const inte_t<B,S>& r){ l|=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator^(inte_t<B,S> l, const inte_t<B,S>& r){ l^=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator<<(inte_t<B,S> l, const inte_t<B,S>& r){ l<<=r; return l; }
template <std::size_t B, bool S> inline inte_t<B,S> operator>>(inte_t<B,S> l, const inte_t<B,S>& r){ l>>=r; return l; }

template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator+(inte_t<B,S> l, T r){ l += static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator-(inte_t<B,S> l, T r){ l -= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator*(inte_t<B,S> l, T r){ l *= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator/(inte_t<B,S> l, T r){ l /= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator%(inte_t<B,S> l, T r){ l %= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator&(inte_t<B,S> l, T r){ l &= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator|(inte_t<B,S> l, T r){ l |= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator^(inte_t<B,S> l, T r){ l ^= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator<<(inte_t<B,S> l, T r){ l <<= static_cast<typename inte_t<B,S>::value_type>(r); return l; }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator>>(inte_t<B,S> l, T r){ l >>= static_cast<typename inte_t<B,S>::value_type>(r); return l; }

template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator+(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) + r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator-(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) - r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator*(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) * r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator/(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) / r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator%(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) % r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator&(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) & r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator|(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) | r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator^(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) ^ r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator<<(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) << r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator>>(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) >> r; }

// Integer / relational operators (including logical boolean-as-encrypted-int results).
template <std::size_t B, bool S> inline inte_t<B,S> operator&&(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(_land(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S> inline inte_t<B,S> operator||(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(_lor(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator&&(const inte_t<B,S>& l, T r){ return inte_t<B,S>(_landi(l.encrypted(), static_cast<uint64_t>(r))); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator||(const inte_t<B,S>& l, T r){ return inte_t<B,S>(_lori(l.encrypted(), static_cast<uint64_t>(r))); }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator&&(T l, const inte_t<B,S>& r){ return inte_t<B,S>(_landi(r.encrypted(), static_cast<uint64_t>(l))); }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator||(T l, const inte_t<B,S>& r){ return inte_t<B,S>(_lori(r.encrypted(), static_cast<uint64_t>(l))); }
template <std::size_t B, bool S> inline inte_t<B,S> operator==(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(_seq(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S> inline inte_t<B,S> operator!=(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(_sne(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S> inline inte_t<B,S> operator<(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(S ? _slt(l.encrypted(), r.encrypted()) : _sltu(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S> inline inte_t<B,S> operator<=(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(S ? _sle(l.encrypted(), r.encrypted()) : _sleu(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S> inline inte_t<B,S> operator>(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(S ? _sgt(l.encrypted(), r.encrypted()) : _sgtu(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S> inline inte_t<B,S> operator>=(const inte_t<B,S>& l, const inte_t<B,S>& r){ return inte_t<B,S>(S ? _sge(l.encrypted(), r.encrypted()) : _sgeu(l.encrypted(), r.encrypted())); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator==(const inte_t<B,S>& l, T r){ return l == inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(r)); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator!=(const inte_t<B,S>& l, T r){ return l != inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(r)); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator<(const inte_t<B,S>& l, T r){ return l < inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(r)); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator<=(const inte_t<B,S>& l, T r){ return l <= inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(r)); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator>(const inte_t<B,S>& l, T r){ return l > inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(r)); }
template <std::size_t B, bool S, typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator>=(const inte_t<B,S>& l, T r){ return l >= inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(r)); }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator==(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) == r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator!=(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) != r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator<(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) < r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator<=(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) <= r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator>(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) > r; }
template <typename T, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<T>::value>::type>
inline inte_t<B,S> operator>=(T l, const inte_t<B,S>& r){ return inte_t<B,S>(static_cast<typename inte_t<B,S>::value_type>(l)) >= r; }

// ============================================================================
// Floating-point groups
// ============================================================================

// Floating point / arithmetic operators.
template <std::size_t B> inline fpe_t<B> operator+(fpe_t<B> l, const fpe_t<B>& r){ l += r; return l; }
template <std::size_t B> inline fpe_t<B> operator-(fpe_t<B> l, const fpe_t<B>& r){ l -= r; return l; }
template <std::size_t B> inline fpe_t<B> operator*(fpe_t<B> l, const fpe_t<B>& r){ l *= r; return l; }
template <std::size_t B> inline fpe_t<B> operator/(fpe_t<B> l, const fpe_t<B>& r){ l /= r; return l; }
template <std::size_t B> inline fpe_t<B> operator+(fpe_t<B> l, typename fpe_t<B>::value_type r){ l += r; return l; }
template <std::size_t B> inline fpe_t<B> operator-(fpe_t<B> l, typename fpe_t<B>::value_type r){ l -= r; return l; }
template <std::size_t B> inline fpe_t<B> operator*(fpe_t<B> l, typename fpe_t<B>::value_type r){ l *= r; return l; }
template <std::size_t B> inline fpe_t<B> operator/(fpe_t<B> l, typename fpe_t<B>::value_type r){ l /= r; return l; }
template <std::size_t B> inline fpe_t<B> operator+(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return fpe_t<B>(l) + r; }
template <std::size_t B> inline fpe_t<B> operator-(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return fpe_t<B>(l) - r; }
template <std::size_t B> inline fpe_t<B> operator*(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return fpe_t<B>(l) * r; }
template <std::size_t B> inline fpe_t<B> operator/(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return fpe_t<B>(l) / r; }
// Floating point / relational operators.
template <std::size_t B> inline auto operator==(const fpe_t<B>& l, const fpe_t<B>& r){ return inte_t<64,false>(_fseq(l.encrypted(), r.encrypted())); }
template <std::size_t B> inline auto operator!=(const fpe_t<B>& l, const fpe_t<B>& r){ return inte_t<64,false>(_fsne(l.encrypted(), r.encrypted())); }
template <std::size_t B> inline auto operator<(const fpe_t<B>& l, const fpe_t<B>& r){ return inte_t<64,false>(_fslt(l.encrypted(), r.encrypted())); }
template <std::size_t B> inline auto operator<=(const fpe_t<B>& l, const fpe_t<B>& r){ return inte_t<64,false>(_fsle(l.encrypted(), r.encrypted())); }
template <std::size_t B> inline auto operator>(const fpe_t<B>& l, const fpe_t<B>& r){ return inte_t<64,false>(_fsgt(l.encrypted(), r.encrypted())); }
template <std::size_t B> inline auto operator>=(const fpe_t<B>& l, const fpe_t<B>& r){ return inte_t<64,false>(_fsge(l.encrypted(), r.encrypted())); }
template <std::size_t B> inline auto operator==(const fpe_t<B>& l, typename fpe_t<B>::value_type r){ return inte_t<64,false>(_fseqi(l.encrypted(), static_cast<double>(r))); }
template <std::size_t B> inline auto operator!=(const fpe_t<B>& l, typename fpe_t<B>::value_type r){ return inte_t<64,false>(_fsnei(l.encrypted(), static_cast<double>(r))); }
template <std::size_t B> inline auto operator<(const fpe_t<B>& l, typename fpe_t<B>::value_type r){ return inte_t<64,false>(_fslti(l.encrypted(), static_cast<double>(r))); }
template <std::size_t B> inline auto operator<=(const fpe_t<B>& l, typename fpe_t<B>::value_type r){ return inte_t<64,false>(_fslei(l.encrypted(), static_cast<double>(r))); }
template <std::size_t B> inline auto operator>(const fpe_t<B>& l, typename fpe_t<B>::value_type r){ return inte_t<64,false>(_fsgti(l.encrypted(), static_cast<double>(r))); }
template <std::size_t B> inline auto operator>=(const fpe_t<B>& l, typename fpe_t<B>::value_type r){ return inte_t<64,false>(_fsgei(l.encrypted(), static_cast<double>(r))); }
template <std::size_t B> inline auto operator==(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return r == l; }
template <std::size_t B> inline auto operator!=(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return r != l; }
template <std::size_t B> inline auto operator<(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return inte_t<64,false>(_fsgti(r.encrypted(), static_cast<double>(l))); }
template <std::size_t B> inline auto operator<=(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return inte_t<64,false>(_fsgei(r.encrypted(), static_cast<double>(l))); }
template <std::size_t B> inline auto operator>(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return inte_t<64,false>(_fslti(r.encrypted(), static_cast<double>(l))); }
template <std::size_t B> inline auto operator>=(typename fpe_t<B>::value_type l, const fpe_t<B>& r){ return inte_t<64,false>(_fslei(r.encrypted(), static_cast<double>(l))); }

using uint8e_t = inte_t<8, false>;
using uint16e_t = inte_t<16, false>;
using uint32e_t = inte_t<32, false>;
using uint64_generic_t = inte_t<64, false>;
using uint64e_t = inte_t<64, false>;
using int8e_t = inte_t<8, true>;
using int16e_t = inte_t<16, true>;
using int32e_t = inte_t<32, true>;
using int64_generic_t = inte_t<64, true>;
using int64e_t = inte_t<64, true>;
using fp32e_t = fpe_t<32>;
using fp64_generic_t = fpe_t<64>;
using fp64e_t = fpe_t<64>;

/* Generate a fresh certified random value. SiteId identifies this leaf in the
 * proof graph and has no effect on the sampled value. */
template <unsigned SiteId>
inline uint64e_t certified_random() { return uint64e_t(_certrng<SiteId>()); }

// Integer / special functions.
// Conditional move for encrypted integer results.
template <std::size_t B, bool S>
inline inte_t<B,S> cmov(const inte_t<64,false>& predicate, const inte_t<B,S>& if_true, const inte_t<B,S>& if_false) {
  return inte_t<B,S>(_cmov(predicate.encrypted(), if_true.encrypted(), if_false.encrypted()));
}

template <std::size_t B, bool S, bool PS>
inline inte_t<B,S> cmov(const inte_t<64,PS>& predicate, const inte_t<B,S>& if_true, const inte_t<B,S>& if_false) {
  return cmov(inte_t<64, false>(predicate), if_true, if_false);
}

template <typename T, typename std::enable_if<std::is_integral<T>::value && !std::is_same<typename std::decay<T>::type, bool>::value, int>::type = 0>
inline auto cmov(const inte_t<64,false>& predicate, T if_true, T if_false) {
  using result_t = inte_t<sizeof(T) * 8u, std::is_signed<T>::value>;
  return cmov(predicate,
              result_t(static_cast<typename result_t::value_type>(if_true)),
              result_t(static_cast<typename result_t::value_type>(if_false)));
}

template <typename T, bool PS, typename std::enable_if<std::is_integral<T>::value && !std::is_same<typename std::decay<T>::type, bool>::value, int>::type = 0>
inline auto cmov(const inte_t<64,PS>& predicate, T if_true, T if_false) {
  return cmov(inte_t<64, false>(predicate), if_true, if_false);
}

template <typename Pred, std::size_t B, bool S, typename = typename std::enable_if<std::is_integral<Pred>::value>::type>
inline inte_t<B,S> cmov(Pred predicate, const inte_t<B,S>& if_true, const inte_t<B,S>& if_false) {
  return cmov(inte_t<64, false>(static_cast<uint64_t>(predicate)), if_true, if_false);
}

template <typename Pred, std::size_t B, bool S, typename T,
          typename std::enable_if<std::is_integral<Pred>::value &&
                                  std::is_integral<T>::value &&
                                  !std::is_same<typename std::decay<T>::type, bool>::value, int>::type = 0>
inline inte_t<B,S> cmov(Pred predicate, T if_true, const inte_t<B,S>& if_false) {
  using plain_t = typename inte_t<B,S>::value_type;
  return cmov(predicate, inte_t<B,S>(static_cast<plain_t>(if_true)), if_false);
}

template <bool PS, std::size_t B, bool S, typename T,
          typename std::enable_if<std::is_integral<T>::value &&
                                  !std::is_same<typename std::decay<T>::type, bool>::value, int>::type = 0>
inline inte_t<B,S> cmov(const inte_t<64,PS>& predicate, T if_true, const inte_t<B,S>& if_false) {
  using plain_t = typename inte_t<B,S>::value_type;
  return cmov(predicate, inte_t<B,S>(static_cast<plain_t>(if_true)), if_false);
}

template <typename Pred, std::size_t B, bool S, typename T,
          typename std::enable_if<std::is_integral<Pred>::value &&
                                  std::is_integral<T>::value &&
                                  !std::is_same<typename std::decay<T>::type, bool>::value, int>::type = 0>
inline inte_t<B,S> cmov(Pred predicate, const inte_t<B,S>& if_true, T if_false) {
  using plain_t = typename inte_t<B,S>::value_type;
  return cmov(predicate, if_true, inte_t<B,S>(static_cast<plain_t>(if_false)));
}

template <bool PS, std::size_t B, bool S, typename T,
          typename std::enable_if<std::is_integral<T>::value &&
                                  !std::is_same<typename std::decay<T>::type, bool>::value, int>::type = 0>
inline inte_t<B,S> cmov(const inte_t<64,PS>& predicate, const inte_t<B,S>& if_true, T if_false) {
  using plain_t = typename inte_t<B,S>::value_type;
  return cmov(predicate, if_true, inte_t<B,S>(static_cast<plain_t>(if_false)));
}

// Floating point / special functions.
// Conditional move overloads for encrypted floating-point results.
template <std::size_t B>
inline fpe_t<B> cmov(const inte_t<64,false>& predicate, const fpe_t<B>& if_true, const fpe_t<B>& if_false) {
  return fpe_t<B>(_fcmov(predicate.encrypted(), if_true.encrypted(), if_false.encrypted()));
}
template <std::size_t B, bool PS>
inline fpe_t<B> cmov(const inte_t<64,PS>& predicate, const fpe_t<B>& if_true, const fpe_t<B>& if_false) {
  return cmov(inte_t<64, false>(predicate), if_true, if_false);
}
template <typename Pred, std::size_t B, typename = typename std::enable_if<std::is_integral<Pred>::value>::type>
inline fpe_t<B> cmov(Pred predicate, const fpe_t<B>& if_true, const fpe_t<B>& if_false) {
  return cmov(inte_t<64, false>(static_cast<uint64_t>(predicate)), if_true, if_false);
}

template <typename Pred, std::size_t B, typename T,
          typename std::enable_if<std::is_integral<Pred>::value &&
                                  std::is_floating_point<T>::value, int>::type = 0>
inline fpe_t<B> cmov(Pred predicate, T if_true, const fpe_t<B>& if_false) {
  using plain_t = typename fpe_t<B>::value_type;
  return cmov(predicate, fpe_t<B>(static_cast<plain_t>(if_true)), if_false);
}

template <bool PS, std::size_t B, typename T,
          typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
inline fpe_t<B> cmov(const inte_t<64,PS>& predicate, T if_true, const fpe_t<B>& if_false) {
  using plain_t = typename fpe_t<B>::value_type;
  return cmov(predicate, fpe_t<B>(static_cast<plain_t>(if_true)), if_false);
}

template <typename Pred, std::size_t B, typename T,
          typename std::enable_if<std::is_integral<Pred>::value &&
                                  std::is_floating_point<T>::value, int>::type = 0>
inline fpe_t<B> cmov(Pred predicate, const fpe_t<B>& if_true, T if_false) {
  using plain_t = typename fpe_t<B>::value_type;
  return cmov(predicate, if_true, fpe_t<B>(static_cast<plain_t>(if_false)));
}

template <bool PS, std::size_t B, typename T,
          typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
inline fpe_t<B> cmov(const inte_t<64,PS>& predicate, const fpe_t<B>& if_true, T if_false) {
  using plain_t = typename fpe_t<B>::value_type;
  return cmov(predicate, if_true, fpe_t<B>(static_cast<plain_t>(if_false)));
}

template <typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
inline auto cmov(const inte_t<64,false>& predicate, T if_true, T if_false) {
  using result_t = fpe_t<sizeof(T) * 8u>;
  return cmov(predicate,
              result_t(static_cast<typename result_t::value_type>(if_true)),
              result_t(static_cast<typename result_t::value_type>(if_false)));
}

template <typename T, bool PS, typename std::enable_if<std::is_floating_point<T>::value, int>::type = 0>
inline auto cmov(const inte_t<64,PS>& predicate, T if_true, T if_false) {
  return cmov(inte_t<64, false>(predicate), if_true, if_false);
}

template <typename Pred, typename T, typename std::enable_if<std::is_integral<Pred>::value && ((std::is_integral<T>::value && !std::is_same<typename std::decay<T>::type, bool>::value) || std::is_floating_point<T>::value), int>::type = 0>
inline auto cmov(Pred predicate, T if_true, T if_false) {
  return cmov(inte_t<64, false>(static_cast<uint64_t>(predicate)), if_true, if_false);
}

// Absolute value for encrypted FP64.
inline fp64e_t fabs(const fp64e_t& value) { return fp64e_t(_fabs(value.encrypted())); }
}  // namespace exo

using exo::cmov;
using exo::certified_random;
using exo::datagrant_t;
using exo::debug_context;
using exo::fp32e_t;
using exo::fp64e_t;
using exo::fp64_generic_t;
using exo::fpe_t;
using exo::int8e_t;
using exo::int16e_t;
using exo::int32e_t;
using exo::int64e_t;
using exo::int64_generic_t;
using exo::inte_t;
using exo::uint8e_t;
using exo::uint16e_t;
using exo::uint32e_t;
using exo::uint64e_t;
using exo::uint64_generic_t;

#endif  // __cplusplus

#endif  // MOJOV_EXO_H
