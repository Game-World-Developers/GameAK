#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Bits {

template <typename T> [[nodiscard]] constexpr T bit(T n) noexcept {
  return T(1) << n;
}

template <typename T> [[nodiscard]] constexpr T mask(T n) noexcept {
  return (T(1) << n) - 1;
}

template <typename T>
[[nodiscard]] constexpr bool is_power_of_two(T value) noexcept {
  return value && !(value & (value - 1));
}

template <typename T>
[[nodiscard]] constexpr T align_up(T value, T alignment) noexcept {
  return (value + alignment - 1) & ~(alignment - 1);
}

template <typename T> [[nodiscard]] constexpr T popcount(T value) noexcept {
  T count = 0;
  while (value) {
    value &= (value - 1);
    ++count;
  }

  return count;
}
} // namespace GameAK::Bits
