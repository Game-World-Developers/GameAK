#pragma once

#include <AK/Backend/Backend.hpp>
#include <AK/Core/Bits/BitOps.hpp>

#include <cstring>

namespace GameAK::Backend {

namespace Detail {

inline void* mem_copy(void* dst, const void* src, usize size) {
  return std::memcpy(dst, src, size);
}

inline void* mem_set(void* dst, int value, usize size) {
  return std::memset(dst, value, size);
}

inline int mem_cmp(const void* a, const void* b, usize size) {
  return std::memcmp(a, b, size);
}

inline void bitset_and(u64* dst, const u64* a, const u64* b, usize word_count) {
  for (usize i = 0; i < word_count; ++i) {
    dst[i] = a[i] & b[i];
  }
}

inline void bitset_or(u64* dst, const u64* a, const u64* b, usize word_count) {
  for (usize i = 0; i < word_count; ++i) {
    dst[i] = a[i] | b[i];
  }
}

inline void bitset_xor(u64* dst, const u64* a, const u64* b, usize word_count) {
  for (usize i = 0; i < word_count; ++i) {
    dst[i] = a[i] ^ b[i];
  }
}

inline void bitset_not(u64* dst, const u64* a, usize word_count) {
  for (usize i = 0; i < word_count; ++i) {
    dst[i] = ~a[i];
  }
}

inline usize bitset_popcount_range(const u64* data, usize word_count) {
  usize total = 0;
  for (usize i = 0; i < word_count; ++i) {
    total += static_cast<usize>(Bits::popcount(data[i]));
  }
  return total;
}

inline constexpr VTable kScalarVTable = {
  .type        = Type::Scalar,
  .simd_width  = 0,
  .name        = "scalar",
  .mem_copy    = &Detail::mem_copy,
  .mem_set     = &Detail::mem_set,
  .mem_cmp     = &Detail::mem_cmp,
  .bitset_and  = &Detail::bitset_and,
  .bitset_or   = &Detail::bitset_or,
  .bitset_xor  = &Detail::bitset_xor,
  .bitset_not  = &Detail::bitset_not,
  .bitset_popcount_range = &Detail::bitset_popcount_range,
};

} // namespace Detail

} // namespace GameAK::Backend
