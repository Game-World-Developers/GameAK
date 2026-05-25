#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Backend {

enum class Type : u8 {
  Scalar,
  SIMD,
};

struct VTable {
  Type    type;
  usize   simd_width;
  const char* name;

  void* (*mem_copy)(void* dst, const void* src, usize size);
  void* (*mem_set)(void* dst, int value, usize size);
  int   (*mem_cmp)(const void* a, const void* b, usize size);

  void  (*bitset_and)(u64* dst, const u64* a, const u64* b, usize word_count);
  void  (*bitset_or)(u64* dst, const u64* a, const u64* b, usize word_count);
  void  (*bitset_xor)(u64* dst, const u64* a, const u64* b, usize word_count);
  void  (*bitset_not)(u64* dst, const u64* a, usize word_count);
  usize (*bitset_popcount_range)(const u64* data, usize word_count);
};

extern const VTable* g_vtable;

void init() noexcept;

[[nodiscard]] inline void* mem_copy(void* dst, const void* src, usize size) {
  return g_vtable->mem_copy(dst, src, size);
}

[[nodiscard]] inline void* mem_set(void* dst, int value, usize size) {
  return g_vtable->mem_set(dst, value, size);
}

[[nodiscard]] inline int mem_cmp(const void* a, const void* b, usize size) {
  return g_vtable->mem_cmp(a, b, size);
}

inline void bitset_and(u64* dst, const u64* a, const u64* b, usize word_count) {
  g_vtable->bitset_and(dst, a, b, word_count);
}

inline void bitset_or(u64* dst, const u64* a, const u64* b, usize word_count) {
  g_vtable->bitset_or(dst, a, b, word_count);
}

inline void bitset_xor(u64* dst, const u64* a, const u64* b, usize word_count) {
  g_vtable->bitset_xor(dst, a, b, word_count);
}

inline void bitset_not(u64* dst, const u64* a, usize word_count) {
  g_vtable->bitset_not(dst, a, word_count);
}

[[nodiscard]] inline usize bitset_popcount_range(const u64* data, usize word_count) {
  return g_vtable->bitset_popcount_range(data, word_count);
}

} // namespace GameAK::Backend
