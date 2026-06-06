#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::Backend {

/// Pure metadata describing the execution environment.
struct ExecutionProfile {
  u8   simd_width;        // 0=scalar, 16=SSE, 32=AVX2, 64=AVX-512
  u8   cache_line_bytes;  // 64 on x86, 128 on some ARM
  bool has_huge_pages;
};

} // namespace GameAK::Backend
