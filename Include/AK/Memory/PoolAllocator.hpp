/// @file
/// @brief Fixed-size block pool allocator.
///
/// Manages a contiguous buffer divided into fixed-size blocks, providing
/// O(1) acquire and release via an intrusive free list stored within the
/// free blocks themselves — zero metadata overhead per block.
///
/// Like ArenaAllocator, the pool does NOT own its backing memory.
/// The caller is responsible for providing and managing the buffer.
///
/// Constraints:
///   - block_size must be >= sizeof(void*) to store the free list pointer.
///   - block_size must be a power of two.
///   - block_alignment must be a power of two and >= alignof(void*).
///   - block_size must be a multiple of block_alignment.

#pragma once

#include <AK/Core/Macros.hpp>
#include <AK/Core/TypeTraits.hpp>
#include <AK/Core/Types.hpp>
#include <AK/Platform/ArchDetect.hpp>

namespace GameAK {

class GAMEAK_ALIGN_CACHE PoolAllocator {
public:
  /// Constructs a pool over an externally-owned buffer.
  ///
  /// @param buffer           Backing memory. Must not be null.
  /// @param capacity         Total size of the buffer in bytes.
  /// @param block_size       Size of each individual block in bytes
  ///                         (must be power of two, >= sizeof(void*)).
  /// @param block_alignment  Alignment of each block (power of two,
  ///                         >= alignof(void*)).
  PoolAllocator(void *buffer, usize capacity, usize block_size,
                usize block_alignment) noexcept;

  PoolAllocator(const PoolAllocator &) = delete;
  PoolAllocator &operator=(const PoolAllocator &) = delete;
  PoolAllocator(PoolAllocator &&) = delete;
  PoolAllocator &operator=(PoolAllocator &&) = delete;

  ~PoolAllocator() = default;

  /// Acquires a free block from the pool.
  ///
  /// Does not construct — caller is responsible for placement-new
  /// when the block holds a non-trivial type.
  ///
  /// @return Pointer to a free block, or @c nullptr if the pool is full.
  [[nodiscard]] void *acquire() noexcept;

  /// Typed acquire — convenience wrapper over acquire().
  ///
  /// @tparam T  Type whose storage is requested. sizeof(T) and alignof(T)
  ///            must be <= block_size and block_alignment respectively.
  template <typename T>
  [[nodiscard]] GAMEAK_FORCE_INLINE T *acquire() noexcept {
    return static_cast<T *>(acquire());
  }

  /// Returns a block to the pool.
  ///
  /// @p ptr must have been obtained from this pool's acquire().
  /// Returning a foreign pointer or an already-released block is a
  /// programming error and triggers GAMEAK_DEBUG_BREAK in debug builds.
  ///
  /// Does not destroy — caller is responsible for explicitly calling
  /// the destructor before release() for non-trivial types.
  void release(void *ptr) noexcept;

  /// Typed release — convenience wrapper over release().
  ///
  /// Calls ptr->~T() before returning the block to the pool.
  /// Only call this when the object was fully constructed.
  template <typename T> GAMEAK_FORCE_INLINE void release(T *ptr) noexcept {
    if constexpr (!IsTriviallyDestructible<T>) {
      if (ptr != nullptr) {
        ptr->~T();
      }
    }
    release(static_cast<void *>(ptr));
  }

  /// Re-initialises the free list, making all blocks available again.
  ///
  /// All prior acquisitions are invalidated. Does not zero memory.
  void reset() noexcept;

  /// @return true if @p ptr is a block-aligned address within this pool.
  [[nodiscard]] bool owns(const void *ptr) const noexcept;

  /// @return Number of blocks currently acquired (in use).
  [[nodiscard]] usize used_count() const noexcept;

  /// @return Number of blocks currently free.
  [[nodiscard]] usize free_count() const noexcept;

  /// @return Total number of blocks in the pool.
  [[nodiscard]] usize block_count() const noexcept;

  /// @return Size of each block in bytes.
  [[nodiscard]] usize block_size() const noexcept;

  /// @return true if all blocks are free.
  [[nodiscard]] bool is_empty() const noexcept;

  /// @return true if no blocks are free.
  [[nodiscard]] bool is_full() const noexcept;

private:
  u8 *m_buffer;
  usize m_block_size;
  usize m_block_count;
  usize m_free_count;
  void *m_free_head;

  /// Builds the intrusive free list over the backing buffer.
  /// Called by both the constructor and reset().
  void init_free_list() noexcept;

  /// @return true if @p ptr is block-aligned within the buffer.
  [[nodiscard]] bool is_valid_block(const void *ptr) const noexcept;
};
} // namespace GameAK
