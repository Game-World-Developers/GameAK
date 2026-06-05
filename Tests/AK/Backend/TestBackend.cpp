#include <AK/Backend/Backend.hpp>
#include <AK/Backend/ScalarBackend.hpp>
#include <AK/Core/Bits/BitOps.hpp>
#include <cest.h>
#include <cstring>

int main() {
  describe("GameAK::Backend", {
    it("init() should set the global vtable to scalar", {
      GameAK::Backend::init();
      expect(GameAK::Backend::g_vtable != nullptr).toBeTruthy();
      expect(GameAK::Backend::g_vtable->type == GameAK::Backend::Type::Scalar)
          .toBeTruthy();
      expect(GameAK::Backend::g_vtable->name[0] == 's').toBeTruthy();
    });

    it("mem_copy should copy bytes correctly", {
      GameAK::Backend::init();
      const char src[] = "Hello GameAK";
      char dst[32] = {};
      expect(GameAK::Backend::mem_copy(dst, src, sizeof(src)) == dst)
          .toBeTruthy();
      expect(std::memcmp(dst, src, sizeof(src))).toBe(0);
    });

    it("mem_copy should handle zero-size copy", {
      GameAK::Backend::init();
      char dst[8] = {};
      char src[8] = {};
      expect(GameAK::Backend::mem_copy(dst, src, 0) == dst).toBeTruthy();
    });

    it("mem_set should fill memory correctly", {
      GameAK::Backend::init();
      char buf[16] = {};
      expect(GameAK::Backend::mem_set(buf, 0xAB, sizeof(buf)) == buf)
          .toBeTruthy();
      for (int i = 0; i < 16; ++i) {
        expect(static_cast<unsigned char>(buf[i])).toBe(0xABu);
      }
    });

    it("mem_set should handle zero-size set", {
      GameAK::Backend::init();
      char buf[8] = {};
      expect(GameAK::Backend::mem_set(buf, 0xFF, 0) == buf).toBeTruthy();
    });

    it("mem_cmp should return zero for identical buffers", {
      GameAK::Backend::init();
      const char a[] = "GameAK";
      const char b[] = "GameAK";
      expect(GameAK::Backend::mem_cmp(a, b, sizeof(a))).toBe(0);
    });

    it("mem_cmp should return non-zero for different buffers", {
      GameAK::Backend::init();
      const char a[] = "GameAK";
      const char b[] = "GameBK";
      expect(GameAK::Backend::mem_cmp(a, b, sizeof(a)) != 0).toBeTruthy();
    });

    it("mem_cmp should return zero for zero-size compare", {
      GameAK::Backend::init();
      const char a[] = "GameAK";
      const char b[] = "GameBK";
      expect(GameAK::Backend::mem_cmp(a, b, 0)).toBe(0);
    });

    it("bitset_and should compute bitwise AND", {
      GameAK::Backend::init();
      GameAK::u64 a[2] = {0xFFFF, 0x0F0F};
      GameAK::u64 b[2] = {0xFF00, 0x00FF};
      GameAK::u64 dst[2] = {};
      GameAK::Backend::bitset_and(dst, a, b, 2);
      expect(dst[0]).toBe(0xFF00u);
      expect(dst[1]).toBe(0x000Fu);
    });

    it("bitset_or should compute bitwise OR", {
      GameAK::Backend::init();
      GameAK::u64 a[2] = {0xFF00, 0x0000};
      GameAK::u64 b[2] = {0x00FF, 0xFFFF};
      GameAK::u64 dst[2] = {};
      GameAK::Backend::bitset_or(dst, a, b, 2);
      expect(dst[0]).toBe(0xFFFFu);
      expect(dst[1]).toBe(0xFFFFu);
    });

    it("bitset_xor should compute bitwise XOR", {
      GameAK::Backend::init();
      GameAK::u64 a[2] = {0xFFFF, 0x0000};
      GameAK::u64 b[2] = {0xFF00, 0xFFFF};
      GameAK::u64 dst[2] = {};
      GameAK::Backend::bitset_xor(dst, a, b, 2);
      expect(dst[0]).toBe(0x00FFu);
      expect(dst[1]).toBe(0xFFFFu);
    });

    it("bitset_not should compute bitwise NOT", {
      GameAK::Backend::init();
      GameAK::u64 a[2] = {0xFFFF0000FFFF0000, 0};
      GameAK::u64 dst[2] = {};
      GameAK::Backend::bitset_not(dst, a, 2);
      expect(dst[0]).toBe(~0xFFFF0000FFFF0000ull);
      expect(dst[1]).toBe(~0ull);
    });

    it("[Property] bitset_not(bitset_not(x)) == x", {
      GameAK::Backend::init();
      GameAK::u64 orig[2] = {0xDEADBEEFCAFEBABE, 0x1234567890ABCDEF};
      GameAK::u64 tmp[2] = {};
      GameAK::u64 back[2] = {};
      GameAK::Backend::bitset_not(tmp, orig, 2);
      GameAK::Backend::bitset_not(back, tmp, 2);
      expect(back[0]).toBe(orig[0]);
      expect(back[1]).toBe(orig[1]);
    });

    it("bitset_popcount_range should count all set bits", {
      GameAK::Backend::init();
      GameAK::u64 data[3] = {0xFFFFFFFFFFFFFFFF, 0xFFFFFFFFFFFFFFFF,
                              0xFFFFFFFFFFFFFFFF};
      expect(GameAK::Backend::bitset_popcount_range(data, 3)).toBe(192u);
    });

    it("bitset_popcount_range should return 0 for zero data", {
      GameAK::Backend::init();
      GameAK::u64 data[3] = {};
      expect(GameAK::Backend::bitset_popcount_range(data, 3)).toBe(0u);
    });

    it("[Invariant] bitset_popcount_range agrees with BitOps::popcount", {
      GameAK::Backend::init();
      GameAK::u64 data[4] = {0xAAAAAAAAAAAAAAAA, 0x5555555555555555,
                               0xDEADBEEFCAFEBABE, 0x1234567890ABCDEF};
      GameAK::usize expected = GameAK::Bits::popcount(data[0]) +
                               GameAK::Bits::popcount(data[1]) +
                               GameAK::Bits::popcount(data[2]) +
                               GameAK::Bits::popcount(data[3]);
      expect(GameAK::Backend::bitset_popcount_range(data, 4)).toBe(expected);
    });

    it("[Edge: bitset ops zero word_count should not crash]", {
      GameAK::Backend::init();
      GameAK::u64 dst[1] = {0xDEAD};
      GameAK::u64 a[1] = {0xBEEF};
      GameAK::u64 b[1] = {0xCAFE};
      GameAK::Backend::bitset_and(dst, a, b, 0);
      GameAK::Backend::bitset_or(dst, a, b, 0);
      GameAK::Backend::bitset_xor(dst, a, b, 0);
      GameAK::Backend::bitset_not(dst, a, 0);
      expect(GameAK::Backend::bitset_popcount_range(a, 0)).toBe(0u);
    });

    it("[Stress: Backend::init called 1000x]", {
      for (int i = 0; i < 1000; ++i) {
        GameAK::Backend::init();
      }
      expect(GameAK::Backend::g_vtable->type == GameAK::Backend::Type::Scalar)
          .toBeTruthy();
    });
  });

  return cest_result();
}
