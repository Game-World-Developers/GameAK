#include <AK/Platform/CompilerDetect.hpp>
#include <cest.h>

#ifdef GAMEAK_COMPILER_MSVC
constexpr bool kCompilerMsvc = true;
#else
constexpr bool kCompilerMsvc = false;
#endif

#ifdef GAMEAK_COMPILER_CLANG
constexpr bool kCompilerClang = true;
#else
constexpr bool kCompilerClang = false;
#endif

#ifdef GAMEAK_COMPILER_GCC
constexpr bool kCompilerGcc = true;
#else
constexpr bool kCompilerGcc = false;
#endif

int main() {
  describe("GameAK::CompilerDetect", {
    it("should define exactly one compiler macro", {
      int count = kCompilerMsvc + kCompilerClang + kCompilerGcc;
      expect(count).toBe(1);
    });

    it("should detect the correct compiler", {
#if defined(__clang__)
      expect(kCompilerClang).toBeTruthy();
#elif defined(_MSC_VER)
      expect(kCompilerMsvc).toBeTruthy();
#elif defined(__GNUC__)
      expect(kCompilerGcc).toBeTruthy();
#endif
    });
  });

  return cest_result();
}
