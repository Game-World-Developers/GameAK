/// @file
/// @brief Bounded and unbounded bit array view over an external u64 buffer.
///
/// Provides set, clear, test, and bulk bitwise operations (and, or, xor, not)
/// over a fixed-size word array. Bounds checking is selectable at compile time
/// via the BitCheck template parameter.

#pragma once

#include <AK/Backend/Backend.hpp>
#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

enum class BitCheck : u8 { None, Bounded };

template <BitCheck Check = BitCheck::None>
class BitArray {
public:
  BitArray(u64* data, usize words) noexcept
      : m_data(data), m_words(words) {}

  BitArray(const BitArray&) = delete;
  BitArray& operator=(const BitArray&) = delete;
  BitArray(BitArray&&) = delete;
  BitArray& operator=(BitArray&&) = delete;

  void set(usize bit) noexcept {
    if constexpr (Check == BitCheck::Bounded) {
      if (bit / kBitsPerWord >= m_words) { return; }
    }
    m_data[word(bit)] |= bit_mask(bit);
  }

  void clear(usize bit) noexcept {
    if constexpr (Check == BitCheck::Bounded) {
      if (bit / kBitsPerWord >= m_words) { return; }
    }
    m_data[word(bit)] &= ~bit_mask(bit);
  }

  bool test(usize bit) const noexcept {
    if constexpr (Check == BitCheck::Bounded) {
      if (bit / kBitsPerWord >= m_words) { return false; }
    }
    return (m_data[word(bit)] & bit_mask(bit)) != 0;
  }

  void reset() noexcept {
    (void)Backend::mem_set(m_data, 0, m_words * sizeof(u64));
  }

  void and_with(const BitArray& other) noexcept {
    Backend::bitset_and(m_data, m_data, other.m_data, m_words);
  }

  void or_with(const BitArray& other) noexcept {
    Backend::bitset_or(m_data, m_data, other.m_data, m_words);
  }

  void xor_with(const BitArray& other) noexcept {
    Backend::bitset_xor(m_data, m_data, other.m_data, m_words);
  }

  void negate() noexcept {
    Backend::bitset_not(m_data, m_data, m_words);
  }

  usize popcount() const noexcept {
    return Backend::bitset_popcount_range(m_data, m_words);
  }

  bool any() const noexcept {
    for (usize i = 0; i < m_words; ++i) {
      if (m_data[i] != 0) return true;
    }
    return false;
  }

  bool none() const noexcept { return !any(); }

  bool all() const noexcept {
    for (usize i = 0; i < m_words; ++i) {
      if (m_data[i] != ~u64(0)) return false;
    }
    return true;
  }

  /// Returns the index of the first set bit, or words()*64 if none.
  usize find_first_set() const noexcept {
    for (usize i = 0; i < m_words; ++i) {
      if (m_data[i] != 0) {
        return i * kBitsPerWord + ctz(m_data[i]);
      }
    }
    return m_words * kBitsPerWord;
  }

  /// Returns the index of the next set bit after @p prev,
  /// or words()*64 if none.
  usize find_next_set(usize prev) const noexcept {
    usize bit = prev + 1;
    usize word_idx = bit / kBitsPerWord;
    if (word_idx >= m_words) return m_words * kBitsPerWord;

    u64 word = m_data[word_idx] & (~u64(0) << (bit % kBitsPerWord));
    if (word != 0) return word_idx * kBitsPerWord + ctz(word);

    for (usize i = word_idx + 1; i < m_words; ++i) {
      if (m_data[i] != 0) return i * kBitsPerWord + ctz(m_data[i]);
    }
    return m_words * kBitsPerWord;
  }

  /// Sets bits in the inclusive range [first, last].
  void set_range(usize first, usize last) noexcept {
    if (first > last) return;
    usize fw = first / kBitsPerWord;
    usize lw = last / kBitsPerWord;

    if (fw == lw) {
      u64 m = range_mask(first % kBitsPerWord, last % kBitsPerWord);
      m_data[fw] |= m;
    } else {
      m_data[fw] |= ~u64(0) << (first % kBitsPerWord);
      for (usize i = fw + 1; i < lw; ++i) m_data[i] = ~u64(0);
      m_data[lw] |= ~u64(0) >> (kBitsPerWord - 1 - (last % kBitsPerWord));
    }
  }

  /// Clears bits in the inclusive range [first, last].
  void clear_range(usize first, usize last) noexcept {
    if (first > last) return;
    usize fw = first / kBitsPerWord;
    usize lw = last / kBitsPerWord;

    if (fw == lw) {
      u64 m = range_mask(first % kBitsPerWord, last % kBitsPerWord);
      m_data[fw] &= ~m;
    } else {
      m_data[fw] &= ~(~u64(0) << (first % kBitsPerWord));
      for (usize i = fw + 1; i < lw; ++i) m_data[i] = 0;
      m_data[lw] &= ~(~u64(0) >> (kBitsPerWord - 1 - (last % kBitsPerWord)));
    }
  }

  usize words() const noexcept { return m_words; }
  u64* data() noexcept { return m_data; }
  const u64* data() const noexcept { return m_data; }

private:
  static constexpr usize kBitsPerWord = 64;

  u64*  m_data;
  usize m_words;

  static constexpr usize word(usize bit) noexcept {
    return bit / kBitsPerWord;
  }

  static constexpr u64 bit_mask(usize bit) noexcept {
    return Bits::bit(u64(bit % kBitsPerWord));
  }

  static constexpr u64 range_mask(usize low, usize high) noexcept {
    return (~u64(0) << low) & (~u64(0) >> (kBitsPerWord - 1 - high));
  }
};

} // namespace GameAK::Bits
