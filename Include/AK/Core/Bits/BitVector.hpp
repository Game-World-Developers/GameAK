/// @file
/// @brief Bounds-checked bitmap over an external u64 array.
///
/// Like Bitmap, but with runtime bounds checking on every operation:
/// out-of-bounds accesses are silently ignored (set/clear) or return
/// @c false (test). This provides safer access at a small branching
/// cost per operation.
///
/// The vector does NOT own its backing storage — the caller is
/// responsible for providing and managing the buffer.

#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

/// Bounds-checked bitmap over an external u64 buffer.
///
/// Wraps a raw u64 array and guards every access with a bounds check.
/// Unlike Bitmap, operations on out-of-bounds indices either become
/// no-ops or return @c false, making this suitable for contexts where
/// the caller cannot statically guarantee valid indices.
class BitVector {
public:
  /// Constructs a bit vector over an externally-owned word array.
  ///
  /// @param data   Pointer to the backing u64 array. Must not be null.
  /// @param words  Number of u64 elements in the array.
  BitVector(u64 *buffer, usize words) noexcept
      : m_data(buffer), m_words(words) {}

  /// Sets the bit at position @p bit to 1.
  ///
  /// If @p bit exceeds the vector's capacity the call is a no-op.
  ///
  /// @param bit  Zero-indexed bit position.
  void set(usize bit) noexcept {
    ensure(bit);
    m_data[word(bit)] |= mask(bit);
  }

  /// Clears the bit at position @p bit to 0.
  ///
  /// If @p bit exceeds the vector's capacity the call is a no-op.
  ///
  /// @param bit  Zero-indexed bit position.
  void clear(usize bit) noexcept {
    if (bit / kBitsPerWord < m_words)
      m_data[word(bit)] &= ~mask(bit);
  }

  /// Returns @c true when the bit at position @p bit is 1.
  ///
  /// Returns @c false when @p bit exceeds the vector's capacity.
  ///
  /// @param bit  Zero-indexed bit position.
  /// @return     @c true if the bit is set, @c false if clear or
  ///             out of bounds.
  bool test(usize bit) const noexcept {
    if (bit / kBitsPerWord >= m_words) {
      return false;
    }
    return m_data[word(bit)] & mask(bit);
  }

  /// Zeros all bits in the vector.
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
