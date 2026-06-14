#pragma once

#include <cstddef>
#include <cstring>

namespace gameak::core::simd {

// Generic (portable) implementations — source of truth

inline void memcopy(void* dst, const void* src, size_t n) {
    std::memcpy(dst, src, n);
}

inline void memzero(void* dst, size_t n) {
    std::memset(dst, 0, n);
}

// Compile-time selection: platform.h macros determine which implementation is used.
// Runtime selection can be layered on top via detect_platform().
//
// Currently, only generic implementations are provided.
// Architecture-specific optimizations (SSE2, AVX2, NEON) follow this pattern:
//
//   #if defined(__SSE2__)
//   inline void memcopy(void* dst, const void* src, size_t n) {
//       // SSE2-optimized copy
//   }
//   #endif
//
// Every optimized implementation must:
//   1. Preserve generic behavior
//   2. Pass the same test suite
//   3. Remain interchangeable with the generic version

} // namespace gameak::core::simd
