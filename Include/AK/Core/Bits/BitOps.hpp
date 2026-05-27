/// @file
/// @brief Lightweight bit-level operations: bit generation, masking,
///        alignment, power-of-two tests, and population count.
///
/// All functions are constexpr templates operating on unsigned integer types.
/// They are designed for zero-overhead composition with the rest of the Bits
/// module (BitMap, BitMask, BitPack, BitVector) as well as standalone use in
/// allocators and other performance-sensitive code paths.
///
/// Functions are constexpr and safe for use in static_assert and template
/// metaprogramming contexts.

#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

/// Returns 1 << n — a single-bit value with bit @p n set.
///
/// @tparam T  Unsigned integer type of the result.
/// @param n  Bit position (0-indexed). Behaviour is undefined if n
///           exceeds the width of T.
/// @return    T(1) << n.
template <typename T> [[nodiscard]] constexpr T bit(T n) noexcept {
  return T(1) << n;
}

/// Returns (1 << n) - 1 — a mask with the lowest @p n bits set.
///
/// @tparam T  Unsigned integer type of the result.
/// @param n  Number of low bits to set. Behaviour is undefined if n
///           exceeds the width of T.
/// @return    (T(1) << n) - 1.
template <typename T> [[nodiscard]] constexpr T mask(T n) noexcept {
  return (T(1) << n) - 1;
}

/// Returns @c true when @p value is a power of two (or zero).
///
/// @tparam T  Integer type (signed or unsigned).
/// @param value  Value to test.
/// @return        @c false when value is zero; @c true when value has
///                exactly one bit set.
template <typename T>
[[nodiscard]] constexpr bool is_power_of_two(T value) noexcept {
  return value && !(value & (value - 1));
}

/// Rounds @p value up to the next multiple of @p alignment.
///
/// Alignment must be a power of two. The result is computed as
/// (value + alignment - 1) & ~(alignment - 1).
///
/// @tparam T  Unsigned integer type.
/// @param value      Value to align.
/// @param alignment  Power-of-two alignment boundary.
/// @return            Aligned value >= @p value.
template <typename T>
[[nodiscard]] constexpr T align_up(T value, T alignment) noexcept {
  return (value + alignment - 1) & ~(alignment - 1);
}

/// Counts the number of set (1) bits in @p value (population count).
///
/// Uses the Brian Kernighan algorithm: iterates once per set bit.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value.
/// @return        Number of bits set to 1.
template <typename T> [[nodiscard]] constexpr T popcount(T value) noexcept {
  T count = 0;
  while (value) {
    value &= (value - 1);
    ++count;
  }

  return count;
}
} // namespace GameAK::Bits
