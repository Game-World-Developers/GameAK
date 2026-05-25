#include <AK/Backend/Backend.hpp>
#include <AK/Backend/ScalarBackend.hpp>

namespace GameAK::Backend {

const VTable* g_vtable = &Detail::kScalarVTable;

void init() noexcept {
  // Future: detect CPU features and select SIMD backend when available.
  g_vtable = &Detail::kScalarVTable;
}

} // namespace GameAK::Backend
