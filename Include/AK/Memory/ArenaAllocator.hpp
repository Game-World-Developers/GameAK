/// @file
/// @brief Bump-pointer arena allocator.
///
/// Provides a fast, deterministic allocator backed by an externally-owned
/// contiguous buffer. The arena does not own its backing memory — the caller
/// is responsible for providing and managing the underlying storage.
///
/// This design allows arenas to be backed by:
///   - Static arrays       (zero heap usage)
///   - mmap regions        (BMP Container integration)
///   - Parent allocators   (nested arena hierarchies)
///
/// Allocation is O(1): a single aligned bump of the internal offset.
/// Individual deallocation is not supported. Memory is reclaimed via
/// reset() or through checkpoint/restore pairs for scoped lifetimes.

#pragma once

#include <AK/Core/Macros.hpp>
#include <AK/Core/Optional.hpp>
#include <AK/Core/TypeTraits.hpp>
#include <AK/Core/Types.hpp>

#include <AK/Platform/ArchDetect.hpp>

namespace GameAK {

/// @brief Bump-pointer arena allocator over an external buffer.
///
/// Not copyable or movable — arenas are fixed resources tied to a
/// specific memory region. Pass by pointer or reference.

class GAMEAK_ALIGN_CACHE ArenaAllocator {
public:
  /// Constructs an arena over an externally-owned buffer.
  ///
  /// @param buffer    Pointer to the backing memory. Must not be null.
  /// @param capacity  Size of the backing buffer in bytes.
  ArenaAllocator(void *buffer, usize capacity) noexcept;

  // Arena is a fixed resource — copying or moving is not permitted.
  ArenaAllocator(const ArenaAllocator &) = delete;
  ArenaAllocator &operator=(const ArenaAllocator &) = delete;
  ArenaAllocator(ArenaAllocator &&) = delete;
  ArenaAllocator &operator=(ArenaAllocator &&) = delete;

  ~ArenaAllocator() = default;

  /// Allocates @p size bytes aligned to @p alignment.
  ///
  /// Alignment must be a power of two. Returns @c nullptr if the
  /// arena has insufficient remaining space.
  ///
  /// @param size       Number of bytes to allocate.
  /// @param alignment  Required alignment in bytes (must be power of 2).
  /// @return           Pointer to allocated region, or @c nullptr.
  [[nodiscard]] void *allocate(usize size, usize alignment) noexcept;

  /// Allocates storage for @p count objects of type @c T.
  ///
  /// Uses @c alignof(T) automatically. Does not construct objects —
  /// caller is responsible for placement-new where required.
  ///
  /// @tparam T      Type to allocate storage for.
  /// @param count  Number of T objects (default: 1).
  /// @return        Pointer to raw storage, or @c nullptr.
  template <typename T>
  [[nodiscard]] GAMEAK_FORCE_INLINE T *allocate(usize count = 1) noexcept {
    if (count > std::numeric_limits<usize>::max() / sizeof(T)) {
      return nullptr;
    }
    return static_cast<T *>(allocate(sizeof(T) * count, alignof(T)));
  }

  /// Saves the current allocation offset as a checkpoint.
  ///
  /// Use with restore() to implement scoped/temporary allocations
  /// (e.g. per-frame scratch arenas in the Cardinal loop).
  ///
  /// @return Opaque offset value to pass to restore().
  [[nodiscard]] usize save() const noexcept;

  /// Restores the arena to a previously saved checkpoint.
  ///
  /// All allocations made after save() are invalidated.
  /// Does not zero memory — data remains in the buffer but is
  /// considered dead and will be overwritten by future allocations.
  ///
  /// @param checkpoint  Value returned by a prior call to save().
  void restore(usize checkpoint) noexcept;

  /// Resets the arena to its initial empty state.
  ///
  /// Equivalent to restore(0). All prior allocations are invalidated.
  void reset() noexcept;

  /// @return Bytes currently allocated.
  [[nodiscard]] usize used() const noexcept;

  /// @return Bytes available for future allocations.
  [[nodiscard]] usize remaining() const noexcept;

  /// @return Total capacity of the backing buffer in bytes.
  [[nodiscard]] usize capacity() const noexcept;

  /// @return @c true if @p ptr points into this arena's buffer.
  [[nodiscard]] bool owns(const void *ptr) const noexcept;

  /// @return @c true if an allocation of @p size bytes at @p alignment
  ///         would succeed given the current offset.
  [[nodiscard]] bool can_alloc(usize size, usize alignment) const noexcept;

private:
  u8 *m_buffer;
  usize m_capacity;
  usize m_offset;

  [[nodiscard]] Optional<usize> alloc_impl(usize size,
                                           usize alignment) const noexcept;
};

} // namespace GameAK
