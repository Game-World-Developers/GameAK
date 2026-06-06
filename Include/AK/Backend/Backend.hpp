#pragma once

#include <AK/Backend/ExecutionProfile.hpp>

namespace GameAK::Backend {

/// Initializes the backend and returns an ExecutionProfile with runtime-detected
/// hardware capabilities (SIMD width, cache line size, huge page support).
ExecutionProfile init() noexcept;

} // namespace GameAK::Backend
