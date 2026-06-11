#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Macros.hpp>
#include <AK/Memory/ArenaAllocator.hpp>
#include <AK/Memory/MemoryDebug.hpp>

namespace GameAK {

ArenaAllocator::ArenaAllocator(void *buffer, usize capacity) noexcept
    : m_buffer{static_cast<u8 *>(buffer)}, m_capacity{capacity}, m_offset{0U} {}

usize ArenaAllocator::save() const noexcept { return m_offset; }

void ArenaAllocator::restore(usize checkpoint) noexcept {
  Memory::Debug::validate_offset(checkpoint, m_capacity);
  if (GAMEAK_UNLIKELY(checkpoint > m_offset)) {
    GAMEAK_DEBUG_BREAK();
    return;
  }
  m_offset = checkpoint;
}

void *ArenaAllocator::allocate(usize size, usize alignment) noexcept {
  Memory::Debug::validate_alignment(alignment);
  if (GAMEAK_UNLIKELY(!Bits::is_power_of_two(alignment))) {
    return nullptr;
  }

  Memory::Debug::debug_assert([this] {
    auto *self = reinterpret_cast<const u8 *>(this);
    return self < m_buffer || self >= m_buffer + m_capacity;
  });

  auto result = alloc_impl(size, alignment);
  if (GAMEAK_UNLIKELY(!result)) {
    return nullptr;
  }

  const usize aligned_offset = *result;
  m_offset = aligned_offset + size;
  return m_buffer + aligned_offset;
}

bool ArenaAllocator::can_alloc(usize size, usize alignment) const noexcept {
  if (!Bits::is_power_of_two(alignment))
    return false;
  return alloc_impl(size, alignment).has_value();
}

void ArenaAllocator::reset() noexcept { restore(0U); }

usize ArenaAllocator::used() const noexcept { return m_offset; }
usize ArenaAllocator::remaining() const noexcept {
  return m_capacity - m_offset;
}
usize ArenaAllocator::capacity() const noexcept { return m_capacity; }

bool ArenaAllocator::owns(const void *ptr) const noexcept {
  const auto *const own_ptr = static_cast<const u8 *>(ptr);
  return own_ptr >= m_buffer && own_ptr < m_buffer + m_capacity;
}

Optional<usize> ArenaAllocator::alloc_impl(usize size,
                                           usize alignment) const noexcept {
  const usize aligned_offset = Bits::align_up(m_offset, alignment);

  Memory::Debug::validate_offset(aligned_offset, m_capacity);
  Memory::Debug::validate_alignment(alignment);

  if (size > m_capacity - aligned_offset) {
    return Nullopt;
  }
  return aligned_offset;
}

} // namespace GameAK
