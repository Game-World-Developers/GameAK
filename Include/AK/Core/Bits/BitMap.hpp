#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Bits {
class Bitmap {
public:
  Bitmap(u64 *data, usize words) noexcept : m_data(data), m_words(words) {}

  void set(usize bit) noexcept { m_data[word(bit)] |= mask(bit); }

  void clear(usize bit) noexcept { m_data[word(bit)] &= ~mask(bit); }

  bool test(usize bit) const noexcept { return m_data[word(bit)] & mask(bit); }

  void reset() noexcept {
    for (usize i = 0; i < m_words; ++i) {
      m_data[i] = 0;
    }
  }

private:
  static constexpr usize kBitsPerWord = 64;

  u64 *m_data;
  usize m_words;

  static constexpr usize word(usize bit) noexcept { return bit / kBitsPerWord; }

  static constexpr u64 mask(usize bit) noexcept { return u64(1) << (bit % kBitsPerWord); }
};
} // namespace GameAK::Bits
