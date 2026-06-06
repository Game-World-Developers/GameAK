/// @file
/// @brief Debug-only validation helpers for memory operations.
///
/// Provides alignment, offset, and pointer-range validation. All functions
/// compile to no-ops in release (non-debug) builds.

#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Macros.hpp>
#include <AK/Core/Types.hpp>
#include <concepts>

namespace GameAK::Memory::Debug {

/// Compile-time flag indicating whether debug validation is active.
template <typename = void> struct DebugTraits {
  static constexpr bool enabled =
#ifdef GAMEAK_DEBUG_VALIDATE
      true
#else
      false
#endif
      ;
};

/// Convenience variable template: @c true when debug validation is active.
template <typename T = void>
inline constexpr bool debug_enabled_v = DebugTraits<T>::enabled;

/// Concept: any callable that returns @c bool without side effects.
template <typename F>
concept ValidationPredicate = requires(F &&f) {
  { f() } -> std::same_as<bool>;
};

/// If debug validation is enabled, evaluates @p predicate and triggers
/// @c GAMEAK_DEBUG_BREAK() when the predicate returns @c false.
/// Compiles to a no-op in release builds.
template <ValidationPredicate P>
inline void debug_assert(P &&predicate) noexcept {
  if constexpr (debug_enabled_v<>) {
    if (!predicate()) {
      GAMEAK_DEBUG_BREAK();
    }
  }
}

/// Asserts that @p alignment is a power of two.
inline void validate_alignment(usize alignment) noexcept {
  debug_assert([alignment] { return Bits::is_power_of_two(alignment); });
}

/// Asserts that @p offset does not exceed @p capacity.
inline void validate_offset(usize offset, usize capacity) noexcept {
  debug_assert([offset, capacity] { return offset <= capacity; });
}

/// Asserts that @p ptr points within [buffer, buffer + capacity).
inline void validate_pointer_in_range(const void *ptr, const u8 *buffer,
                                      usize capacity) noexcept {
  debug_assert([ptr, buffer, capacity] {
    const auto *byte_ptr = static_cast<const u8 *>(ptr);
    return byte_ptr >= buffer && byte_ptr < buffer + capacity;
  });
}

} // namespace GameAK::Memory::Debug
