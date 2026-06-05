/// @file
/// @brief Compile-time bit-field accessor.
///
/// Extracts or inserts a fixed-width bit range from/into an unsigned
/// integer at compile time. All parameters (type, offset, width) are
/// template arguments validated at compile time.

#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

/// Compile-time bit-field accessor over unsigned integer @p T.
///
/// @tparam T       Unsigned integer type of the container word.
/// @tparam Offset  Bit position of the field (0 = LSB).
/// @tparam Width   Number of bits in the field (must be > 0).
template <typename T, usize Offset, usize Width>
class BitField {
  static_assert(Width > 0, "BitField width must be > 0");
  static_assert(Offset + Width <= sizeof(T) * 8,
                "BitField exceeds the bit-width of T");

public:
  using Type = T;

  static constexpr T kMask = T(mask(T(Width)) << Offset);

  /// Extracts the field bits from @p word, returned in low positions.
  [[nodiscard]] static constexpr T extract(T word) noexcept {
    return (word >> Offset) & mask(T(Width));
  }

  /// Returns @p word with the field replaced by the low @p Width bits
  /// of @p val.
  [[nodiscard]] static constexpr T insert(T word, T val) noexcept {
    return (word & ~kMask) | ((val & mask(T(Width))) << Offset);
  }
};

} // namespace GameAK::Bits
