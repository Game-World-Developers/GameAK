#include "cest.h"
#include "AK/Core/Types.hpp"

int main() {
    describe("AK::Types", {
        it("should have correct sizes for integer types", {
            expect(sizeof(AK::i8)).toBe(1);
            expect(sizeof(AK::u8)).toBe(1);
            expect(sizeof(AK::i16)).toBe(2);
            expect(sizeof(AK::u16)).toBe(2);
            expect(sizeof(AK::i32)).toBe(4);
            expect(sizeof(AK::u32)).toBe(4);
            expect(sizeof(AK::i64)).toBe(8);
            expect(sizeof(AK::u64)).toBe(8);
        });

        it("should have correct sizes for floating point types", {
            expect(sizeof(AK::f32)).toBe(4);
            expect(sizeof(AK::f64)).toBe(8);
        });

        it("should have correct sizes for size and pointer types", {
            expect(sizeof(AK::usize)).toBe(sizeof(void*));
            expect(sizeof(AK::isize)).toBe(sizeof(void*));
            expect(sizeof(AK::uptr)).toBe(sizeof(void*));
            expect(sizeof(AK::iptr)).toBe(sizeof(void*));
        });

        it("should have correct properties for byte type", {
            expect(sizeof(AK::byte)).toBe(1);
            expect(std::is_enum_v<AK::byte>).toBeTruthy();
        });

        it("should have correct values for memory constants", {
            expect(AK::KiB).toBe(1024ull);
            expect(AK::MiB).toBe(1024ull * 1024);
            expect(AK::GiB).toBe(1024ull * 1024 * 1024);
        });
    });

    return cest_result();
}
