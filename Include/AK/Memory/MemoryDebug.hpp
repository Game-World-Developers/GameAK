#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Macros.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::Memory::Debug {

#ifdef GAMEAK_DEBUG_VALIDATE

inline void validate_alignment(usize alignment) noexcept {
  if (!Bits::is_power_of_two(alignment)) {
    GAMEAK_DEBUG_BREAK();
  }
}

inline void validate_offset(usize offset, usize capacity) noexcept {
  if (offset > capacity) {
    GAMEAK_DEBUG_BREAK();
  }
}

inline void validate_pointer_in_range(const void* ptr,
                                      const u8* buffer,
                                      usize capacity) noexcept {
  const auto* byte_ptr = static_cast<const u8*>(ptr);
  if (byte_ptr < buffer || byte_ptr >= buffer + capacity) {
    GAMEAK_DEBUG_BREAK();
  }
}

#else

inline void validate_alignment(usize) noexcept {}
inline void validate_offset(usize, usize) noexcept {}
inline void validate_pointer_in_range(const void*, const u8*, usize) noexcept {}

#endif

} // namespace GameAK::Memory::Debug
