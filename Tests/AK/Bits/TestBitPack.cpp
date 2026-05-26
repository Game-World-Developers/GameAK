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

    it("[Property] read after write reproduces original values", {
      GameAK::u8 buffer[32] = {};
      GameAK::Bits::BitPack pack(buffer);

      GameAK::u64 vals[] = {0x5A, 0x1A, 0x2EF, 0x5};
      GameAK::usize bits[] = {7, 5, 10, 3};

      for (int i = 0; i < 4; ++i) {
        pack.write(vals[i], GameAK::Bits::BitCount(bits[i]));
      }

      pack.reset();
      for (int i = 0; i < 4; ++i) {
        GameAK::u64 r = pack.read(GameAK::Bits::BitCount(bits[i]));
        expect(r).toBe(vals[i]);
      }
    });

    it("[Property] read after reset restores original stream", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      pack.write(0xDE, GameAK::Bits::BitCount(8));
      pack.write(0xAD, GameAK::Bits::BitCount(8));

      pack.reset();
      expect(pack.read(GameAK::Bits::BitCount(4))).toBe(0xEu);
      pack.reset();
      expect(pack.bit_offset()).toBe(0u);

      // Re-read entire stream
      expect(pack.read(GameAK::Bits::BitCount(8))).toBe(0xDEu);
      expect(pack.read(GameAK::Bits::BitCount(8))).toBe(0xADu);
    });

    it("[Edge: 0-bit write and read]", {
      GameAK::u8 buffer[16] = {};
      GameAK::Bits::BitPack pack(buffer);

      pack.write(0xFF, GameAK::Bits::BitCount(0));
      expect(pack.bit_offset()).toBe(0u);

      pack.write(42, GameAK::Bits::BitCount(8));
      pack.reset();
      GameAK::u64 r = pack.read(GameAK::Bits::BitCount(0));
      expect(r).toBe(0u);
      expect(pack.read(GameAK::Bits::BitCount(8))).toBe(42u);
    });

    it("[Stress: sequential write/read round-trip]", {
      GameAK::u8 buffer[128] = {};
      GameAK::Bits::BitPack pack(buffer);

      for (int i = 0; i < 100; ++i) {
        pack.write(i % 256, GameAK::Bits::BitCount(8));
      }
      pack.reset();
      for (int i = 0; i < 100; ++i) {
        GameAK::u64 r = pack.read(GameAK::Bits::BitCount(8));
        expect(r).toBe(static_cast<GameAK::u64>(i % 256));
      }
    });
  });

  return cest_result();
}
