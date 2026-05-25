#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Macros.hpp>
#include <AK/Memory/MemoryDebug.hpp>
#include <AK/Memory/PoolAllocator.hpp>

namespace GameAK {

PoolAllocator::PoolAllocator(void *buffer, usize capacity, usize block_size,
                             usize block_alignment) noexcept
    : m_buffer{static_cast<u8 *>(buffer)}, m_block_size{block_size},
      m_block_count{0U}, m_free_count{0U}, m_free_head{nullptr} {

  Memory::Debug::validate_alignment(block_alignment);
  if (block_alignment < alignof(void *)) {
    GAMEAK_DEBUG_BREAK();
    return;
  }

  const bool size_valid =
      (block_size >= sizeof(void *)) && ((block_size % block_alignment) == 0U);

  if (GAMEAK_UNLIKELY(!size_valid)) {
    GAMEAK_DEBUG_BREAK();
    return;
  }

  m_block_count = capacity / block_size;

  if (GAMEAK_UNLIKELY(m_block_count == 0U)) {
    GAMEAK_DEBUG_BREAK();
    return;
  }

  m_free_count = m_block_count;
  init_free_list();
}

void PoolAllocator::init_free_list() noexcept {
  for (usize i = 0U; i < m_block_count - 1U; ++i) {
    void *current = m_buffer + (i * m_block_size);
    void *next = m_buffer + ((i + 1U) * m_block_size);
    *reinterpret_cast<void **>(current) = next;
  }

  void *last = m_buffer + ((m_block_count - 1U) * m_block_size);
  *reinterpret_cast<void **>(last) = nullptr;

  m_free_head = m_buffer;
}

bool PoolAllocator::is_valid_block(const void *ptr) const noexcept {
  const auto *const own_ptr = static_cast<const u8 *>(ptr);

  if (own_ptr < m_buffer ||
      own_ptr >= m_buffer + (m_block_count * m_block_size)) {
    return false;
  }

  const usize offset = static_cast<usize>(own_ptr - m_buffer);
  return (offset & (m_block_size - 1)) == 0U;
}

void *PoolAllocator::acquire() noexcept {
  if (GAMEAK_UNLIKELY(m_free_head == nullptr)) {
    return nullptr;
  }

  void *block = m_free_head;
  m_free_head = *reinterpret_cast<void **>(m_free_head);
  --m_free_count;

  return block;
}

void PoolAllocator::release(void *ptr) noexcept {
  if (GAMEAK_UNLIKELY(ptr == nullptr)) {
    return;
  }

  if (GAMEAK_UNLIKELY(!is_valid_block(ptr))) {
    GAMEAK_DEBUG_BREAK();
    return;
  }

#if defined(GAMEAK_DEBUG_VALIDATE)
  {
    void *curr = m_free_head;
    while (curr != nullptr) {
      if (GAMEAK_UNLIKELY(curr == ptr)) {
        return; // double-release: already in free list, no-op
      }
      curr = *static_cast<void **>(curr);
    }
  }
#endif

  *static_cast<void **>(ptr) = m_free_head;
  m_free_head = ptr;
  ++m_free_count;
}

void PoolAllocator::reset() noexcept {
  if (m_block_count == 0U) return;
  m_free_count = m_block_count;
  init_free_list();
}

bool PoolAllocator::owns(const void *ptr) const noexcept {
  return is_valid_block(ptr);
}

usize PoolAllocator::used_count() const noexcept {
  return m_block_count - m_free_count;
}

usize PoolAllocator::free_count() const noexcept { return m_free_count; }

usize PoolAllocator::block_count() const noexcept { return m_block_count; }

usize PoolAllocator::block_size() const noexcept { return m_block_size; }

bool PoolAllocator::is_empty() const noexcept {
  return m_free_count == m_block_count;
}

bool PoolAllocator::is_full() const noexcept { return m_free_count == 0U; }

} // namespace GameAK
