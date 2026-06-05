#include <AK/Core/Bits/BitArray.hpp>
#include <AK/Core/Bits/BitSetIterator.hpp>
#include <cest.h>

int main() {
  describe("GameAK::Bits::BitSetIterator", {
    it("should return sentinel for empty array", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitSetIterator iter(storage, 2);
      expect(iter.next()).toBe(128u);
    });

    it("should iterate set bits in order", {
      GameAK::u64 storage[2] = {};
      storage[0] = (1ull << 3) | (1ull << 10);
      storage[1] = (1ull << 1);
      GameAK::Bits::BitSetIterator iter(storage, 2);

      expect(iter.next()).toBe(3u);
      expect(iter.next()).toBe(10u);
      expect(iter.next()).toBe(64u + 1u); // bit 65
      expect(iter.next()).toBe(128u);
    });

    it("should iterate single set bit", {
      GameAK::u64 storage[1] = {1ull << 63};
      GameAK::Bits::BitSetIterator iter(storage, 1);

      expect(iter.next()).toBe(63u);
      expect(iter.next()).toBe(64u);
    });

    it("should support reset", {
      GameAK::u64 storage[1] = {(1ull << 5) | (1ull << 20)};
      GameAK::Bits::BitSetIterator iter(storage, 1);

      expect(iter.next()).toBe(5u);
      iter.reset();
      expect(iter.next()).toBe(5u);
    });

    it("should iterate all set bits in dense array", {
      GameAK::u64 storage[2] = {~0ull, ~0ull};
      GameAK::Bits::BitSetIterator iter(storage, 2);

      for (GameAK::usize i = 0; i < 128; ++i) {
        expect(iter.next()).toBe(i);
      }
      expect(iter.next()).toBe(128u);
    });

    it("should handle single-word array", {
      GameAK::u64 storage[1] = {(1ull << 0) | (1ull << 63)};
      GameAK::Bits::BitSetIterator iter(storage, 1);

      expect(iter.next()).toBe(0u);
      expect(iter.next()).toBe(63u);
      expect(iter.next()).toBe(64u);
    });

    it("[Stress] iterate 1000 bits across 16 words", {
      GameAK::u64 storage[16] = {};
      for (GameAK::usize i = 0; i < 1000; ++i) {
        storage[i / 64] |= (1ull << (i % 64));
      }
      GameAK::Bits::BitSetIterator iter(storage, 16);

      for (GameAK::usize i = 0; i < 1000; ++i) {
        expect(iter.next()).toBe(i);
      }
      expect(iter.next()).toBe(1024u);
    });
  });

  return cest_result();
}
