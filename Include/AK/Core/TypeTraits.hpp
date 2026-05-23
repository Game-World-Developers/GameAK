/// @file
/// @brief Compile-time type traits, metafunctions, and utility helpers for GameAK.
///
/// Provides a consistent GameAK-namespaced wrapper around @c &lt;type_traits&gt;
/// primitives, plus custom metafunctions such as @c IntegralConstant,
/// @c Conditional, @c EnableIf, and the @c Is* family of variable templates.
///
/// Also includes lightweight implementations of @c Move, @c Forward, and
/// @c Swap that mirror @c std::move, @c std::forward, and @c std::swap.

#pragma once

#include <AK/Platform/ArchDetect.hpp>
#include <type_traits>

namespace GameAK {

/// Compile-time integral constant wrapper.
///
/// @tparam T  The type of the constant value.
/// @tparam V  The constant value.
template <typename T, T V> struct IntegralConstant {
  static constexpr T value = V;
  using value_type = T;
  using type = IntegralConstant;
  constexpr operator value_type() const noexcept { return value; }
  constexpr value_type operator()() const noexcept { return value; }
};

/// @c IntegralConstant&lt;bool, true&gt;.
using TrueType = IntegralConstant<bool, true>;

/// @c IntegralConstant&lt;bool, false&gt;.
using FalseType = IntegralConstant<bool, false>;

/// Identity metafunction -- yields @c T unchanged.
///
/// @tparam T  The type to forward.
template <typename T> struct TypeIdentity {
  using type = T;
};

/// Conditional type selection (compile-time ternary).
///
/// @tparam B  If @c true, selects @c T; otherwise selects @c F.
/// @tparam T  Type when @c B is @c true.
/// @tparam F  Type when @c B is @c false.
template <bool B, typename T, typename F> struct Conditional {
  using type = T;
};

/// @copydoc Conditional
template <typename T, typename F> struct Conditional<false, T, F> {
  using type = F;
};

/// Alias for @c Conditional&lt;B, T, F&gt;::type.
template <bool B, typename T, typename F>
using ConditionalT = typename Conditional<B, T, F>::type;

/// Alias for @c std::enable_if_t.
///
/// @tparam B  The boolean condition.
/// @tparam T  The type to enable (defaults to @c void).
template <bool B, typename T = void> using EnableIf = std::enable_if_t<B, T>;

/// @{ @name Reference / CV qualifier manipulation

/// Removes reference qualifiers from @c T.
template <typename T> using RemoveRef = std::remove_reference_t<T>;

/// Removes @c const from @c T.
template <typename T> using RemoveConst = std::remove_const_t<T>;

/// Removes both @c const and @c volatile from @c T.
template <typename T> using RemoveCV = std::remove_cv_t<T>;

/// Removes cv-qualifiers and reference from @c T.
template <typename T>
using RemoveCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

/// Adds @c const to @c T.
template <typename T> using AddConst = std::add_const_t<T>;

/// Adds an lvalue reference to @c T.
template <typename T> using AddLValueRef = std::add_lvalue_reference_t<T>;

/// Adds an rvalue reference to @c T.
template <typename T> using AddRValueRef = std::add_rvalue_reference_t<T>;

/// Applies std::decay to @c T.
template <typename T> using Decay = std::decay_t<T>;

/// @}

/// @{ @name Type relation queries

/// @c true when @c A and @c B are the same type.
template <typename A, typename B>
inline constexpr bool IsSame = std::is_same_v<A, B>;

/// @c true when @c Base is a base class of @c Derived.
template <typename Base, typename Derived>
inline constexpr bool IsBaseOf = std::is_base_of_v<Base, Derived>;

/// @c true when @c From is implicitly convertible to @c To.
template <typename From, typename To>
inline constexpr bool IsConvertible = std::is_convertible_v<From, To>;

/// @}

/// @{ @name Type category queries

/// @c true when @c T is @c void.
template <typename T> inline constexpr bool IsVoid = std::is_void_v<T>;

/// @c true when @c T is @c std::nullptr_t.
template <typename T>
inline constexpr bool IsNullptr = std::is_null_pointer_v<T>;

/// @c true when @c T is an integral type.
template <typename T> inline constexpr bool IsIntegral = std::is_integral_v<T>;

/// @c true when @c T is a floating-point type.
template <typename T>
inline constexpr bool IsFloatingPoint = std::is_floating_point_v<T>;

/// @c true when @c T is an arithmetic type (integral or floating-point).
template <typename T>
inline constexpr bool IsArithmetic = std::is_arithmetic_v<T>;

/// @c true when @c T is a pointer type.
template <typename T> inline constexpr bool IsPointer = std::is_pointer_v<T>;

/// @c true when @c T is a reference type (lvalue or rvalue).
template <typename T>
inline constexpr bool IsReference = std::is_reference_v<T>;

/// @c true when @c T is an lvalue reference.
template <typename T>
inline constexpr bool IsLValueReference = std::is_lvalue_reference_v<T>;

/// @c true when @c T is an rvalue reference.
template <typename T>
inline constexpr bool IsRValueReference = std::is_rvalue_reference_v<T>;

/// @c true when @c T is an enum type.
template <typename T> inline constexpr bool IsEnum = std::is_enum_v<T>;

/// @c true when @c T is a union type.
template <typename T> inline constexpr bool IsUnion = std::is_union_v<T>;

/// @c true when @c T is a class (or struct) type.
template <typename T> inline constexpr bool IsClass = std::is_class_v<T>;

/// @c true when @c T is an array type.
template <typename T> inline constexpr bool IsArray = std::is_array_v<T>;

/// @c true when @c T is @c const-qualified.
template <typename T> inline constexpr bool IsConst = std::is_const_v<T>;

/// @}

/// @{ @name Trivially relocatable / copyable queries

/// @c true when @c T is trivially copyable.
template <typename T>
inline constexpr bool IsTriviallyCopyable = std::is_trivially_copyable_v<T>;

/// @c true when @c T is trivially destructible.
template <typename T>
inline constexpr bool IsTriviallyDestructible =
    std::is_trivially_destructible_v<T>;

/// @c true when @c T is trivially default-constructible.
template <typename T>
inline constexpr bool IsTriviallyDefaultConstructible =
    std::is_trivially_default_constructible_v<T>;

/// @c true when @c T is trivially copy-constructible.
template <typename T>
inline constexpr bool IsTriviallyCopyConstructible =
    std::is_trivially_copy_constructible_v<T>;

/// @c true when @c T is trivially move-constructible.
template <typename T>
inline constexpr bool IsTriviallyMoveConstructible =
    std::is_trivially_move_constructible_v<T>;

/// @c true when @c T is trivially relocatable (same as trivially copyable).
template <typename T>
inline constexpr bool IsTriviallyRelocatable = IsTriviallyCopyable<T>;

/// @}

/// @{ @name Constructibility / assignability queries

/// @c true when @c T is default-constructible.
template <typename T>
inline constexpr bool IsDefaultConstructible =
    std::is_default_constructible_v<T>;

/// @c true when @c T is move-constructible.
template <typename T>
inline constexpr bool IsMoveConstructible = std::is_move_constructible_v<T>;

/// @c true when @c T is copy-constructible.
template <typename T>
inline constexpr bool IsCopyConstructible = std::is_copy_constructible_v<T>;

/// @c true when @c T is move-assignable.
template <typename T>
inline constexpr bool IsMoveAssignable = std::is_move_assignable_v<T>;

/// @c true when @c T is copy-assignable.
template <typename T>
inline constexpr bool IsCopyAssignable = std::is_copy_assignable_v<T>;

/// @}

/// Converts a value to an xvalue (rvalue reference), enabling move semantics.
///
/// @tparam T  The type of the value.
/// @param the  The value to cast.
/// @return     An rvalue reference to @c the.
template <typename T> constexpr RemoveRef<T> &&Move(T &&the) noexcept {
  return static_cast<RemoveRef<T> &&>(the);
}

/// Forwards an lvalue or rvalue depending on @c T.
///
/// @tparam T  The original type of the argument (used to preserve value category).
/// @param the  The value to forward.
/// @return     An rvalue reference if @c T is an rvalue reference type; otherwise an lvalue reference.
template <typename T> constexpr T &&Forward(RemoveRef<T> &the) noexcept {
  return static_cast<T &&>(the);
}

/// Forwards an rvalue, with a compile-time check that @c T is not an lvalue reference.
///
/// @tparam T  The original type of the argument.
/// @param the  The rvalue to forward.
/// @return     An rvalue reference to @c the.
template <typename T> constexpr T &&Forward(RemoveRef<T> &&the) noexcept {
  static_assert(!IsLValueReference<T>,
                "GameAK: Forward — lvalue ref cannot be forwarded as rvalue; "
                "check the call site for accidental lvalue binding");
  return static_cast<T &&>(the);
}

/// Swaps two values using move semantics.
///
/// @tparam T  The type of the values (must be move-constructible and move-assignable).
/// @param a_val  First value.
/// @param b_val  Second value.
template <typename T>
constexpr void Swap(T &a_val, T &b_val) noexcept(noexcept(Move(a_val))) {
  T tmp = Move(a_val);
  a_val = Move(b_val);
  b_val = Move(tmp);
}

} // namespace GameAK
