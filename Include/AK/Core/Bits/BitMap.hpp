/// @file
/// @brief Fixed-size bitmap backed by an external u64 array.
///
/// Provides O(1) set, clear, and test operations over a caller-supplied
/// array of 64-bit words. The bitmap does NOT own its backing storage —
/// the caller is responsible for providing and managing the buffer.
///
/// This is the minimum-overhead bitmap primitive. For a bounds-checked
/// variant see BitVector.

#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

/// Fixed-size bitmap over an external u64 buffer.
///
/// Not copyable or movable — the bitmap is tied to a specific memory
/// region. Pass by pointer or reference when ownership semantics are
/// required.
class Bitmap {
public:
  /// Constructs a bitmap over an externally-owned word array.
  ///
  /// @param data   Pointer to the backing u64 array. Must not be null.
  /// @param words  Number of u64 elements in the array.
  Bitmap(u64 *data, usize words) noexcept : m_data(data), m_words(words) {}

  /// Sets the bit at position @p bit to 1.
  ///
  /// Behaviour is undefined if @p bit exceeds the total number of
  /// addressable bits (words * 64).
  ///
  /// @param bit  Zero-indexed bit position.
  void set(usize bit) noexcept { m_data[word(bit)] |= mask(bit); }

  /// Clears the bit at position @p bit to 0.
  ///
  /// Behaviour is undefined if @p bit exceeds the total number of
  /// addressable bits (words * 64).
  ///
  /// @param bit  Zero-indexed bit position.
  void clear(usize bit) noexcept { m_data[word(bit)] &= ~mask(bit); }

  /// Returns @c true when the bit at position @p bit is 1.
  ///
  /// Behaviour is undefined if @p bit exceeds the total number of
  /// addressable bits (words * 64).
  ///
  /// @param bit  Zero-indexed bit position.
  /// @return     @c true if the bit is set, @c false otherwise.
  bool test(usize bit) const noexcept { return m_data[word(bit)] & mask(bit); }

  /// Zeros all bits in the bitmap.
  ///
  /// Equivalent to writing 0 to every word.
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
