#include <AK/Core/Bits/BitArray.hpp>
#include <cest.h>

namespace {

template <GameAK::Bits::BitCheck Check>
static bool call_test(const GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, Check> &arr,
                      GameAK::usize bit) {
  return (arr.test)(bit);
}

} // namespace

int main() {
  describe("GameAK::Bits::BitArray<..., BitCheck::None>", {
    it("should set and test a single bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(5);
      expect(call_test(arr, 5)).toBeTruthy();
    });

    it("should clear a set bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(10);
      expect(call_test(arr, 10)).toBeTruthy();
      arr.clear(10);
      expect(call_test(arr, 10)).toBeFalsy();
    });

    it("should handle multiple bits across words", {
      GameAK::u64 storage[4] = {};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 4);

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
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 1);

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
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(0);
      arr.set(100);
      arr.set(127);
      arr.reset();

      expect(call_test(arr, 0)).toBeFalsy();
      expect(call_test(arr, 100)).toBeFalsy();
      expect(call_test(arr, 127)).toBeFalsy();
    });

    it("should handle bits at word boundaries", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 2);

      arr.set(63);
      arr.set(64);
      arr.clear(63);

      expect(call_test(arr, 63)).toBeFalsy();
      expect(call_test(arr, 64)).toBeTruthy();
    });

    it("[Property] set(x) => test(x) == true for all bits in range", {
      GameAK::u64 storage[4] = {};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 4);

      for (GameAK::usize i = 0; i < 256; ++i) {
        arr.set(i);
        expect(call_test(arr, i)).toBeTruthy();
      }
    });

    it("[Property] clear(x) => test(x) == false after set(x)", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 2);

      for (GameAK::usize i = 0; i < 128; ++i) {
        arr.set(i);
        arr.clear(i);
        expect(call_test(arr, i)).toBeFalsy();
      }
    });

    it("should work with const reference", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::None> arr(storage, 1);
      arr.set(42);

      const auto &carr = arr;
      expect(call_test(carr, 42)).toBeTruthy();
    });
  });

  describe("GameAK::Bits::BitArray<..., BitCheck::Bounded>", {
    using BoundedArr = GameAK::Bits::BitArray<GameAK::Backend::ScalarBuiltinsBackend, GameAK::Bits::BitCheck::Bounded>;

    it("should set and test a bit within bounds", {
      GameAK::u64 storage[2] = {};
      BoundedArr arr(storage, 2);

      arr.set(5);
      expect(call_test(arr, 5)).toBeTruthy();
    });

    it("should clear a set bit", {
      GameAK::u64 storage[2] = {};
      BoundedArr arr(storage, 2);

      arr.set(10);
      expect(call_test(arr, 10)).toBeTruthy();
      arr.clear(10);
      expect(call_test(arr, 10)).toBeFalsy();
    });

    it("should handle bits across word boundaries", {
      GameAK::u64 storage[2] = {};
      BoundedArr arr(storage, 2);

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
      BoundedArr arr(storage, 1);

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
      BoundedArr arr(storage, 2);

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
      BoundedArr arr(storage, 1);

      arr.set(200);
      expect(call_test(arr, 200)).toBeFalsy();
    });

    it("should not crash on clear of out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      BoundedArr arr(storage, 1);

      arr.clear(200);
      expect(call_test(arr, 0)).toBeFalsy();
    });

    it("should not crash on set of out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      BoundedArr arr(storage, 1);

      arr.set(200);
      arr.set(0);
      expect(call_test(arr, 0)).toBeTruthy();
    });
  });

  describe("GameAK::Bits::BitArray queries (default backend)", {
    it("any() should return false for empty array", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      expect(arr.any()).toBeFalsy();
    });

    it("any() should return true when a bit is set", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      arr.set(42);
      expect(arr.any()).toBeTruthy();
    });

    it("none() should return true for empty array", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      expect(arr.none()).toBeTruthy();
    });

    it("none() should return false when a bit is set", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      arr.set(0);
      expect(arr.none()).toBeFalsy();
    });

    it("all() should return false for empty array", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      expect(arr.all()).toBeFalsy();
    });

    it("all() should return true when all bits are set", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      storage[0] = ~0ull;
      storage[1] = ~0ull;
      expect(arr.all()).toBeTruthy();
    });

    it("all() should return false when only one word is full", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      storage[0] = ~0ull;
      storage[1] = 0;
      expect(arr.all()).toBeFalsy();
    });
  });

  describe("GameAK::Bits::BitArray find_first_set", {
    it("should return sentinel for empty array", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      expect(arr.find_first_set()).toBe(128u);
    });

    it("should find first set bit in first word", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      arr.set(3);
      expect(arr.find_first_set()).toBe(3u);
    });

    it("should find first set bit in second word", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      arr.set(100);
      expect(arr.find_first_set()).toBe(100u);
    });

    it("find_next_set should iterate all set bits", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);
      arr.set(5);
      arr.set(10);
      arr.set(100);

      GameAK::usize bit = arr.find_first_set();
      expect(bit).toBe(5u);
      bit = arr.find_next_set(bit);
      expect(bit).toBe(10u);
      bit = arr.find_next_set(bit);
      expect(bit).toBe(100u);
      bit = arr.find_next_set(bit);
      expect(bit).toBe(128u);
    });
  });

  describe("GameAK::Bits::BitArray range operations", {
    it("set_range should set bits within a single word", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitArray<> arr(storage, 1);

      arr.set_range(2, 5);
      expect((arr.test)(1)).toBeFalsy();
      expect((arr.test)(2)).toBeTruthy();
      expect((arr.test)(3)).toBeTruthy();
      expect((arr.test)(4)).toBeTruthy();
      expect((arr.test)(5)).toBeTruthy();
      expect((arr.test)(6)).toBeFalsy();
      expect(storage[0] == 0b111100u).toBeTruthy();
    });

    it("set_range should set bits across words", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitArray<> arr(storage, 2);

      arr.set_range(60, 68);
      expect((arr.test)(59)).toBeFalsy();
      expect((arr.test)(60)).toBeTruthy();
      expect((arr.test)(63)).toBeTruthy();
      expect((arr.test)(64)).toBeTruthy();
      expect((arr.test)(68)).toBeTruthy();
      expect((arr.test)(69)).toBeFalsy();
    });

    it("clear_range should clear bits within a single word", {
      GameAK::u64 storage[1] = {0b11111111};
      GameAK::Bits::BitArray<> arr(storage, 1);

      arr.clear_range(2, 5);
      expect((arr.test)(2)).toBeFalsy();
      expect((arr.test)(3)).toBeFalsy();
      expect((arr.test)(4)).toBeFalsy();
      expect((arr.test)(5)).toBeFalsy();
      expect((arr.test)(0)).toBeTruthy();
      expect((arr.test)(1)).toBeTruthy();
      expect((arr.test)(6)).toBeTruthy();
      expect((arr.test)(7)).toBeTruthy();
    });

    it("clear_range should clear bits across words", {
      GameAK::u64 storage[4] = {~0ull, ~0ull, ~0ull, ~0ull};
      GameAK::Bits::BitArray<> arr(storage, 4);

      arr.clear_range(60, 130);
      expect((arr.test)(59)).toBeTruthy();
      expect((arr.test)(60)).toBeFalsy();
      expect((arr.test)(130)).toBeFalsy();
      expect((arr.test)(131)).toBeTruthy();
      expect(storage[0] == 0x0FFFFFFFFFFFFFFFull).toBeTruthy();
      expect(storage[1] == 0ull).toBeTruthy();
      expect(storage[2] == 0xFFFFFFFFFFFFFFF8ull).toBeTruthy();
    });
  });

  describe("GameAK::Bits::BitArray batch operations", {
    it("should perform AND operation", {
      GameAK::u64 storage_a[2] = {0b1100, 0b0011};
      GameAK::u64 storage_b[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<> a(storage_a, 2);
      GameAK::Bits::BitArray<> b(storage_b, 2);

      a.and_with(b);

      expect(storage_a[0] == 0b1000).toBeTruthy();
      expect(storage_a[1] == 0b0001).toBeTruthy();
    });

    it("should perform OR operation", {
      GameAK::u64 storage_a[2] = {0b1100, 0b0011};
      GameAK::u64 storage_b[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<> a(storage_a, 2);
      GameAK::Bits::BitArray<> b(storage_b, 2);

      a.or_with(b);

      expect(storage_a[0] == 0b1110).toBeTruthy();
      expect(storage_a[1] == 0b0111).toBeTruthy();
    });

    it("should perform XOR operation", {
      GameAK::u64 storage_a[2] = {0b1100, 0b0011};
      GameAK::u64 storage_b[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<> a(storage_a, 2);
      GameAK::Bits::BitArray<> b(storage_b, 2);

      a.xor_with(b);

      expect(storage_a[0] == 0b0110).toBeTruthy();
      expect(storage_a[1] == 0b0110).toBeTruthy();
    });

    it("should perform NOT (negate) operation", {
      GameAK::u64 storage[2] = {0b1100, 0b0011};
      GameAK::Bits::BitArray<> a(storage, 2);

      a.negate();

      expect(storage[0] == ~0b1100UL).toBeTruthy();
      expect(storage[1] == ~0b0011UL).toBeTruthy();
    });

    it("should compute popcount", {
      GameAK::u64 storage[2] = {0b1010, 0b0101};
      GameAK::Bits::BitArray<> a(storage, 2);

      expect(a.popcount() == 4UL).toBeTruthy();
    });

    it("[Property] popcount equals number of set bits", {
      GameAK::u64 storage[4] = {};
      GameAK::Bits::BitArray<> a(storage, 4);

      for (GameAK::usize i = 0; i < 256; i += 2) {
        a.set(i);
      }
      expect(a.popcount() == 128UL).toBeTruthy();
    });
  });

  return cest_result();
}
