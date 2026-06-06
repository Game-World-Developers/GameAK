#pragma once

#include <AK/Backend/ScalarBuiltinsBackend.hpp>

namespace GameAK::Backend {

/// AVX2 backend — placeholder that currently delegates to ScalarBuiltinsBackend.
/// When AVX2 intrinsics are implemented, this will use _mm256_* operations.
struct AVX2Backend : ScalarBuiltinsBackend {};

} // namespace GameAK::Backend
