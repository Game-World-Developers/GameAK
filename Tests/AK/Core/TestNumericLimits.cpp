#include "cest.h"
#include "AK/Core/Types.hpp"
#include <limits>
#include <type_traits>

int main() {
    describe("AK::NumericLimits", {
        it("should have correct limits for signed integers", {
            expect(std::numeric_limits<AK::i8>::min()).toBe(-128);
            expect(std::numeric_limits<AK::i8>::max()).toBe(127);
            expect(std::numeric_limits<AK::i16>::min()).toBe(-32768);
            expect(std::numeric_limits<AK::i16>::max()).toBe(32767);
            expect(std::numeric_limits<AK::i32>::min()).toBe(-2147483648ll);
            expect(std::numeric_limits<AK::i32>::max()).toBe(2147483647);
        });

        it("should have correct limits for unsigned integers", {
            expect(std::numeric_limits<AK::u8>::max()).toBe(255);
            expect(std::numeric_limits<AK::u16>::max()).toBe(65535);
            expect(std::numeric_limits<AK::u32>::max()).toBe(4294967295ull);
        });

        it("should correctly identify signedness", {
            expect(std::is_signed_v<AK::i8>).toBeTruthy();
            expect(std::is_signed_v<AK::i16>).toBeTruthy();
            expect(std::is_signed_v<AK::i32>).toBeTruthy();
            expect(std::is_signed_v<AK::i64>).toBeTruthy();

            expect(std::is_unsigned_v<AK::u8>).toBeTruthy();
            expect(std::is_unsigned_v<AK::u16>).toBeTruthy();
            expect(std::is_unsigned_v<AK::u32>).toBeTruthy();
            expect(std::is_unsigned_v<AK::u64>).toBeTruthy();
        });
    });

    return cest_result();
}
