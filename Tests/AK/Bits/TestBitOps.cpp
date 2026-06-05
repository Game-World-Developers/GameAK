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

    it("[Edge: bit(31) for u32 boundary]", {
      expect(GameAK::Bits::bit(31u)).toBe(2147483648u);
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

    it("[Edge: align_up with zero value]", {
      expect(GameAK::Bits::align_up(0u, 1u)).toBe(0u);
      expect(GameAK::Bits::align_up(0u, 8u)).toBe(0u);
      expect(GameAK::Bits::align_up(0u, 4096u)).toBe(0u);
    });

    it("[Edge: align_up with already-aligned value]", {
      for (GameAK::u32 align = 1; align <= 4096; align *= 2) {
        expect(GameAK::Bits::align_up(align, align)).toBe(align);
      }
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

  describe("GameAK::Bits::clz", {
    it("should return 32 for u32 zero", {
      expect(GameAK::Bits::clz(0u)).toBe(32u);
    });

    it("should return 31 for u32(1)", {
      expect(GameAK::Bits::clz(1u)).toBe(31u);
    });

    it("should return 0 for u32 max", {
      expect(GameAK::Bits::clz(0xFFFFFFFFu)).toBe(0u);
    });

    it("should return 64 for u64 zero", {
      expect(GameAK::Bits::clz(0ull)).toBe(64u);
    });

    it("should return correct leading zeros for u32", {
      expect(GameAK::Bits::clz(0x00FFFFFFu)).toBe(8u);
      expect(GameAK::Bits::clz(0x0000FFFFu)).toBe(16u);
      expect(GameAK::Bits::clz(0x00000001u)).toBe(31u);
    });

    it("should return correct leading zeros for u64", {
      expect(GameAK::Bits::clz(0x00FFFFFFFFFFFFFFull)).toBe(8u);
      expect(GameAK::Bits::clz(0x0000000000000001ull)).toBe(63u);
    });
  });

  describe("GameAK::Bits::ctz", {
    it("should return 32 for u32 zero", {
      expect(GameAK::Bits::ctz(0u)).toBe(32u);
    });

    it("should return 0 for u32 odd values", {
      expect(GameAK::Bits::ctz(1u)).toBe(0u);
      expect(GameAK::Bits::ctz(3u)).toBe(0u);
      expect(GameAK::Bits::ctz(0xFFu)).toBe(0u);
    });

    it("should return correct trailing zeros for u32", {
      expect(GameAK::Bits::ctz(2u)).toBe(1u);
      expect(GameAK::Bits::ctz(8u)).toBe(3u);
      expect(GameAK::Bits::ctz(0x100u)).toBe(8u);
      expect(GameAK::Bits::ctz(0x80000000u)).toBe(31u);
    });

    it("should return 64 for u64 zero", {
      expect(GameAK::Bits::ctz(0ull)).toBe(64u);
    });

    it("should return correct trailing zeros for u64", {
      expect(GameAK::Bits::ctz(0x100000000ull)).toBe(32u);
    });
  });

  describe("GameAK::Bits::rotl", {
    it("should rotate left by 0", {
      expect(GameAK::Bits::rotl(0x12345678u, 0)).toBe(0x12345678u);
    });

    it("should rotate left u32", {
      expect(GameAK::Bits::rotl(0x80000000u, 1)).toBe(0x1u);
      expect(GameAK::Bits::rotl(0x12345678u, 4)).toBe(0x23456781u);
    });

    it("should rotate left u64", {
      expect(GameAK::Bits::rotl(0x8000000000000000ull, 1)).toBe(0x1ull);
    });

    it("[Property] rotl(x, 32) == x for u32", {
      expect(GameAK::Bits::rotl(0x12345678u, 32)).toBe(0x12345678u);
    });
  });

  describe("GameAK::Bits::rotr", {
    it("should rotate right by 0", {
      expect(GameAK::Bits::rotr(0x12345678u, 0)).toBe(0x12345678u);
    });

    it("should rotate right u32", {
      expect(GameAK::Bits::rotr(0x1u, 1)).toBe(0x80000000u);
      expect(GameAK::Bits::rotr(0x12345678u, 4)).toBe(0x81234567u);
    });

    it("should rotate right u64", {
      expect(GameAK::Bits::rotr(0x1ull, 1)).toBe(0x8000000000000000ull);
    });

    it("[Property] rotl(x, n) == rotr(x, 32-n) for u32", {
      expect(GameAK::Bits::rotl(0x12345678u, 8)).toBe(GameAK::Bits::rotr(0x12345678u, 24));
    });
  });

  describe("GameAK::Bits::log2_floor", {
    it("should return correct values for powers of two", {
      expect(GameAK::Bits::log2_floor(1u)).toBe(0u);
      expect(GameAK::Bits::log2_floor(2u)).toBe(1u);
      expect(GameAK::Bits::log2_floor(4u)).toBe(2u);
      expect(GameAK::Bits::log2_floor(1024u)).toBe(10u);
      expect(GameAK::Bits::log2_floor(2147483648u)).toBe(31u);
    });

    it("should floor for non-powers of two", {
      expect(GameAK::Bits::log2_floor(3u)).toBe(1u);
      expect(GameAK::Bits::log2_floor(7u)).toBe(2u);
      expect(GameAK::Bits::log2_floor(15u)).toBe(3u);
    });

    it("should work for u64", {
      expect(GameAK::Bits::log2_floor(0x100000000ull)).toBe(32u);
    });
  });

  describe("GameAK::Bits::bit_width", {
    it("should return 0 for zero", {
      expect(GameAK::Bits::bit_width(0u)).toBe(0u);
    });

    it("should return correct bit widths", {
      expect(GameAK::Bits::bit_width(1u)).toBe(1u);
      expect(GameAK::Bits::bit_width(2u)).toBe(2u);
      expect(GameAK::Bits::bit_width(3u)).toBe(2u);
      expect(GameAK::Bits::bit_width(4u)).toBe(3u);
      expect(GameAK::Bits::bit_width(255u)).toBe(8u);
      expect(GameAK::Bits::bit_width(256u)).toBe(9u);
    });

    it("should work for u64", {
      expect(GameAK::Bits::bit_width(0xFFFFFFFFFFFFFFFFull)).toBe(64u);
    });
  });

  describe("GameAK::Bits::bit_ceil", {
    it("should return 1 for zero", {
      expect(GameAK::Bits::bit_ceil(0u)).toBe(1u);
    });

    it("should round up powers of two", {
      expect(GameAK::Bits::bit_ceil(1u)).toBe(1u);
      expect(GameAK::Bits::bit_ceil(2u)).toBe(2u);
      expect(GameAK::Bits::bit_ceil(4u)).toBe(4u);
    });

    it("should round up non-powers of two", {
      expect(GameAK::Bits::bit_ceil(3u)).toBe(4u);
      expect(GameAK::Bits::bit_ceil(5u)).toBe(8u);
      expect(GameAK::Bits::bit_ceil(1023u)).toBe(1024u);
    });
  });

  describe("GameAK::Bits::bit_floor", {
    it("should return 0 for zero", {
      expect(GameAK::Bits::bit_floor(0u)).toBe(0u);
    });

    it("should return same value for powers of two", {
      expect(GameAK::Bits::bit_floor(1u)).toBe(1u);
      expect(GameAK::Bits::bit_floor(2u)).toBe(2u);
      expect(GameAK::Bits::bit_floor(1024u)).toBe(1024u);
    });

    it("should round down non-powers of two", {
      expect(GameAK::Bits::bit_floor(3u)).toBe(2u);
      expect(GameAK::Bits::bit_floor(7u)).toBe(4u);
      expect(GameAK::Bits::bit_floor(1023u)).toBe(512u);
    });
  });

  describe("GameAK::Bits::extract_bits", {
    it("should extract low bits", {
      expect(GameAK::Bits::extract_bits(0xABu, 0u, 4u)).toBe(0xBu);
      expect(GameAK::Bits::extract_bits(0xABu, 4u, 4u)).toBe(0xAu);
    });

    it("should extract zero bits", {
      expect(GameAK::Bits::extract_bits(0xFFFFu, 0u, 0u)).toBe(0u);
    });

    it("should extract all bits", {
      expect(GameAK::Bits::extract_bits(0xDEADBEEFu, 0u, 32u)).toBe(0xDEADBEEFu);
    });

    it("[Property] extract_bits(x, off, 1) == ((x >> off) & 1)", {
      for (GameAK::u32 i = 0; i < 32; ++i) {
        GameAK::u32 v = 0xAAAAAAAAu;
        expect(GameAK::Bits::extract_bits(v, i, 1u)).toBe((v >> i) & 1u);
      }
    });
  });

  describe("GameAK::Bits::insert_bits", {
    it("should insert into low bits", {
      expect(GameAK::Bits::insert_bits(0xFF00u, 0xABu, 0u, 8u)).toBe(0xFFABu);
    });

    it("should insert at offset", {
      expect(GameAK::Bits::insert_bits(0x00FFu, 0xABu, 8u, 8u)).toBe(0xABFFu);
    });

    it("should insert zero bits", {
      expect(GameAK::Bits::insert_bits(0xFFFFu, 0xFFu, 0u, 0u)).toBe(0xFFFFu);
    });

    it("[Property] insert_bits(x, 0, off, count) clears the range", {
      expect(GameAK::Bits::insert_bits(0xFFFFu, 0u, 4u, 4u)).toBe(0xFF0Fu);
    });
  });

  describe("GameAK::Bits::reverse_bits", {
    it("should reverse u8", {
      expect(GameAK::Bits::reverse_bits(GameAK::u8(0b10110010))).toBe(GameAK::u8(0b01001101));
      expect(GameAK::Bits::reverse_bits(GameAK::u8(0b00000001))).toBe(GameAK::u8(0b10000000));
      expect(GameAK::Bits::reverse_bits(GameAK::u8(0xFF))).toBe(GameAK::u8(0xFF));
    });

    it("should reverse u32", {
      expect(GameAK::Bits::reverse_bits(0x00000001u)).toBe(0x80000000u);
      expect(GameAK::Bits::reverse_bits(0xFFFF0000u)).toBe(0x0000FFFFu);
    });

    it("should reverse u64", {
      expect(GameAK::Bits::reverse_bits(0x0000000000000001ull)).toBe(0x8000000000000000ull);
    });
  });

  return cest_result();
}
