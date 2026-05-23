/// @file
/// @brief A lightweight optional value type for GameAK.
///
/// Represents a value that may or may not be present, without dynamic
/// allocation or exceptions. Storage is stack-local aligned raw bytes.
///
/// Destruction is a no-op for trivially destructible types — the compiler
/// eliminates the destructor entirely in those cases.

#pragma once

#include <AK/Core/Macros.hpp>
#include <AK/Core/TypeTraits.hpp>
#include <AK/Core/Types.hpp>
#include <new>

namespace GameAK {
struct NulloptT {
  explicit constexpr NulloptT() = default;
};

inline constexpr NulloptT Nullopt{};

template <typename T> class Optional {
public:
  constexpr Optional() noexcept : m_has_value{false} {}
  constexpr Optional(NulloptT) noexcept : m_has_value{false} {}

  Optional(const T &value) noexcept(noexcept(T(value))) : m_has_value{true} {
    new (ptr()) T(value);
  }

  Optional(T &&value) noexcept(noexcept(T(Move(value)))) : m_has_value{true} {
    new (ptr()) T(Move(value));
  }

  ~Optional() noexcept {
    if constexpr (!IsTriviallyDestructible<T>) {
      if (m_has_value) {
        ptr()->~T();
      }
    }
  }

  Optional(const Optional &other) noexcept(noexcept(T(*other.ptr())))
      : m_has_value{other.m_has_value} {
    if (m_has_value) {
      new (ptr()) T(*other.ptr());
    }
  }

  Optional(Optional &&other) noexcept(noexcept(T(Move(*other.ptr()))))
      : m_has_value{other.m_has_value} {
    if (m_has_value) {
      new (ptr()) T(Move(*other.ptr()));
      other.reset();
    }
  }

  Optional &
  operator=(const Optional &other) noexcept(noexcept(T(*other.ptr()))) {
    if (this != &other) {
      reset();
      if (other.m_has_value) {
        new (ptr()) T(*other.ptr());
        m_has_value = true;
      }
    }
    return *this;
  }

  Optional &
  operator=(Optional &&other) noexcept(noexcept(T(Move(*other.ptr())))) {
    if (this != &other) {
      reset();
      if (other.m_has_value) {
        new (ptr()) T(Move(*other.ptr()));
        m_has_value = true;
        other.reset();
      }
    }
    return *this;
  }

  /// @return true if a value is present.
  [[nodiscard]] GAMEAK_FORCE_INLINE bool has_value() const noexcept {
    return m_has_value;
  }

  /// Explicit bool conversion — allows `if (optional)` syntax.
  [[nodiscard]] GAMEAK_FORCE_INLINE explicit operator bool() const noexcept {
    return m_has_value;
  }

  /// Unchecked access — undefined behaviour if empty.
  /// Use only when has_value() is guaranteed by the call site.
  [[nodiscard]] GAMEAK_FORCE_INLINE T &operator*() noexcept { return *ptr(); }

  [[nodiscard]] GAMEAK_FORCE_INLINE const T &operator*() const noexcept {
    return *ptr();
  }

  /// Checked access — triggers AK_DEBUG_BREAK if empty.
  [[nodiscard]] T &value() noexcept {
    if (GAMEAK_UNLIKELY(!m_has_value)) {
      GAMEAK_DEBUG_BREAK();
    }
    return *ptr();
  }

  [[nodiscard]] const T &value() const noexcept {
    if (GAMEAK_UNLIKELY(!m_has_value)) {
      GAMEAK_DEBUG_BREAK();
    }
    return *ptr();
  }

  /// @return The contained value, or @p fallback if empty.
  [[nodiscard]] T value_or(T fallback) const noexcept {
    return m_has_value ? *ptr() : Move(fallback);
  }

  void reset() noexcept {
    if constexpr (!IsTriviallyDestructible<T>) {
      if (m_has_value) {
        ptr()->~T();
      }
    }
    m_has_value = false;
  }

private:
  alignas(T) u8 m_storage[sizeof(T)];
  bool m_has_value;

  GAMEAK_FORCE_INLINE T *ptr() noexcept {
    return reinterpret_cast<T *>(m_storage);
  }

  GAMEAK_FORCE_INLINE const T *ptr() const noexcept {
    return reinterpret_cast<const T *>(m_storage);
  }
};

} // namespace GameAK
