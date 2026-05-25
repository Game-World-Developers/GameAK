#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

template <typename Enum> class BitMask {
public:
  using UnderlyingType = u64;

  constexpr BitMask() : m_value(0) {}
  explicit constexpr BitMask(Enum select_enum) noexcept : m_value(to_bit(select_enum)) {}

  constexpr void set(Enum select_enum) noexcept {
    m_value |= to_bit(select_enum);
  }

  constexpr void clear(Enum select_enum) noexcept {
    m_value &= ~to_bit(select_enum);
  }

  constexpr bool has(Enum select_enum) const noexcept {
    return (m_value & to_bit(select_enum)) != 0;
  }

  constexpr UnderlyingType raw() const noexcept { return m_value; }

private:
  UnderlyingType m_value;

  static constexpr UnderlyingType to_bit(Enum select_enum) noexcept {
    return UnderlyingType(1) << static_cast<UnderlyingType>(select_enum);
  };
};
} // namespace GameAK::Bits
