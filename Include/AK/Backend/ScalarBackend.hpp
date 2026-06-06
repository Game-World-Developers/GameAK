#pragma once

#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Types.hpp>

#include <cstring>

namespace GameAK::Backend {

/// Scalar backend — pure C++ loops, no builtins, no intrinsics.
struct ScalarBackend {
  // ---- 256-bit block kernels (4 × u64) ----

  static void bit_and_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    dst[0] = a[0] & b[0];
    dst[1] = a[1] & b[1];
    dst[2] = a[2] & b[2];
    dst[3] = a[3] & b[3];
  }

  static void bit_or_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    dst[0] = a[0] | b[0];
    dst[1] = a[1] | b[1];
    dst[2] = a[2] | b[2];
    dst[3] = a[3] | b[3];
  }

  static void bit_xor_256(u64 *dst, const u64 *a, const u64 *b) noexcept {
    dst[0] = a[0] ^ b[0];
    dst[1] = a[1] ^ b[1];
    dst[2] = a[2] ^ b[2];
    dst[3] = a[3] ^ b[3];
  }

  static void bit_not_256(u64 *dst, const u64 *a) noexcept {
    dst[0] = ~a[0];
    dst[1] = ~a[1];
    dst[2] = ~a[2];
    dst[3] = ~a[3];
  }

  static usize popcount_256(const u64 *data) noexcept {
    return static_cast<usize>(Bits::popcount(data[0]))
         + static_cast<usize>(Bits::popcount(data[1]))
         + static_cast<usize>(Bits::popcount(data[2]))
         + static_cast<usize>(Bits::popcount(data[3]));
  }

  // ---- Bulk memory operations ----

  static void *mem_copy(void *d, const void *s, usize n) noexcept {
    return std::memcpy(d, s, n);
  }

  static void *mem_set(void *d, int v, usize n) noexcept {
    return std::memset(d, v, n);
  }

  static int mem_cmp(const void *a, const void *b, usize n) noexcept {
    return std::memcmp(a, b, n);
  }
};

} // namespace GameAK::Backend
