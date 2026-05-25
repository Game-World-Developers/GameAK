#include <AK/Backend/Backend.hpp>

// SIMD backend stub — currently falls back to scalar operations.
// When SIMD intrinsics are added, this file provides the VTable
// with SSE/AVX/NEON implementations and is selected by Backend::init()
// based on runtime CPU detection.

namespace GameAK::Backend {

// Future: define Detail::kSIMDVTable with intrinsic-based implementations.

} // namespace GameAK::Backend
