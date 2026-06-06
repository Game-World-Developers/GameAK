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
/// When @p n equals or exceeds the bit width of T, all bits are set.
///
/// @tparam T  Unsigned integer type of the result.
/// @param n  Number of low bits to set.
/// @return    (T(1) << n) - 1, or ~T(0) when n >= bit-width of T.
template <typename T> [[nodiscard]] constexpr T mask(T n) noexcept {
  constexpr T kBits = T(sizeof(T) * 8);
  return n >= kBits ? T(~0) : (T(1) << n) - 1;
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
/// Uses __builtin_popcountll at runtime when available, falls back to a
/// constexpr-friendly Kernighan loop in constant-evaluation contexts.
template <typename T> [[nodiscard]] constexpr T popcount(T value) noexcept {
#if defined(__GNUC__) || defined(__clang__)
  if (!__builtin_is_constant_evaluated()) {
    if constexpr (sizeof(T) <= sizeof(unsigned int))
      return T(__builtin_popcount((unsigned int)value));
    else
      return T(__builtin_popcountll((unsigned long long)value));
  }
#endif
  T count = 0;
  while (value) {
    value &= (value - 1);
    ++count;
  }
  return count;
}
/// Counts the number of leading zero bits in @p value.
///
/// If @p value is zero, returns the bit-width of T.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value.
/// @return        Number of leading zero bits.
template <typename T> [[nodiscard]] constexpr T clz(T value) noexcept {
  constexpr usize kBits = sizeof(T) * 8;
  if (value == 0)
    return T(kBits);
  if constexpr (kBits <= 32) {
    return T(__builtin_clz((u32)(value)) - (32 - kBits));
  } else {
    return T(__builtin_clzll((u64)(value)));
  }
}

/// Counts the number of trailing zero bits in @p value.
///
/// If @p value is zero, returns the bit-width of T.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value.
/// @return        Number of trailing zero bits.
template <typename T> [[nodiscard]] constexpr T ctz(T value) noexcept {
  constexpr usize kBits = sizeof(T) * 8;
  if (value == 0)
    return T(kBits);
  if constexpr (kBits <= 32) {
    return T(__builtin_ctz((u32)(value)));
  } else {
    return T(__builtin_ctzll((u64)(value)));
  }
}

/// Rotates @p value left by @p shift bits.
///
/// @tparam T  Unsigned integer type.
/// @param value   Value to rotate.
/// @param shift   Number of bits to rotate left.
/// @return        Rotated value.
template <typename T>
[[nodiscard]] constexpr T rotl(T value, int shift) noexcept {
  constexpr int kBits = int(sizeof(T) * 8);
  shift &= (kBits - 1);
  if (shift == 0)
    return value;
  return T((value << shift) | (value >> (kBits - shift)));
}

/// Rotates @p value right by @p shift bits.
///
/// @tparam T  Unsigned integer type.
/// @param value   Value to rotate.
/// @param shift   Number of bits to rotate right.
/// @return        Rotated value.
template <typename T>
[[nodiscard]] constexpr T rotr(T value, int shift) noexcept {
  constexpr int kBits = int(sizeof(T) * 8);
  shift &= (kBits - 1);
  if (shift == 0)
    return value;
  return T((value >> shift) | (value << (kBits - shift)));
}

/// Floor of the base-2 logarithm of @p value.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value. Behaviour is undefined for value == 0.
/// @return       floor(log2(value)).
template <typename T> [[nodiscard]] constexpr T log2_floor(T value) noexcept {
  return T((sizeof(T) * 8 - 1) - clz(value));
}

/// Minimum number of bits required to represent @p value.
///
/// Returns 0 when @p value is zero.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value.
/// @return       bit-width of value.
template <typename T> [[nodiscard]] constexpr T bit_width(T value) noexcept {
  return T(sizeof(T) * 8 - clz(value));
}

/// Smallest power of two that is >= @p value.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value.
/// @return       ceil(log2(value)) as a power of two.
template <typename T> [[nodiscard]] constexpr T bit_ceil(T value) noexcept {
  if (value == 0)
    return T(1);
  return T(1) << bit_width(T(value - 1));
}

/// Largest power of two that is <= @p value.
///
/// Returns 0 when @p value is zero.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value.
/// @return       floor(log2(value)) as a power of two.
template <typename T> [[nodiscard]] constexpr T bit_floor(T value) noexcept {
  if (value == 0)
    return T(0);
  return T(1) << (bit_width(value) - 1);
}

/// Extracts a range of bits from @p value.
///
/// Returns the @p count bits starting at bit position @p offset in the
/// low positions of the result.
///
/// @tparam T  Unsigned integer type.
/// @param value   Source value.
/// @param offset  Bit offset to start extraction (0 = LSB).
/// @param count   Number of bits to extract.
/// @return        Extracted bit range in low bits.
template <typename T>
[[nodiscard]] constexpr T extract_bits(T value, usize offset,
                                       usize count) noexcept {
  return (value >> offset) & mask(T(count));
}

/// Inserts @p bits into @p value at the given offset.
///
/// The lowest @p count bits of @p bits are placed at bit position @p offset.
///
/// @tparam T  Unsigned integer type.
/// @param value   Target value that receives the bits.
/// @param bits    Source value containing the bits to insert.
/// @param offset  Bit offset for insertion.
/// @param count   Number of bits to insert.
/// @return        Value with the bit-range replaced.
template <typename T>
[[nodiscard]] constexpr T insert_bits(T value, T bits, usize offset,
                                      usize count) noexcept {
  T m = mask(T(count));
  return (value & ~(m << offset)) | ((bits & m) << offset);
}

/// Reverses the order of all bits in @p value.
///
/// @tparam T  Unsigned integer type.
/// @param value  Input value.
/// @return       Bit-reversed value.
template <typename T> [[nodiscard]] constexpr T reverse_bits(T value) noexcept {
  constexpr usize kBits = sizeof(T) * 8;
  T result = T(0);
  for (usize i = 0; i < kBits; ++i) {
    result = T((result << 1) | (value & 1));
    value >>= 1;
  }
  return result;
}
} // namespace GameAK::Bits
