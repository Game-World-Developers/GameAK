#pragma once

#include <AK/Backend/ScalarBuiltinsBackend.hpp>
#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

using Backend::ScalarBuiltinsBackend;

enum class BitCheck : u8 { None, Bounded };

template <typename Backend = ScalarBuiltinsBackend, BitCheck Check = BitCheck::None>
class BitArray {
public:
  BitArray(u64 *data, usize words) noexcept : m_data(data), m_words(words) {}

  BitArray(const BitArray &) = delete;
  BitArray &operator=(const BitArray &) = delete;
  BitArray(BitArray &&) = delete;
  BitArray &operator=(BitArray &&) = delete;

  void set(usize bit) noexcept {
    if constexpr (Check == BitCheck::Bounded) {
      if (bit / kBitsPerWord >= m_words) {
        return;
      }
    }
    m_data[word(bit)] |= bit_mask(bit);
  }

  void clear(usize bit) noexcept {
    if constexpr (Check == BitCheck::Bounded) {
      if (bit / kBitsPerWord >= m_words) {
        return;
      }
    }
    m_data[word(bit)] &= ~bit_mask(bit);
  }

  bool test(usize bit) const noexcept {
    if constexpr (Check == BitCheck::Bounded) {
      if (bit / kBitsPerWord >= m_words) {
        return false;
      }
    }
    return (m_data[word(bit)] & bit_mask(bit)) != 0;
  }

  void reset() noexcept {
    (void)Backend::mem_set(m_data, 0, m_words * sizeof(u64));
  }

  void and_with(const BitArray &other) noexcept {
    usize i = 0;
    for (; i + 4 <= m_words; i += 4)
      Backend::bit_and_256(m_data + i, m_data + i, other.m_data + i);
    for (; i < m_words; ++i)
      m_data[i] &= other.m_data[i];
  }

  void or_with(const BitArray &other) noexcept {
    usize i = 0;
    for (; i + 4 <= m_words; i += 4)
      Backend::bit_or_256(m_data + i, m_data + i, other.m_data + i);
    for (; i < m_words; ++i)
      m_data[i] |= other.m_data[i];
  }

  void xor_with(const BitArray &other) noexcept {
    usize i = 0;
    for (; i + 4 <= m_words; i += 4)
      Backend::bit_xor_256(m_data + i, m_data + i, other.m_data + i);
    for (; i < m_words; ++i)
      m_data[i] ^= other.m_data[i];
  }

  void negate() noexcept {
    usize i = 0;
    for (; i + 4 <= m_words; i += 4)
      Backend::bit_not_256(m_data + i, m_data + i);
    for (; i < m_words; ++i)
      m_data[i] = ~m_data[i];
  }

  usize popcount() const noexcept {
    usize total = 0;
    usize i = 0;
    for (; i + 4 <= m_words; i += 4)
      total += Backend::popcount_256(m_data + i);
    for (; i < m_words; ++i)
      total += static_cast<usize>(Bits::popcount(m_data[i]));
    return total;
  }

  bool any() const noexcept {
    for (usize i = 0; i < m_words; ++i) {
      if (m_data[i] != 0)
        return true;
    }
    return false;
  }

  bool none() const noexcept { return !any(); }

  bool all() const noexcept {
    for (usize i = 0; i < m_words; ++i) {
      if (m_data[i] != ~u64(0))
        return false;
    }
    return true;
  }

  usize find_first_set() const noexcept {
    for (usize i = 0; i < m_words; ++i) {
      if (m_data[i] != 0) {
        return i * kBitsPerWord + ctz(m_data[i]);
      }
    }
    return m_words * kBitsPerWord;
  }

  usize find_next_set(usize prev) const noexcept {
    usize bit = prev + 1;
    usize word_idx = bit / kBitsPerWord;
    if (word_idx >= m_words)
      return m_words * kBitsPerWord;

    u64 word = m_data[word_idx] & (~u64(0) << (bit % kBitsPerWord));
    if (word != 0)
      return word_idx * kBitsPerWord + ctz(word);

    for (usize i = word_idx + 1; i < m_words; ++i) {
      if (m_data[i] != 0)
        return i * kBitsPerWord + ctz(m_data[i]);
    }
    return m_words * kBitsPerWord;
  }

  void set_range(usize first, usize last) noexcept {
    if (first > last)
      return;
    usize fw = first / kBitsPerWord;
    usize lw = last / kBitsPerWord;

    if (fw == lw) {
      u64 m = range_mask(first % kBitsPerWord, last % kBitsPerWord);
      m_data[fw] |= m;
    } else {
      m_data[fw] |= ~u64(0) << (first % kBitsPerWord);
      for (usize i = fw + 1; i < lw; ++i)
        m_data[i] = ~u64(0);
      m_data[lw] |= ~u64(0) >> (kBitsPerWord - 1 - (last % kBitsPerWord));
    }
  }

  void clear_range(usize first, usize last) noexcept {
    if (first > last)
      return;
    usize fw = first / kBitsPerWord;
    usize lw = last / kBitsPerWord;

    if (fw == lw) {
      u64 m = range_mask(first % kBitsPerWord, last % kBitsPerWord);
      m_data[fw] &= ~m;
    } else {
      m_data[fw] &= ~(~u64(0) << (first % kBitsPerWord));
      for (usize i = fw + 1; i < lw; ++i)
        m_data[i] = 0;
      m_data[lw] &= ~(~u64(0) >> (kBitsPerWord - 1 - (last % kBitsPerWord)));
    }
  }

  usize words() const noexcept { return m_words; }
  u64 *data() noexcept { return m_data; }
  const u64 *data() const noexcept { return m_data; }

private:
  static constexpr usize kBitsPerWord = 64;

  u64 *m_data;
  usize m_words;

  static constexpr usize word(usize bit) noexcept { return bit / kBitsPerWord; }

  static constexpr u64 bit_mask(usize bit) noexcept {
    return Bits::bit(u64(bit % kBitsPerWord));
  }

  static constexpr u64 range_mask(usize low, usize high) noexcept {
    return (~u64(0) << low) & (~u64(0) >> (kBitsPerWord - 1 - high));
  }
};

} // namespace GameAK::Bits
