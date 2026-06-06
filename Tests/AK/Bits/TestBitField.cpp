#include <AK/Core/Bits/BitField.hpp>
#include <cest.h>

int main() {
  describe("GameAK::Bits::BitField<u32, 0, 8>", {
    using BF = GameAK::Bits::BitField<GameAK::u32, 0, 8>;

    it("should extract low byte",
       { expect(BF::extract(0x12345678u)).toBe(0x78u); });

    it("should insert low byte",
       { expect(BF::insert(0x12345600u, 0xABu)).toBe(0x123456ABu); });
  });

  describe("GameAK::Bits::BitField<u32, 8, 8>", {
    using BF = GameAK::Bits::BitField<GameAK::u32, 8, 8>;

    it("should extract second byte",
       { expect(BF::extract(0x12345678u)).toBe(0x56u); });

    it("should insert second byte",
       { expect(BF::insert(0x12340078u, 0xABu)).toBe(0x1234AB78u); });
  });

  describe("GameAK::Bits::BitField<u32, 0, 32>", {
    using BF = GameAK::Bits::BitField<GameAK::u32, 0, 32>;

    it("should extract full width",
       { expect(BF::extract(0xDEADBEEFu)).toBe(0xDEADBEEFu); });

    it("should insert full width",
       { expect(BF::insert(0x00000000u, 0xCAFEBABEu)).toBe(0xCAFEBABEu); });
  });

  describe("GameAK::Bits::BitField<u64, 0, 1>", {
    using BF = GameAK::Bits::BitField<GameAK::u64, 0, 1>;

    it("should extract single bit", {
      expect(BF::extract(0x1ull)).toBe(1u);
      expect(BF::extract(0x2ull)).toBe(0u);
    });

    it("should insert single bit", {
      expect(BF::insert(0x0ull, 1u)).toBe(0x1ull);
      expect(BF::insert(0x2ull, 0u)).toBe(0x2ull);
    });
  });

  describe("GameAK::Bits::BitField<u16, 3, 5>", {
    using BF = GameAK::Bits::BitField<GameAK::u16, 3, 5>;

    it("should extract middle bits", {
      // 0xAD3C, bits 3-7 (bit 3 = LSB) = 00111 = 7
      expect(BF::extract(GameAK::u16(0xAD3Cu))).toBe(GameAK::u16(7));
    });

    it("should insert middle bits", {
      // Clear bits 3-7 of 0xFFFF, insert 0b10101 = 21 at pos 3
      // result = 0b1111_1111_1010_1111 = 0xFFAF
      expect(BF::insert(GameAK::u16(0xFFFFu), GameAK::u16(21)))
          .toBe(GameAK::u16(0xFFAFu));
    });
  });

  describe("GameAK::Bits::BitField<u8, 2, 6>", {
    using BF = GameAK::Bits::BitField<GameAK::u8, 2, 6>;

    it("should extract top 6 bits of a byte", {
      // 0b11011001, bits 2-7 = 0b110110 = 54
      expect(BF::extract(GameAK::u8(0xD9u))).toBe(GameAK::u8(54));
    });

    it("should insert into top 6 bits", {
      // 0x00 with 0b101010 at pos 2 = 0b10101000 = 0xA8
      expect(BF::insert(GameAK::u8(0x00u), GameAK::u8(42)))
          .toBe(GameAK::u8(0xA8u));
    });
  });

  return cest_result();
}
