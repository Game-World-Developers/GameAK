#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

class BitVector {
public:
  BitVector(u64 *buffer, usize words) noexcept
      : m_data(buffer), m_words(words) {}

  void set(usize bit) noexcept {
    ensure(bit);
    m_data[word(bit)] |= mask(bit);
  }

  void clear(usize bit) noexcept {
    if (bit / kBitsPerWord < m_words)
      m_data[word(bit)] &= ~mask(bit);
  }

  bool test(usize bit) const noexcept {
    if (bit / kBitsPerWord >= m_words) {
      return false;
    }
    return m_data[word(bit)] & mask(bit);
  }

  void reset() noexcept {
    for (usize i = 0; i < m_words; ++i) {
      m_data[i] = 0;
    }
  }

private:
  static constexpr usize kBitsPerWord = 64;

  u64 *m_data;
  usize m_words;

  void ensure(usize bit) const noexcept {
    if (bit / kBitsPerWord >= m_words) {
      return;
    }
  }

  static constexpr usize word(usize bit) noexcept { return bit / kBitsPerWord; }

  static constexpr u64 mask(usize bit) noexcept { return u64(1) << (bit % kBitsPerWord); }
};
} // namespace GameAK::Bits
