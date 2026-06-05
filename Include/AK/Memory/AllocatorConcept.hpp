/// @file
/// @brief Allocator concepts and construction/destruction helpers.
///
/// Defines Allocator, ArenaAllocatorC, and PoolAllocatorC concepts,
/// plus generic construct_at / destroy_at utilities backed by any Allocator.

#pragma once

#include <AK/Core/TypeTraits.hpp>
#include <AK/Core/Types.hpp>

#include <concepts>
#include <new>

namespace GameAK::Memory {

template <typename A>
concept Allocator = requires(A& a, usize size, usize alignment) {
  { a.allocate(size, alignment) } -> std::same_as<void*>;
  { a.owns(static_cast<const void*>(nullptr)) } -> std::same_as<bool>;
};

template <typename A>
concept ArenaAllocatorC = Allocator<A> && requires(A& a) {
  { a.save() } -> std::same_as<usize>;
  { a.restore(usize{}) } -> std::same_as<void>;
  { a.reset() } -> std::same_as<void>;
};

template <typename A>
concept PoolAllocatorC = Allocator<A> && requires(A& a) {
  { a.release(static_cast<void*>(nullptr)) } -> std::same_as<void>;
  { a.reset() } -> std::same_as<void>;
  { a.free_count() } -> std::same_as<usize>;
  { a.used_count() } -> std::same_as<usize>;
};

/// Constructs an object of type T in storage obtained from allocator @p a.
/// Returns nullptr if allocation fails.
template <typename T, Allocator A, typename... Args>
[[nodiscard]] T* construct_at(A& alloc, Args&&... args) noexcept {
  auto* ptr = alloc.template allocate<T>();
  if (!ptr) {
    return nullptr;
  }
  return new (ptr) T(Forward<Args>(args)...);
}

/// Destroys an object constructed with construct_at.
/// No-op for trivially destructible types.
template <typename T>
void destroy_at(T* ptr) noexcept {
  if constexpr (!IsTriviallyDestructible<T>) {
    if (ptr) {
      ptr->~T();
    }
  }
}

} // namespace GameAK::Memory
