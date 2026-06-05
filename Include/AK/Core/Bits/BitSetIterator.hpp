/// @file
/// @brief Iterator over set bits in a u64 bit array.
///
/// Provides sequential iteration over set (1) bits using ctz-based
/// word scanning. Typical use with BitArray:
/// @code
///   BitSetIterator it(arr.data(), arr.words());
///   for (usize bit = it.next(); bit < arr.words() * 64; bit = it.next()) {
///     // process set bit
///   }
/// @endcode

#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

/// Lightweight forward iterator over set bits in a u64 array.
class BitSetIterator {
public:
  BitSetIterator(const u64* data, usize words) noexcept
      : m_data(data), m_words(words), m_current(0) {}

  /// Returns the index of the next set bit, or @p words*64 if none.
  usize next() noexcept {
    usize word_idx = m_current / 64;
    usize offset = m_current % 64;

    if (word_idx >= m_words) return m_words * 64;

    // Check remainder of current word
    u64 word = m_data[word_idx] & (~u64(0) << offset);
    if (word != 0) {
      m_current = word_idx * 64 + ctz(word);
      return m_current++;
    }

    // Scan remaining words
    for (usize i = word_idx + 1; i < m_words; ++i) {
      if (m_data[i] != 0) {
        m_current = i * 64 + ctz(m_data[i]);
        return m_current++;
      }
    }

    m_current = m_words * 64;
    return m_current;
  }

  /// Resets iteration to the beginning.
  void reset() noexcept { m_current = 0; }

  /// @return Current bit position (useful for save/restore).
  usize tell() const noexcept { return m_current; }

private:
  const u64* m_data;
  usize m_words;
  usize m_current;
};

} // namespace GameAK::Bits
