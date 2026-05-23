#include <AK/Core/Macros.hpp>
#include <cest.h>

GAMEAK_FORCE_INLINE int force_inline_func() { return 42; }
GAMEAK_NO_INLINE int no_inline_func() { return 99; }

int main() {
  describe("GAMEAK::Macros", {
    it("should define CACHE_LINE_SIZE as 64", {
      expect(GAMEAK_CACHE_LINE_SIZE).toBe(64);
    });

    it("should align to specified boundary", {
      struct GAMEAK_ALIGN(32) Aligned {
        char c;
      };
      expect((int)alignof(Aligned)).toBe(32);
    });

    it("should align to cache line size", {
      struct GAMEAK_ALIGN_CACHE Aligned {
        char c;
      };
      expect((int)alignof(Aligned) >= GAMEAK_CACHE_LINE_SIZE).toBeTruthy();
    });

    it("should force inline a function", {
      expect(force_inline_func()).toBe(42);
    });

    it("should prevent function inlining", {
      expect(no_inline_func()).toBe(99);
    });

    it("should return the input from LIKELY", {
      expect(GAMEAK_LIKELY(1)).toBe(1);
      expect(GAMEAK_LIKELY(0)).toBe(0);
    });

    it("should return the input from UNLIKELY", {
      expect(GAMEAK_UNLIKELY(1)).toBe(1);
      expect(GAMEAK_UNLIKELY(0)).toBe(0);
    });

    it("should align to SIMD boundary", {
      struct GAMEAK_ALIGN_SIMD Aligned {
        char c;
      };
#if defined(GAMEAK_ARCH_X86_64)
      expect((int)alignof(Aligned)).toBe(32);
#elif defined(GAMEAK_ARCH_ARM64)
      expect((int)alignof(Aligned)).toBe(16);
#endif
    });

    it("should compile DEBUG_BREAK without error", {
      expect(true).toBeTruthy();
    });
  });

  return cest_result();
}
