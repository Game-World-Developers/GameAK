#include <AK/Core/Types.hpp>
#include <cest.h>
#include <limits>
#include <type_traits>

int main() {
  describe("GameAK::NumericLimits", {
    it("should have correct limits for signed integers", {
      expect(std::numeric_limits<GameAK::i8>::min()).toBe(-128);
      expect(std::numeric_limits<GameAK::i8>::max()).toBe(127);
      expect(std::numeric_limits<GameAK::i16>::min()).toBe(-32768);
      expect(std::numeric_limits<GameAK::i16>::max()).toBe(32767);
      expect(std::numeric_limits<GameAK::i32>::min()).toBe(-2147483648ll);
      expect(std::numeric_limits<GameAK::i32>::max()).toBe(2147483647);
    });

    it("should have correct limits for unsigned integers", {
      expect(std::numeric_limits<GameAK::u8>::max()).toBe(255);
      expect(std::numeric_limits<GameAK::u16>::max()).toBe(65535);
      expect(std::numeric_limits<GameAK::u32>::max()).toBe(4294967295ull);
    });

    it("should have correct limits for i64 and u64", {
      expect(std::numeric_limits<GameAK::i64>::max())
          .toBe(9223372036854775807ll);
      expect(std::numeric_limits<GameAK::u64>::max())
          .toBe(18446744073709551615ull);
    });

    it("should have correct limits for floating point types", {
      expect(std::numeric_limits<GameAK::f32>::is_iec559).toBeTruthy();
      expect(std::numeric_limits<GameAK::f64>::is_iec559).toBeTruthy();
    });

    it("should correctly identify signedness", {
      expect(std::is_signed_v<GameAK::i8>).toBeTruthy();
      expect(std::is_signed_v<GameAK::i16>).toBeTruthy();
      expect(std::is_signed_v<GameAK::i32>).toBeTruthy();
      expect(std::is_signed_v<GameAK::i64>).toBeTruthy();

      expect(std::is_unsigned_v<GameAK::u8>).toBeTruthy();
      expect(std::is_unsigned_v<GameAK::u16>).toBeTruthy();
      expect(std::is_unsigned_v<GameAK::u32>).toBeTruthy();
      expect(std::is_unsigned_v<GameAK::u64>).toBeTruthy();
    });

    it("[Invariant] signed min is negative for all signed types", {
      expect(std::numeric_limits<GameAK::i8>::min() < 0).toBeTruthy();
      expect(std::numeric_limits<GameAK::i16>::min() < 0).toBeTruthy();
      expect(std::numeric_limits<GameAK::i32>::min() < 0).toBeTruthy();
      expect(std::numeric_limits<GameAK::i64>::min() < 0).toBeTruthy();
    });

    it("[Invariant] unsigned min is zero for all unsigned types", {
      expect(std::numeric_limits<GameAK::u8>::min()).toBe(0);
      expect(std::numeric_limits<GameAK::u16>::min()).toBe(0);
      expect(std::numeric_limits<GameAK::u32>::min()).toBe(0);
      expect(std::numeric_limits<GameAK::u64>::min()).toBe(0);
    });
  });

  return cest_result();
}
