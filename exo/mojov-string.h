#ifndef MOJOV_STRING_H
#define MOJOV_STRING_H

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#include "mojov-exo.h"

namespace exo {

/**
 * @brief Fixed-capacity encrypted string container.
 *
 * stringe_t stores encrypted bytes packed into encrypted 64-bit words
 * (8 encrypted uint8e_t lanes per uint64e_t word).
 *
 * Security model in this type:
 * - length() is public and fixed for each allocation.
 * - size(), empty(), comparisons, find results, and exception() are encrypted.
 * - exception() is sticky: once non-zero, it remains non-zero.
 */
class stringe_t {
 public:
  using value_type = uint8e_t;
  using size_type = std::size_t;

  static constexpr size_type npos = static_cast<size_type>(-1);
  static constexpr uint64_t kNposPlain = ~uint64_t(0);

  /** @brief Construct an empty encrypted string with public capacity length. */
  explicit stringe_t(size_type length = 0)
      : length_(length), words_(allocate_words(length)), size_(0), exception_(0) {}

  stringe_t(const stringe_t& rhs)
      : length_(rhs.length_), words_(allocate_words(rhs.length_)), size_(rhs.size_), exception_(rhs.exception_) {
    copy_words(words_, rhs.words_, word_count(length_));
  }

  stringe_t(stringe_t&& rhs) noexcept
      : length_(rhs.length_), words_(rhs.words_), size_(rhs.size_), exception_(rhs.exception_) {
    rhs.length_ = 0;
    rhs.words_ = nullptr;
    rhs.size_ = uint64e_t(0);
    rhs.exception_ = uint64e_t(0);
  }

  ~stringe_t() { release_words(); }

  /**
   * @brief Assignment with realloc-on-length-mismatch semantics.
   *
   * If lengths match, storage is reused. Otherwise storage is reallocated
   * for destination length.
   */
  stringe_t& operator=(const stringe_t& rhs) {
    if (this == &rhs) {
      return *this;
    }
    if (length_ != rhs.length_) {
      replace_storage(rhs.length_);
    }
    copy_words(words_, rhs.words_, word_count(length_));
    size_ = rhs.size_;
    exception_ = rhs.exception_;
    return *this;
  }

  stringe_t& operator=(stringe_t&& rhs) noexcept {
    if (this == &rhs) {
      return *this;
    }
    release_words();
    length_ = rhs.length_;
    words_ = rhs.words_;
    size_ = rhs.size_;
    exception_ = rhs.exception_;
    rhs.length_ = 0;
    rhs.words_ = nullptr;
    rhs.size_ = uint64e_t(0);
    rhs.exception_ = uint64e_t(0);
    return *this;
  }

  /** @brief Public fixed capacity in bytes. */
  size_type length() const { return length_; }

  /** @brief Encrypted current size. */
  uint64e_t size() const { return size_; }

  /** @brief Encrypted empty predicate: size()==0. */
  uint64e_t empty() const { return size_ == uint64e_t(0); }

  /** @brief Sticky encrypted exception flag. Non-zero means an invalid operation occurred. */
  uint64e_t exception() const { return exception_; }

  /** @brief Trap if i >= length(). Otherwise return encrypted byte at i. */
  uint8e_t operator[](size_type i) const {
    if (i >= length_) {
      __builtin_trap();
    }
    return get_char_public(i);
  }

  /** @brief Append one encrypted byte. Sets exception on overflow. */
  stringe_t& push_back(uint8e_t ch) {
    const uint64e_t fits = size_ < uint64e_t(length_);
    oblivious_write(size_, ch, fits);
    size_ = cmov(fits, size_ + uint64e_t(1), size_);
    exception_ = exception_ | (uint64e_t(1) - fits);
    return *this;
  }

  /** @brief Append another encrypted string. */
  stringe_t& append(const stringe_t& rhs) {
    for (size_type i = 0; i < rhs.length_; ++i) {
      const uint64e_t in_rhs = uint64e_t(i) < rhs.size_;
      const uint64e_t fits = size_ < uint64e_t(length_);
      const uint64e_t do_write = in_rhs & fits;
      const uint8e_t ch = rhs.get_char_public(i);
      oblivious_write(size_, ch, do_write);
      size_ = cmov(do_write, size_ + uint64e_t(1), size_);
      exception_ = exception_ | (in_rhs & (uint64e_t(1) - fits));
    }
    exception_ = exception_ | rhs.exception_;
    return *this;
  }

  /** @brief Append one encrypted byte alias for push_back(). */
  stringe_t& append(uint8e_t ch) { return push_back(ch); }

  stringe_t& operator+=(const stringe_t& rhs) { return append(rhs); }
  stringe_t& operator+=(uint8e_t ch) { return push_back(ch); }

  /** @brief Concatenate into a new string with length = lhs.length()+rhs.length(). */
  friend stringe_t operator+(const stringe_t& lhs, const stringe_t& rhs) {
    stringe_t out(lhs.length_ + rhs.length_);
    out.append(lhs);
    out.append(rhs);
    out.exception_ = out.exception_ | lhs.exception_ | rhs.exception_;
    return out;
  }

  /** @brief Lexicographic compare. Returns encrypted {-1,0,+1}. */
  int64e_t compare(const stringe_t& rhs) const {
    int64e_t result(0);
    uint64e_t decided(0);
    const size_type m = (length_ > rhs.length_) ? length_ : rhs.length_;

    for (size_type i = 0; i < m; ++i) {
      const uint64e_t in_l = uint64e_t(i) < size_;
      const uint64e_t in_r = uint64e_t(i) < rhs.size_;
      const uint8e_t lc = (i < length_) ? get_char_public(i) : uint8e_t(0);
      const uint8e_t rc = (i < rhs.length_) ? rhs.get_char_public(i) : uint8e_t(0);

      const uint64e_t l_only = in_l & (uint64e_t(1) - in_r);
      const uint64e_t r_only = in_r & (uint64e_t(1) - in_l);
      const uint64e_t both = in_l & in_r;

      const uint64e_t lt = both & (lc < rc);
      const uint64e_t gt = both & (lc > rc);
      const uint64e_t choose = uint64e_t(1) - decided;

      result = cmov(choose & (lt | r_only), int64e_t(-1), result);
      result = cmov(choose & (gt | l_only), int64e_t(1), result);
      decided = decided | l_only | r_only | lt | gt;
    }
    return result;
  }

  int64e_t operator<=>(const stringe_t& rhs) const { return compare(rhs); }
  uint64e_t operator==(const stringe_t& rhs) const { return compare(rhs) == int64e_t(0); }
  uint64e_t operator!=(const stringe_t& rhs) const { return compare(rhs) != int64e_t(0); }
  uint64e_t operator<(const stringe_t& rhs) const { return compare(rhs) < int64e_t(0); }
  uint64e_t operator<=(const stringe_t& rhs) const { return compare(rhs) <= int64e_t(0); }
  uint64e_t operator>(const stringe_t& rhs) const { return compare(rhs) > int64e_t(0); }
  uint64e_t operator>=(const stringe_t& rhs) const { return compare(rhs) >= int64e_t(0); }

  /** @brief Find first occurrence of needle; returns encrypted position or npos. */
  uint64e_t find(const stringe_t& needle, uint64e_t pos = uint64e_t(0)) const {
    uint64e_t best(kNposPlain);
    uint64e_t found(0);
    for (size_type i = 0; i < length_; ++i) {
      const uint64e_t i_ok = uint64e_t(i) >= pos;
      const uint64e_t i_in = uint64e_t(i) < size_;
      uint64e_t match = i_ok & i_in;
      for (size_type j = 0; j < needle.length_; ++j) {
        const uint64e_t n_in = uint64e_t(j) < needle.size_;
        const uint64e_t s_in = uint64e_t(i + j) < size_;
        const uint8e_t a = get_char_or_zero(i + j);
        const uint8e_t b = needle.get_char_public(j);
        match = match & cmov(n_in, s_in & (a == b), uint64e_t(1));
      }
      match = match & (needle.size_ > uint64e_t(0));
      const uint64e_t choose = match & (uint64e_t(1) - found);
      best = cmov(choose, uint64e_t(i), best);
      found = found | match;
    }
    const uint64e_t empty_needle = needle.size_ == uint64e_t(0);
    const uint64e_t size_ge_pos = size_ >= pos;
    return cmov(empty_needle & size_ge_pos, pos, best);
  }

  uint64e_t find(uint8e_t ch, uint64e_t pos = uint64e_t(0)) const {
    uint64e_t best(kNposPlain);
    uint64e_t found(0);
    for (size_type i = 0; i < length_; ++i) {
      const uint64e_t match = (uint64e_t(i) >= pos) & (uint64e_t(i) < size_) & (get_char_public(i) == ch);
      const uint64e_t choose = match & (uint64e_t(1) - found);
      best = cmov(choose, uint64e_t(i), best);
      found = found | match;
    }
    return best;
  }

  uint64e_t rfind(uint8e_t ch) const {
    uint64e_t best(kNposPlain);
    for (size_type i = 0; i < length_; ++i) {
      const uint64e_t match = (uint64e_t(i) < size_) & (get_char_public(i) == ch);
      best = cmov(match, uint64e_t(i), best);
    }
    return best;
  }

  /** @brief Find last occurrence of substring needle. Returns encrypted npos if not found. */
  uint64e_t rfind(const stringe_t& needle, uint64e_t pos = uint64e_t(kNposPlain)) const {
    uint64e_t best(kNposPlain);
    for (size_type i = 0; i < length_; ++i) {
      const uint64e_t i_in = uint64e_t(i) < size_;
      const uint64e_t i_ok = cmov(pos == uint64e_t(kNposPlain), uint64e_t(1), uint64e_t(i) <= pos);
      uint64e_t match = i_in & i_ok;
      for (size_type j = 0; j < needle.length_; ++j) {
        const uint64e_t n_in = uint64e_t(j) < needle.size_;
        const uint64e_t s_in = uint64e_t(i + j) < size_;
        match = match & cmov(n_in, s_in & (get_char_or_zero(i + j) == needle.get_char_public(j)), uint64e_t(1));
      }
      match = match & (needle.size_ > uint64e_t(0));
      best = cmov(match, uint64e_t(i), best);
    }
    const uint64e_t empty_needle = needle.size_ == uint64e_t(0);
    const uint64e_t ret_pos = cmov(pos == uint64e_t(kNposPlain), size_, pos);
    const uint64e_t size_ge_pos = size_ >= ret_pos;
    return cmov(empty_needle & size_ge_pos, ret_pos, best);
  }

  uint64e_t find_first_of(const stringe_t& chars, uint64e_t pos = uint64e_t(0)) const {
    uint64e_t best(kNposPlain);
    uint64e_t found(0);
    for (size_type i = 0; i < length_; ++i) {
      uint64e_t any(0);
      for (size_type j = 0; j < chars.length_; ++j) {
        const uint64e_t c_in = uint64e_t(j) < chars.size_;
        any = any | (c_in & (get_char_public(i) == chars.get_char_public(j)));
      }
      const uint64e_t match = (uint64e_t(i) >= pos) & (uint64e_t(i) < size_) & any;
      const uint64e_t choose = match & (uint64e_t(1) - found);
      best = cmov(choose, uint64e_t(i), best);
      found = found | match;
    }
    return best;
  }

  uint64e_t find_last_of(const stringe_t& chars) const {
    uint64e_t best(kNposPlain);
    for (size_type i = 0; i < length_; ++i) {
      uint64e_t any(0);
      for (size_type j = 0; j < chars.length_; ++j) {
        const uint64e_t c_in = uint64e_t(j) < chars.size_;
        any = any | (c_in & (get_char_public(i) == chars.get_char_public(j)));
      }
      const uint64e_t match = (uint64e_t(i) < size_) & any;
      best = cmov(match, uint64e_t(i), best);
    }
    return best;
  }

  uint64e_t find_first_of(uint8e_t ch, uint64e_t pos = uint64e_t(0)) const {
    return find(ch, pos);
  }

  uint64e_t find_last_of(uint8e_t ch) const {
    return rfind(ch);
  }

  uint64e_t find_first_not_of(const stringe_t& chars, uint64e_t pos = uint64e_t(0)) const {
    uint64e_t best(kNposPlain);
    uint64e_t found(0);
    for (size_type i = 0; i < length_; ++i) {
      uint64e_t any(0);
      for (size_type j = 0; j < chars.length_; ++j) {
        const uint64e_t c_in = uint64e_t(j) < chars.size_;
        any = any | (c_in & (get_char_public(i) == chars.get_char_public(j)));
      }
      const uint64e_t match = (uint64e_t(i) >= pos) & (uint64e_t(i) < size_) & (uint64e_t(1) - any);
      const uint64e_t choose = match & (uint64e_t(1) - found);
      best = cmov(choose, uint64e_t(i), best);
      found = found | match;
    }
    return best;
  }

  uint64e_t find_last_not_of(const stringe_t& chars) const {
    uint64e_t best(kNposPlain);
    for (size_type i = 0; i < length_; ++i) {
      uint64e_t any(0);
      for (size_type j = 0; j < chars.length_; ++j) {
        const uint64e_t c_in = uint64e_t(j) < chars.size_;
        any = any | (c_in & (get_char_public(i) == chars.get_char_public(j)));
      }
      const uint64e_t match = (uint64e_t(i) < size_) & (uint64e_t(1) - any);
      best = cmov(match, uint64e_t(i), best);
    }
    return best;
  }

  uint64e_t find_first_not_of(uint8e_t ch, uint64e_t pos = uint64e_t(0)) const {
    uint64e_t best(kNposPlain);
    uint64e_t found(0);
    for (size_type i = 0; i < length_; ++i) {
      const uint64e_t match = (uint64e_t(i) >= pos) & (uint64e_t(i) < size_) & (get_char_public(i) != ch);
      const uint64e_t choose = match & (uint64e_t(1) - found);
      best = cmov(choose, uint64e_t(i), best);
      found = found | match;
    }
    return best;
  }

  uint64e_t find_last_not_of(uint8e_t ch) const {
    uint64e_t best(kNposPlain);
    for (size_type i = 0; i < length_; ++i) {
      const uint64e_t match = (uint64e_t(i) < size_) & (get_char_public(i) != ch);
      best = cmov(match, uint64e_t(i), best);
    }
    return best;
  }

  /** @brief Return a fixed-length substring buffer. out.length() == count. */
  stringe_t substr(uint64e_t pos, size_type count) const {
    stringe_t out(count);
    for (size_type i = 0; i < count; ++i) {
      const uint64e_t src = pos + uint64e_t(i);
      const uint64e_t in = src < size_;
      out.oblivious_write(uint64e_t(i), get_char_or_zero_public_src(src, i), in);
      out.size_ = cmov(in, out.size_ + uint64e_t(1), out.size_);
    }
    out.exception_ = exception_;
    return out;
  }

  uint64e_t starts_with(const stringe_t& prefix) const {
    uint64e_t ok = size_ >= prefix.size_;
    for (size_type i = 0; i < prefix.length_; ++i) {
      const uint64e_t in = uint64e_t(i) < prefix.size_;
      ok = ok & cmov(in, get_char_or_zero(i) == prefix.get_char_public(i), uint64e_t(1));
    }
    return ok;
  }

  uint64e_t ends_with(const stringe_t& suffix) const {
    uint64e_t ok = size_ >= suffix.size_;
    const uint64e_t start = size_ - suffix.size_;
    for (size_type i = 0; i < suffix.length_; ++i) {
      const uint64e_t in = uint64e_t(i) < suffix.size_;
      ok = ok & cmov(in, get_char_by_secret_index(start + uint64e_t(i)) == suffix.get_char_public(i), uint64e_t(1));
    }
    return ok;
  }

  uint64e_t contains(const stringe_t& needle) const {
    return find(needle) != uint64e_t(kNposPlain);
  }

 private:
  static size_type word_count(size_type length) {
    return (length + 7u) / 8u;
  }

  static uint64e_t* allocate_words(size_type length) {
    const size_type n = word_count(length);
    if (n == 0) {
      return nullptr;
    }
    uint64e_t* ptr = static_cast<uint64e_t*>(std::malloc(n * sizeof(uint64e_t)));
    if (ptr == nullptr) {
      __builtin_trap();
    }
    for (size_type i = 0; i < n; ++i) {
      ptr[i] = uint64e_t(0);
    }
    return ptr;
  }

  static void copy_words(uint64e_t* dst, const uint64e_t* src, size_type n) {
    for (size_type i = 0; i < n; ++i) {
      dst[i] = src[i];
    }
  }

  void release_words() {
    if (words_ != nullptr) {
      std::free(words_);
      words_ = nullptr;
    }
  }

  void replace_storage(size_type new_length) {
    release_words();
    length_ = new_length;
    words_ = allocate_words(new_length);
  }

  static uint64e_t lane_shift(size_type lane) {
    return uint64e_t(static_cast<uint64_t>((lane & 7u) * 8u));
  }

  uint8e_t get_char_public(size_type idx) const {
    const size_type w = idx / 8u;
    const size_type lane = idx % 8u;
    const uint64e_t word = words_[w];
    return uint8e_t((word >> lane_shift(lane)) & uint64e_t(0xFFu));
  }

  uint8e_t get_char_or_zero(size_type idx) const {
    if (idx >= length_) {
      return uint8e_t(0);
    }
    return get_char_public(idx);
  }

  uint8e_t get_char_or_zero_public_src(uint64e_t src, size_type fallback_public) const {
    uint8e_t result(0);
    for (size_type k = 0; k < length_; ++k) {
      const uint64e_t eq = src == uint64e_t(k);
      result = cmov(eq, get_char_public(k), result);
    }
    if (length_ == 0 && fallback_public == 0) {
      return uint8e_t(0);
    }
    return result;
  }

  uint8e_t get_char_by_secret_index(uint64e_t idx) const {
    uint8e_t result(0);
    for (size_type k = 0; k < length_; ++k) {
      const uint64e_t eq = idx == uint64e_t(k);
      result = cmov(eq, get_char_public(k), result);
    }
    return result;
  }

  void set_char_public(size_type idx, uint8e_t ch) {
    const size_type w = idx / 8u;
    const size_type lane = idx % 8u;
    const uint64e_t shift = lane_shift(lane);
    const uint64e_t mask = uint64e_t(0xFFu) << shift;
    words_[w] = (words_[w] & ~mask) | (uint64e_t(ch) << shift);
  }

  void oblivious_write(uint64e_t idx, uint8e_t ch, uint64e_t enable) {
    for (size_type k = 0; k < length_; ++k) {
      const uint64e_t take = enable & (idx == uint64e_t(k));
      const uint8e_t cur = get_char_public(k);
      set_char_public(k, cmov(take, ch, cur));
    }
  }

  size_type length_;
  uint64e_t* words_;
  uint64e_t size_;
  uint64e_t exception_;
};

}  // namespace exo

#endif  // MOJOV_STRING_H
