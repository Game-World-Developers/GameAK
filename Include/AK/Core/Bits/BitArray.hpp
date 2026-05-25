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
    return u64(1) << (bit % kBitsPerWord);
  }
};

} // namespace GameAK::Bits
