#pragma once

#include <type_traits>

namespace AK {

template <typename T> using RemoveRef = std::remove_reference<T>;
template <typename T> using RemoveConst = std::remove_const<T>;
template <typename T> using RemoveCV = std::remove_cv<T>;
template <typename T>
using RemoveCVRef = std::remove_cv_t<typename std::remove_reference_t<T>>;
template <typename T> using AddConst = std::add_const<T>;
template <typename T> using AddLValueRef = std::add_lvalue_reference<T>;
template <typename T> using AddRValueRef = std::add_rvalue_reference<T>;

template <typename T> inline constexpr bool IsIntegral = std::is_integral_v<T>;

template <typename T>
inline constexpr bool IsFloatingPoint = std::is_floating_point_v<T>;

template <typename T> inline constexpr bool IsPointer = std::is_pointer_v<T>;

template <typename T>
inline constexpr bool IsReference = std::is_reference_v<T>;

template <typename T>
inline constexpr bool IsTriviallyCopyable = std::is_trivially_copyable_v<T>;

template <typename T> using Decay = std::decay<T>;

template <bool B, typename T = void> using EnableIf = std::enable_if_t<B, T>;

template <typename T> constexpr RemoveRef<T> &&Move(T &&t) noexcept {
  return static_cast<RemoveRef<T> &&>(t);
}

template <typename T> constexpr T &&Forward(RemoveRef<T> &t) noexcept {
  return static_cast<T &&>(t);
}

template <typename T> constexpr T &&Forward(RemoveRef<T> &&t) noexcept {
  return static_cast<T &&>(t);
}
} // namespace AK
