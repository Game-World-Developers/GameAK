#include <AK/Backend/Backend.hpp>
#include <AK/Platform/ArchDetect.hpp>
#include <AK/Platform/CompilerDetect.hpp>

namespace GameAK::Backend {

ExecutionProfile init() noexcept {
#if defined(GAMEAK_ARCH_X86_64)
  #if defined(GAMEAK_COMPILER_GCC) || defined(GAMEAK_COMPILER_CLANG)
    if (__builtin_cpu_supports("avx2")) {
      return {.simd_width = 32, .cache_line_bytes = 64, .has_huge_pages = false};
    }
  #endif
#elif defined(GAMEAK_ARCH_ARM64)
  // future: NEON detection via getauxval(AT_HWCAP)
#endif
  return {.simd_width = 0, .cache_line_bytes = 64, .has_huge_pages = false};
}

} // namespace GameAK::Backend
