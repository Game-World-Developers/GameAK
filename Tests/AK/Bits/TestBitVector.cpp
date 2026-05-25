#include <AK/Core/Bits/BitVector.hpp>
#include <cest.h>

// cest.h defines a `test` macro that conflicts with BitVector::test().
// We wrap the call with extra parens to prevent macro expansion.
static bool call_test(const GameAK::Bits::BitVector &vec, GameAK::usize bit) {
  return (vec.test)(bit);
}

int main() {
  describe("GameAK::Bits::BitVector", {
    it("should set and test a bit within bounds", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitVector vec(storage, 2);

      vec.set(5);
      expect(call_test(vec, 5)).toBeTruthy();
    });

    it("should clear a set bit", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitVector vec(storage, 2);

      vec.set(10);
      expect(call_test(vec, 10)).toBeTruthy();
      vec.clear(10);
      expect(call_test(vec, 10)).toBeFalsy();
    });

    it("should handle bits across word boundaries", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitVector vec(storage, 2);

      vec.set(0);
      vec.set(63);
      vec.set(64);
      vec.set(127);

      expect(call_test(vec, 0)).toBeTruthy();
      expect(call_test(vec, 63)).toBeTruthy();
      expect(call_test(vec, 64)).toBeTruthy();
      expect(call_test(vec, 127)).toBeTruthy();
    });

    it("should clear only the specified bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitVector vec(storage, 1);

      vec.set(0);
      vec.set(1);
      vec.set(2);
      vec.clear(1);

      expect(call_test(vec, 0)).toBeTruthy();
      expect(call_test(vec, 1)).toBeFalsy();
      expect(call_test(vec, 2)).toBeTruthy();
    });

    it("should reset all bits to zero", {
      GameAK::u64 storage[2] = {};
      GameAK::Bits::BitVector vec(storage, 2);

      vec.set(0);
      vec.set(100);
      vec.set(127);
      vec.reset();

      expect(call_test(vec, 0)).toBeFalsy();
      expect(call_test(vec, 100)).toBeFalsy();
      expect(call_test(vec, 127)).toBeFalsy();
    });

    it("should return false for test on out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitVector vec(storage, 1);

      vec.set(200);
      expect(call_test(vec, 200)).toBeFalsy();
    });

    it("should not crash on clear of out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitVector vec(storage, 1);

      vec.clear(200);
      expect(call_test(vec, 0)).toBeFalsy();
    });

    it("should not crash on set of out-of-bounds bit", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitVector vec(storage, 1);

      vec.set(200);
      vec.set(0);
      expect(call_test(vec, 0)).toBeTruthy();
    });

    it("should work with const reference", {
      GameAK::u64 storage[1] = {};
      GameAK::Bits::BitVector vec(storage, 1);
      vec.set(42);

      const auto &cvec = vec;
      expect(call_test(cvec, 42)).toBeTruthy();
    });
  });

  return cest_result();
}
