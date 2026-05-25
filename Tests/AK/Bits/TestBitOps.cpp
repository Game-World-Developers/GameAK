#include <AK/Core/Bits/BitOps.hpp>
#include <cest.h>

int main() {
  describe("GameAK::Bits::bit", {
    it("should return 1 << n for u32", {
      expect(GameAK::Bits::bit(0u)).toBe(1u);
      expect(GameAK::Bits::bit(1u)).toBe(2u);
      expect(GameAK::Bits::bit(3u)).toBe(8u);
      expect(GameAK::Bits::bit(5u)).toBe(32u);
      expect(GameAK::Bits::bit(10u)).toBe(1024u);
    });

    it("should return 1 << n for u64", {
      expect(GameAK::Bits::bit(0ull)).toBe(1ull);
      expect(GameAK::Bits::bit(31ull)).toBe(2147483648ull);
      expect(GameAK::Bits::bit(63ull)).toBe(9223372036854775808ull);
    });
  });

  describe("GameAK::Bits::mask", {
    it("should return (1 << n) - 1 for u32", {
      expect(GameAK::Bits::mask(0u)).toBe(0u);
      expect(GameAK::Bits::mask(1u)).toBe(1u);
      expect(GameAK::Bits::mask(3u)).toBe(7u);
      expect(GameAK::Bits::mask(5u)).toBe(31u);
      expect(GameAK::Bits::mask(10u)).toBe(1023u);
    });

    it("should return (1 << n) - 1 for u64", {
      expect(GameAK::Bits::mask(0ull)).toBe(0ull);
      expect(GameAK::Bits::mask(1ull)).toBe(1ull);
      expect(GameAK::Bits::mask(32ull)).toBe(4294967295ull);
    });
  });

  describe("GameAK::Bits::is_power_of_two", {
    it("should return true for powers of two", {
      expect(GameAK::Bits::is_power_of_two(1u)).toBeTruthy();
      expect(GameAK::Bits::is_power_of_two(2u)).toBeTruthy();
      expect(GameAK::Bits::is_power_of_two(4u)).toBeTruthy();
      expect(GameAK::Bits::is_power_of_two(1024u)).toBeTruthy();
      expect(GameAK::Bits::is_power_of_two(2147483648u)).toBeTruthy();
    });

    it("should return false for zero", {
      expect(GameAK::Bits::is_power_of_two(0u)).toBeFalsy();
    });

    it("should return false for non-powers of two", {
      expect(GameAK::Bits::is_power_of_two(3u)).toBeFalsy();
      expect(GameAK::Bits::is_power_of_two(5u)).toBeFalsy();
      expect(GameAK::Bits::is_power_of_two(1023u)).toBeFalsy();
    });
  });

  describe("GameAK::Bits::align_up", {
    it("should align up to power-of-two alignment", {
      expect(GameAK::Bits::align_up(0u, 4u)).toBe(0u);
      expect(GameAK::Bits::align_up(1u, 4u)).toBe(4u);
      expect(GameAK::Bits::align_up(3u, 4u)).toBe(4u);
      expect(GameAK::Bits::align_up(4u, 4u)).toBe(4u);
      expect(GameAK::Bits::align_up(5u, 4u)).toBe(8u);
    });

    it("should align up to 64-bit alignment", {
      expect(GameAK::Bits::align_up(0ull, 16ull)).toBe(0ull);
      expect(GameAK::Bits::align_up(15ull, 16ull)).toBe(16ull);
      expect(GameAK::Bits::align_up(16ull, 16ull)).toBe(16ull);
    });
  });

  describe("GameAK::Bits::popcount", {
    it("should return 0 for zero", {
      expect(GameAK::Bits::popcount(0u)).toBe(0u);
    });

    it("should return 1 for powers of two", {
      expect(GameAK::Bits::popcount(1u)).toBe(1u);
      expect(GameAK::Bits::popcount(2u)).toBe(1u);
      expect(GameAK::Bits::popcount(512u)).toBe(1u);
    });

    it("should count set bits correctly", {
      expect(GameAK::Bits::popcount(0xFFu)).toBe(8u);
      expect(GameAK::Bits::popcount(0xFFFFu)).toBe(16u);
      expect(GameAK::Bits::popcount(0xFFFFFFFFu)).toBe(32u);
      expect(GameAK::Bits::popcount(7u)).toBe(3u);
    });

    it("should handle u64 values", {
      expect(GameAK::Bits::popcount(0xFFFFFFFFFFFFFFFFull)).toBe(64u);
      expect(GameAK::Bits::popcount(0x0123456789ABCDEFull)).toBe(32u);
    });
  });

  return cest_result();
}
