/// @file
/// @brief Type-safe bitmask over an enum.
///
/// Wraps a u64 value and provides set, clear, and has operations keyed
/// by enum values. Each enum value corresponds to a single bit position
/// determined by its underlying integer representation.
///
/// This is a convenience wrapper over a raw u64 intended for flag
/// combinations where type safety and readability are desired.

#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

/// Type-safe bitmask over an enum.
///
/// @tparam Enum  Enum type whose values name individual flag bits.
///               The underlying integer must fit within u64.
template <typename Enum> class BitMask {
public:
  using UnderlyingType = u64;

  /// Constructs an empty mask with no flags set.
  constexpr BitMask() : m_value(0) {}

  /// Constructs a mask with a single flag set.
  ///
  /// @param select_enum  Enum value naming the initial flag.
  explicit constexpr BitMask(Enum select_enum) noexcept : m_value(to_bit(select_enum)) {}

  /// Sets the flag corresponding to @p select_enum.
  ///
  /// @param select_enum  Enum value naming the flag to set.
  constexpr void set(Enum select_enum) noexcept {
    m_value |= to_bit(select_enum);
  }

  /// Clears the flag corresponding to @p select_enum.
  ///
  /// @param select_enum  Enum value naming the flag to clear.
  constexpr void clear(Enum select_enum) noexcept {
    m_value &= ~to_bit(select_enum);
  }

  /// Returns @c true when the flag @p select_enum is set.
  ///
  /// @param select_enum  Enum value naming the flag to test.
  /// @return             @c true if the flag is set, @c false otherwise.
  constexpr bool has(Enum select_enum) const noexcept {
    return (m_value & to_bit(select_enum)) != 0;
  }

  /// @return The raw u64 bitmask value.
  constexpr UnderlyingType raw() const noexcept { return m_value; }

private:
  UnderlyingType m_value;

  static constexpr UnderlyingType to_bit(Enum select_enum) noexcept {
    return UnderlyingType(1) << static_cast<UnderlyingType>(select_enum);
  };
};
} // namespace GameAK::Bits
