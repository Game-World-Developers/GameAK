#include <AK/Core/Bits/BitMap.hpp>
#include <cest.h>

// cest.h defines a `test` macro that conflicts with Bitmap::test().
// We wrap the call with extra parens to prevent macro expansion.
static bool call_test(const GameAK::Bits::Bitmap &map, GameAK::usize bit) {
  return (map.test)(bit);
}

int main() {
  describe("GameAK::Bits::Bitmap", {
    it("should set and test a single bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::Bitmap map(storage, 2);

      map.set(5);
      expect(call_test(map, 5)).toBeTruthy();
    });

    it("should clear a set bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::Bitmap map(storage, 2);

      map.set(10);
      expect(call_test(map, 10)).toBeTruthy();
      map.clear(10);
      expect(call_test(map, 10)).toBeFalsy();
    });

    it("should handle multiple bits across words", {
      GameAK::u64 storage[4] = {};
      GameAK::Bits::Bitmap map(storage, 4);

      map.set(0);
      map.set(63);
      map.set(64);
      map.set(127);
      map.set(200);

      expect(call_test(map, 0)).toBeTruthy();
      expect(call_test(map, 63)).toBeTruthy();
      expect(call_test(map, 64)).toBeTruthy();
      expect(call_test(map, 127)).toBeTruthy();
      expect(call_test(map, 200)).toBeTruthy();
    });

    it("should clear only the specified bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::Bitmap map(storage, 1);

      map.set(0);
      map.set(1);
      map.set(2);
      map.clear(1);

      expect(call_test(map, 0)).toBeTruthy();
      expect(call_test(map, 1)).toBeFalsy();
      expect(call_test(map, 2)).toBeTruthy();
    });

    it("should reset all bits to zero", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::Bitmap map(storage, 2);

      map.set(0);
      map.set(100);
      map.set(127);
      map.reset();

      expect(call_test(map, 0)).toBeFalsy();
      expect(call_test(map, 100)).toBeFalsy();
      expect(call_test(map, 127)).toBeFalsy();
    });

    it("should work with const reference", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::Bitmap map(storage, 1);
      map.set(42);

      const auto &cmap = map;
      expect(call_test(cmap, 42)).toBeTruthy();
    });

    it("should handle bits at word boundaries", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::Bitmap map(storage, 2);

      map.set(63);
      map.set(64);
      map.clear(63);

      expect(call_test(map, 63)).toBeFalsy();
      expect(call_test(map, 64)).toBeTruthy();
    });
  });

  return cest_result();
}
