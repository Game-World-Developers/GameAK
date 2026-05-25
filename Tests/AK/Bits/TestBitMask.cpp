#include <AK/Core/Bits/BitMask.hpp>
#include <cest.h>

enum class TestFlag : GameAK::u64 {
  None   = 0,
  Read   = 0,
  Write  = 1,
  Exec   = 2,
  Four   = 3,
};

int main() {
  describe("GameAK::Bits::BitMask", {
    it("should be empty by default", {
      GameAK::Bits::BitMask<TestFlag> mask;
      expect(mask.has(TestFlag::Read)).toBeFalsy();
      expect(mask.has(TestFlag::Write)).toBeFalsy();
      expect(mask.raw()).toBe(0u);
    });

    it("should construct with a single flag", {
      GameAK::Bits::BitMask<TestFlag> mask(TestFlag::Write);
      expect(mask.has(TestFlag::Write)).toBeTruthy();
      expect(mask.has(TestFlag::Read)).toBeFalsy();
      expect(mask.has(TestFlag::Exec)).toBeFalsy();
    });

    it("should set and clear flags", {
      GameAK::Bits::BitMask<TestFlag> mask;
      mask.set(TestFlag::Read);
      mask.set(TestFlag::Exec);
      expect(mask.has(TestFlag::Read)).toBeTruthy();
      expect(mask.has(TestFlag::Exec)).toBeTruthy();
      expect(mask.has(TestFlag::Write)).toBeFalsy();

      mask.clear(TestFlag::Read);
      expect(mask.has(TestFlag::Read)).toBeFalsy();
      expect(mask.has(TestFlag::Exec)).toBeTruthy();
    });

    it("should accumulate multiple flags in raw value", {
      GameAK::Bits::BitMask<TestFlag> mask;
      mask.set(TestFlag::Write);
      mask.set(TestFlag::Exec);
      // Write = bit 1, Exec = bit 2 -> 0b110 = 6
      expect(mask.raw()).toBe(6u);
    });

    it("should support const operations", {
      const GameAK::Bits::BitMask<TestFlag> mask(TestFlag::Four);
      expect(mask.has(TestFlag::Four)).toBeTruthy();
      expect(mask.raw()).toBe(8u);
    });

    it("should copy correctly", {
      GameAK::Bits::BitMask<TestFlag> a;
      a.set(TestFlag::Read);
      a.set(TestFlag::Write);

      auto b = a;
      expect(b.has(TestFlag::Read)).toBeTruthy();
      expect(b.has(TestFlag::Write)).toBeTruthy();
    });
  });

  return cest_result();
}
