#include <AK/Core/Bits/BitPack.hpp>
#include <cest.h>

int main() {
  describe("GameAK::Bits::BitPack", {
    it("should write and read a single value", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      pack.write(42, GameAK::Bits::BitCount(8));
      pack.reset();
      GameAK::u64 result = pack.read(GameAK::Bits::BitCount(8));
      expect(result).toBe(42u);
    });

    it("should write and read across byte boundaries", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      pack.write(0xAA, GameAK::Bits::BitCount(8));
      pack.write(0xBB, GameAK::Bits::BitCount(8));
      pack.reset();
      expect(pack.read(GameAK::Bits::BitCount(8))).toBe(0xAAu);
      expect(pack.read(GameAK::Bits::BitCount(8))).toBe(0xBBu);
    });

    it("should write and read non-byte-aligned values", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      pack.write(7, GameAK::Bits::BitCount(3));
      pack.write(3, GameAK::Bits::BitCount(2));

      pack.reset();
      expect(pack.read(GameAK::Bits::BitCount(3))).toBe(7u);
      expect(pack.read(GameAK::Bits::BitCount(2))).toBe(3u);
    });

    it("should handle large values (up to 64 bits)", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      GameAK::u64 value = 0xDEADBEEFCAFEBABEull;
      pack.write(value, GameAK::Bits::BitCount(64));
      pack.reset();
      GameAK::u64 result = pack.read(GameAK::Bits::BitCount(64));
      expect(result).toBe(value);
    });

    it("should track bit_offset correctly", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      expect(pack.bit_offset()).toBe(0u);
      pack.write(1, GameAK::Bits::BitCount(3));
      expect(pack.bit_offset()).toBe(3u);
      pack.write(2, GameAK::Bits::BitCount(5));
      expect(pack.bit_offset()).toBe(8u);
    });

    it("should reset bit_offset to zero", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      pack.write(255, GameAK::Bits::BitCount(8));
      expect(pack.bit_offset()).toBe(8u);
      pack.reset();
      expect(pack.bit_offset()).toBe(0u);
    });

    it("should handle multiple small writes followed by reads", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      pack.write(1, GameAK::Bits::BitCount(1));
      pack.write(0, GameAK::Bits::BitCount(1));
      pack.write(1, GameAK::Bits::BitCount(1));
      pack.write(1, GameAK::Bits::BitCount(1));

      pack.reset();
      expect(pack.read(GameAK::Bits::BitCount(1))).toBe(1u);
      expect(pack.read(GameAK::Bits::BitCount(1))).toBe(0u);
      expect(pack.read(GameAK::Bits::BitCount(1))).toBe(1u);
      expect(pack.read(GameAK::Bits::BitCount(1))).toBe(1u);
    });
  });

  return cest_result();
}
