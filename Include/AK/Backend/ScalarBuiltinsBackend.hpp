#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

#include <cstring>

namespace GameAK::Backend {

/// Scalar backend optimized with compiler builtins.
/// Uses __builtin_popcountll, __builtin_assume_aligned, __builtin_prefetch,
/// and __builtin_memcpy for maximum scalar throughput.
struct ScalarBuiltinsBackend {
  // ---- 256-bit block kernels (4 × u64) ----

  static void bit_and_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    u64 *d = static_cast<u64 *>(__builtin_assume_aligned(dst, 32));
    const u64 *aa = static_cast<const u64 *>(__builtin_assume_aligned(a, 32));
    const u64 *bb = static_cast<const u64 *>(__builtin_assume_aligned(b, 32));
    d[0] = aa[0] & bb[0];
    d[1] = aa[1] & bb[1];
    d[2] = aa[2] & bb[2];
    d[3] = aa[3] & bb[3];
  }

  static void bit_or_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    u64 *d = static_cast<u64 *>(__builtin_assume_aligned(dst, 32));
    const u64 *aa = static_cast<const u64 *>(__builtin_assume_aligned(a, 32));
    const u64 *bb = static_cast<const u64 *>(__builtin_assume_aligned(b, 32));
    d[0] = aa[0] | bb[0];
    d[1] = aa[1] | bb[1];
    d[2] = aa[2] | bb[2];
    d[3] = aa[3] | bb[3];
  }

  static void bit_xor_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    u64 *d = static_cast<u64 *>(__builtin_assume_aligned(dst, 32));
    const u64 *aa = static_cast<const u64 *>(__builtin_assume_aligned(a, 32));
    const u64 *bb = static_cast<const u64 *>(__builtin_assume_aligned(b, 32));
    d[0] = aa[0] ^ bb[0];
    d[1] = aa[1] ^ bb[1];
    d[2] = aa[2] ^ bb[2];
    d[3] = aa[3] ^ bb[3];
  }

  static void bit_not_256(u64 *dst, const u64 *a) noexcept {
    u64 *d = static_cast<u64 *>(__builtin_assume_aligned(dst, 32));
    const u64 *aa = static_cast<const u64 *>(__builtin_assume_aligned(a, 32));
    d[0] = ~aa[0];
    d[1] = ~aa[1];
    d[2] = ~aa[2];
    d[3] = ~aa[3];
  }

  static usize popcount_256(const u64 *data) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    const u64 *d = static_cast<const u64 *>(__builtin_assume_aligned(data, 32));
    return static_cast<usize>(__builtin_popcountll(d[0]))
         + static_cast<usize>(__builtin_popcountll(d[1]))
         + static_cast<usize>(__builtin_popcountll(d[2]))
         + static_cast<usize>(__builtin_popcountll(d[3]));
#else
    return static_cast<usize>(Bits::popcount(data[0]))
         + static_cast<usize>(Bits::popcount(data[1]))
         + static_cast<usize>(Bits::popcount(data[2]))
         + static_cast<usize>(Bits::popcount(data[3]));
#endif
  }

  // ---- Bulk memory operations ----

  static void *mem_copy(void *d, const void *s, usize n) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_memcpy(d, s, n);
#else
    return std::memcpy(d, s, n);
#endif
  }

  static void *mem_set(void *d, int v, usize n) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_memset(d, v, n);
#else
    return std::memset(d, v, n);
#endif
  }

  static int mem_cmp(const void *a, const void *b, usize n) noexcept {
#if defined(__GNUC__) || defined(__clang__)
    return __builtin_memcmp(a, b, n);
#else
    return std::memcmp(a, b, n);
#endif
  }
};

} // namespace GameAK::Backend
