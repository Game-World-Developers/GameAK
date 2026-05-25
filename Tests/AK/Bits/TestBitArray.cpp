#include <AK/Core/Bits/BitArray.hpp>
#include <cest.h>

namespace {

template <GameAK::Bits::BitCheck Check>
static bool call_test(const GameAK::Bits::BitArray<Check>& arr, GameAK::usize bit) {
  return (arr.test)(bit);
}

} // namespace

int main() {
  describe("GameAK::Bits::BitArray<BitCheck::None>", {
    it("should set and test a single bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(5);
      expect(call_test(arr, 5)).toBeTruthy();
    });

    it("should clear a set bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(10);
      expect(call_test(arr, 10)).toBeTruthy();
      arr.clear(10);
      expect(call_test(arr, 10)).toBeFalsy();
    });

    it("should handle multiple bits across words", {
      GameAK::u64 storage[4] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> arr(storage, 4);

      arr.set(0);
      arr.set(63);
      arr.set(64);
      arr.set(127);
      arr.set(200);

      expect(call_test(arr, 0)).toBeTruthy();
      expect(call_test(arr, 63)).toBeTruthy();
      expect(call_test(arr, 64)).toBeTruthy();
      expect(call_test(arr, 127)).toBeTruthy();
      expect(call_test(arr, 200)).toBeTruthy();
    });

    it("should clear only the specified bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> arr(storage, 1);

      arr.set(0);
      arr.set(1);
      arr.set(2);
      arr.clear(1);

      expect(call_test(arr, 0)).toBeTruthy();
      expect(call_test(arr, 1)).toBeFalsy();
      expect(call_test(arr, 2)).toBeTruthy();
    });

    it("should reset all bits to zero", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(0);
      arr.set(100);
      arr.set(127);
      arr.reset();

      expect(call_test(arr, 0)).toBeFalsy();
      expect(call_test(arr, 100)).toBeFalsy();
      expect(call_test(arr, 127)).toBeFalsy();
    });

    it("should work with const reference", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> arr(storage, 1);
      arr.set(42);

      const auto& carr = arr;
      expect(call_test(carr, 42)).toBeTruthy();
    });

    it("should handle bits at word boundaries", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(63);
      arr.set(64);
      arr.clear(63);

      expect(call_test(arr, 63)).toBeFalsy();
      expect(call_test(arr, 64)).toBeTruthy();
    });
  });

  describe("GameAK::Bits::BitArray<BitCheck::Bounded>", {
    it("should set and test a bit within bounds", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 2);

      arr.set(5);
      expect(call_test(arr, 5)).toBeTruthy();
    });

    it("should clear a set bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 2);

      arr.set(10);
      expect(call_test(arr, 10)).toBeTruthy();
      arr.clear(10);
      expect(call_test(arr, 10)).toBeFalsy();
    });

    it("should handle bits across word boundaries", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 2);

      arr.set(0);
      arr.set(63);
      arr.set(64);
      arr.set(127);

      expect(call_test(arr, 0)).toBeTruthy();
      expect(call_test(arr, 63)).toBeTruthy();
      expect(call_test(arr, 64)).toBeTruthy();
      expect(call_test(arr, 127)).toBeTruthy();
    });

    it("should clear only the specified bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 1);

      arr.set(0);
      arr.set(1);
      arr.set(2);
      arr.clear(1);

      expect(call_test(arr, 0)).toBeTruthy();
      expect(call_test(arr, 1)).toBeFalsy();
      expect(call_test(arr, 2)).toBeTruthy();
    });

    it("should reset all bits to zero", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 2);

      arr.set(0);
      arr.set(100);
      arr.set(127);
      arr.reset();

      expect(call_test(arr, 0)).toBeFalsy();
      expect(call_test(arr, 100)).toBeFalsy();
      expect(call_test(arr, 127)).toBeFalsy();
    });

    it("should return false for test on out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 1);

      arr.set(200);
      expect(call_test(arr, 200)).toBeFalsy();
    });

    it("should not crash on clear of out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 1);

      arr.clear(200);
      expect(call_test(arr, 0)).toBeFalsy();
    });

    it("should not crash on set of out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 1);

      arr.set(200);
      arr.set(0);
      expect(call_test(arr, 0)).toBeTruthy();
    });

    it("should work with const reference", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> arr(storage, 1);
      arr.set(42);

      const auto& carr = arr;
      expect(call_test(carr, 42)).toBeTruthy();
    });
  });

  describe("GameAK::Bits::BitArray batch operations", {
    it("should perform AND operation", {
      GameAK::u64 storage_a[2] = {0b1100, 0b0011};
      GameAK::u64 storage_b[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> a(storage_a, 2);
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> b(storage_b, 2);

      a.and_with(b);

      expect(storage_a[0] == 0b1000).toBeTruthy();
      expect(storage_a[1] == 0b0001).toBeTruthy();
    });

    it("should perform OR operation", {
      GameAK::u64 storage_a[2] = {0b1100, 0b0011};
      GameAK::u64 storage_b[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> a(storage_a, 2);
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> b(storage_b, 2);

      a.or_with(b);

      expect(storage_a[0] == 0b1110).toBeTruthy();
      expect(storage_a[1] == 0b0111).toBeTruthy();
    });

    it("should perform XOR operation", {
      GameAK::u64 storage_a[2] = {0b1100, 0b0011};
      GameAK::u64 storage_b[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> a(storage_a, 2);
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> b(storage_b, 2);

      a.xor_with(b);

      expect(storage_a[0] == 0b0110).toBeTruthy();
      expect(storage_a[1] == 0b0110).toBeTruthy();
    });

    it("should perform NOT (negate) operation", {
      GameAK::u64 storage[2] = {0b1100, 0b0011};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> a(storage, 2);

      a.negate();

      expect(storage[0] == ~0b1100UL).toBeTruthy();
      expect(storage[1] == ~0b0011UL).toBeTruthy();
    });

    it("should compute popcount", {
      GameAK::u64 storage[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<GameAK::Bits::BitCheck::None> a(storage, 2);

      expect(a.popcount() == 4UL).toBeTruthy();
    });
  });

  return cest_result();
}
