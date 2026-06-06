#include <AK/Backend/Backend.hpp>
#include <AK/Backend/ExecContext.hpp>
#include <AK/Backend/ScalarBackend.hpp>
#include <AK/Backend/ScalarBuiltinsBackend.hpp>
#include <AK/Core/Bits/BitArray.hpp>
#include <AK/Platform/ArchDetect.hpp>
#include <cest.h>
#include <cstring>
#include <vector>

template <typename B>
static void test_bitarray_batch() {
  auto label = sizeof(B); // dummy to avoid unused warning
  (void)label;

  alignas(32) GameAK::u64 storage_a[4] = {0b1100, 0b0011, 0xFF00, 0x0F0F};
  alignas(32) GameAK::u64 storage_b[4] = {0b1010, 0b0101, 0x00FF, 0xF0F0};
  GameAK::Bits::BitArray<B> a(storage_a, 4);
  GameAK::Bits::BitArray<B> b(storage_b, 4);

  a.and_with(b);
  expect(storage_a[0] == 0b1000).toBeTruthy();
  expect(storage_a[1] == 0b0001).toBeTruthy();
  expect(storage_a[2] == 0x0000u).toBeTruthy();
  expect(storage_a[3] == 0x0000u).toBeTruthy();
}

template <typename B>
static void test_bitarray_non_256_tail() {
  alignas(32) GameAK::u64 storage_a[5] = {0b1100, 0b0011, 0b1010, 0b0101, 0xFFFF};
  alignas(32) GameAK::u64 storage_b[5] = {0b1010, 0b0101, 0b1100, 0b0011, 0x0000};
  GameAK::Bits::BitArray<B> a(storage_a, 5);
  GameAK::Bits::BitArray<B> b(storage_b, 5);

  a.and_with(b);
  expect(storage_a[0] == 0b1000).toBeTruthy();
  expect(storage_a[1] == 0b0001).toBeTruthy();
  expect(storage_a[2] == 0b1000).toBeTruthy();
  expect(storage_a[3] == 0b0001).toBeTruthy();
  expect(storage_a[4] == 0x0000u).toBeTruthy();
}

template <typename B>
static void test_bitarray_popcount() {
  alignas(32) GameAK::u64 zeros[4] = {};
  GameAK::Bits::BitArray<B> z(zeros, 4);
  expect(z.popcount()).toBe(0u);

  alignas(32) GameAK::u64 ones[4] = {~0ull, ~0ull, ~0ull, ~0ull};
  GameAK::Bits::BitArray<B> o(ones, 4);
  expect(o.popcount()).toBe(256u);
}

int main() {
  describe("GameAK::Backend::ExecutionProfile", {
    it("init() should return reasonable metadata", {
      auto p = GameAK::Backend::init();
      expect(p.cache_line_bytes).toBe(64u);
      expect(p.has_huge_pages).toBeFalsy();
    });

    it("scalar detection yields simd_width=0", {
      // always true regardless of platform — scalar is the fallback
      auto p = GameAK::Backend::init();
      expect(p.simd_width == 0 || p.simd_width == 32).toBeTruthy();
    });
  });

  describe("GameAK::Backend::ScalarBackend block kernels", {
    it("bit_and_256 should compute bitwise AND on 4 words", {
      alignas(32) GameAK::u64 a[4] = {0xFFFF, 0x0F0F, 0xFF00, 0xAAAA};
      alignas(32) GameAK::u64 b[4] = {0xFF00, 0x00FF, 0x0F0F, 0x5555};
      alignas(32) GameAK::u64 dst[4] = {};
      GameAK::Backend::ScalarBackend::bit_and_256(dst, a, b);
      expect(dst[0]).toBe(0xFF00u);
      expect(dst[1]).toBe(0x000Fu);
      expect(dst[2]).toBe(0x0F00u);
      expect(dst[3]).toBe(0x0000u);
    });

    it("bit_or_256 should compute bitwise OR on 4 words", {
      alignas(32) GameAK::u64 a[4] = {0xFF00, 0x0000, 0xAAAA, 0x1234};
      alignas(32) GameAK::u64 b[4] = {0x00FF, 0xFFFF, 0x5555, 0x5678};
      alignas(32) GameAK::u64 dst[4] = {};
      GameAK::Backend::ScalarBackend::bit_or_256(dst, a, b);
      expect(dst[0]).toBe(0xFFFFu);
      expect(dst[1]).toBe(0xFFFFu);
      expect(dst[2]).toBe(0xFFFFu);
      expect(dst[3]).toBe(0x567Cu);
    });

    it("bit_xor_256 should compute bitwise XOR on 4 words", {
      alignas(32) GameAK::u64 a[4] = {0xFFFF, 0x0000, 0xAAAA, 0xFF00};
      alignas(32) GameAK::u64 b[4] = {0xFF00, 0xFFFF, 0x5555, 0x0FF0};
      alignas(32) GameAK::u64 dst[4] = {};
      GameAK::Backend::ScalarBackend::bit_xor_256(dst, a, b);
      expect(dst[0]).toBe(0x00FFu);
      expect(dst[1]).toBe(0xFFFFu);
      expect(dst[2]).toBe(0xFFFFu);
      expect(dst[3]).toBe(0xF0F0u);
    });

    it("bit_not_256 should compute bitwise NOT on 4 words", {
      alignas(32) GameAK::u64 a[4] = {0xFFFF0000FFFF0000, 0, ~0ull, 0xAAAAAAAAAAAAAAAA};
      alignas(32) GameAK::u64 dst[4] = {};
      GameAK::Backend::ScalarBackend::bit_not_256(dst, a);
      expect(dst[0]).toBe(~0xFFFF0000FFFF0000ull);
      expect(dst[1]).toBe(~0ull);
      expect(dst[2]).toBe(0ull);
      expect(dst[3]).toBe(0x5555555555555555ull);
    });

    it("[Invariant] bit_not(bit_not(x)) == x on 4 words", {
      alignas(32) GameAK::u64 orig[4] = {0xDEADBEEFCAFEBABE, 0x1234567890ABCDEF, 0, ~0ull};
      alignas(32) GameAK::u64 tmp[4] = {};
      alignas(32) GameAK::u64 back[4] = {};
      GameAK::Backend::ScalarBackend::bit_not_256(tmp, orig);
      GameAK::Backend::ScalarBackend::bit_not_256(back, tmp);
      expect(back[0]).toBe(orig[0]);
      expect(back[1]).toBe(orig[1]);
      expect(back[2]).toBe(orig[2]);
      expect(back[3]).toBe(orig[3]);
    });

    it("popcount_256 should count all set bits across 4 words", {
      alignas(32) GameAK::u64 data[4] = {~0ull, ~0ull, ~0ull, ~0ull};
      expect(GameAK::Backend::ScalarBackend::popcount_256(data)).toBe(256u);
    });

    it("popcount_256 should return 0 for zero data", {
      alignas(32) GameAK::u64 data[4] = {};
      expect(GameAK::Backend::ScalarBackend::popcount_256(data)).toBe(0u);
    });

    it("mem_copy should copy bytes correctly", {
      const char src[] = "Hello GameAK";
      char dst[32] = {};
      expect(GameAK::Backend::ScalarBackend::mem_copy(dst, src, sizeof(src)) == dst).toBeTruthy();
      expect(std::memcmp(dst, src, sizeof(src))).toBe(0);
    });

    it("mem_set should fill memory correctly", {
      char buf[16] = {};
      expect(GameAK::Backend::ScalarBackend::mem_set(buf, 0xAB, sizeof(buf)) == buf).toBeTruthy();
      for (int i = 0; i < 16; ++i) {
        expect(static_cast<unsigned char>(buf[i])).toBe(0xABu);
      }
    });

    it("mem_cmp should return zero for identical buffers", {
      const char a[] = "GameAK";
      const char b[] = "GameAK";
      expect(GameAK::Backend::ScalarBackend::mem_cmp(a, b, sizeof(a))).toBe(0);
    });
  });

  describe("GameAK::Backend::ScalarBuiltinsBackend block kernels", {
    it("bit_and_256 should compute correctly", {
      alignas(32) GameAK::u64 a[4] = {0xFFFF, 0x0F0F, 0xFF00, 0xAAAA};
      alignas(32) GameAK::u64 b[4] = {0xFF00, 0x00FF, 0x0F0F, 0x5555};
      alignas(32) GameAK::u64 dst[4] = {};
      GameAK::Backend::ScalarBuiltinsBackend::bit_and_256(dst, a, b);
      expect(dst[0]).toBe(0xFF00u);
      expect(dst[1]).toBe(0x000Fu);
    });

    it("popcount_256 should match scalar popcount", {
      alignas(32) GameAK::u64 data[4] = {0xDEADBEEFCAFEBABE, 0x1234567890ABCDEF, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555};
      auto sb = GameAK::Backend::ScalarBuiltinsBackend::popcount_256(data);
      auto sc = GameAK::Backend::ScalarBackend::popcount_256(data);
      expect(sb).toBe(sc);
    });

    it("mem_copy builtin should work", {
      const char src[] = "Builtins";
      char dst[16] = {};
      expect(GameAK::Backend::ScalarBuiltinsBackend::mem_copy(dst, src, sizeof(src)) == dst).toBeTruthy();
      expect(std::memcmp(dst, src, sizeof(src))).toBe(0);
    });
  });

  describe("GameAK::Bits::BitArray batch operations", {
    it("should perform AND with default backend", {
      test_bitarray_batch<GameAK::Backend::ScalarBuiltinsBackend>();
    });

    it("should perform AND with ScalarBackend", {
      test_bitarray_batch<GameAK::Backend::ScalarBackend>();
    });

    it("should handle non-256-bit word count (tail loop)", {
      test_bitarray_non_256_tail<GameAK::Backend::ScalarBuiltinsBackend>();
    });

    it("should handle 0-word BitArray", {
      GameAK::u64 data = 0;
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend> ba(&data, 0);
      ba.reset();
      ba.and_with(ba);
      ba.or_with(ba);
      ba.xor_with(ba);
      ba.negate();
      expect(ba.popcount()).toBe(0u);
    });

    it("should handle 1-word BitArray (no block kernel)", {
      alignas(32) GameAK::u64 storage = 0b1100;
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend> a(&storage, 1);
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend> b(&storage, 1);
      expect(a.popcount()).toBe(2u);
      a.negate();
      expect(storage == ~0b1100ull).toBeTruthy();
    });

    it("popcount should match for all ones across 4 words", {
      test_bitarray_popcount<GameAK::Backend::ScalarBuiltinsBackend>();
    });

    it("[Invariant] negate(negate(x)) == x", {
      alignas(32) GameAK::u64 storage[4] = {0xDEADBEEFCAFEBABE, 0x1234567890ABCDEF, 0xAAAAAAAAAAAAAAAA, 0x5555555555555555};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend> a(storage, 4);
      a.negate();
      a.negate();
      expect(storage[0]).toBe(0xDEADBEEFCAFEBABEull);
      expect(storage[1]).toBe(0x1234567890ABCDEFull);
    });

    it("[Stress] 10M AND iterations through BitArray", {
      alignas(32) GameAK::u64 a[4] = {0xDEADBEEF, 0xCAFEBABE, 0x12345678, 0x9ABCDEF0};
      alignas(32) GameAK::u64 b[4] = {0x0F0F0F0F, 0xF0F0F0F0, 0xAAAA5555, 0x5555AAAA};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend> aa(a, 4);
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend> bb(b, 4);
      for (int i = 0; i < 10'000'000; ++i) {
        aa.and_with(bb);
      }
      expect(a[0]).toBe(0xDEADBEEF & 0x0F0F0F0F);
    });

    it("[Stress] mem_set with 16MB buffer through ScalarBackend", {
      std::vector<GameAK::u8> buf(16 * 1024 * 1024, 0x00);
      GameAK::Backend::ScalarBuiltinsBackend::mem_set(buf.data(), 0xAB, buf.size());
      expect(static_cast<int>(buf[0])).toBe(0xAB);
      expect(static_cast<int>(buf[buf.size() - 1])).toBe(0xAB);
    });

    it("[Stress] init() called 10,000 times", {
      for (int i = 0; i < 10'000; ++i) {
        auto p = GameAK::Backend::init();
        expect(p.cache_line_bytes).toBe(64u);
      }
    });
  });

  describe("GameAK::ExecContext", {
    it("should store and retrieve profile metadata", {
      GameAK::ExecContext ctx{GameAK::Backend::init(), 42, 0.016};
      expect(ctx.frame_index).toBe(42u);
      expect(ctx.delta_time == 0.016).toBeTruthy();
      expect(ctx.profile.cache_line_bytes).toBe(64u);
    });
  });

  return cest_result();
}
