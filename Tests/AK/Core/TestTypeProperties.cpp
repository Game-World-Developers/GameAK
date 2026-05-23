#include "cest.h"
#include "AK/Core/TypeTraits.hpp"

int main() {
    describe("AK::TypeProperties", {
        it("should correctly identify integral types", {
            expect(AK::IsIntegral<int>).toBeTruthy();
            expect(AK::IsIntegral<char>).toBeTruthy();
            expect(AK::IsIntegral<long long>).toBeTruthy();
            expect(AK::IsIntegral<float>).toBeFalsy();
            expect(AK::IsIntegral<int*>).toBeFalsy();
        });

        it("should correctly identify floating point types", {
            expect(AK::IsFloatingPoint<float>).toBeTruthy();
            expect(AK::IsFloatingPoint<double>).toBeTruthy();
            expect(AK::IsFloatingPoint<int>).toBeFalsy();
        });

        it("should correctly identify pointers", {
            expect(AK::IsPointer<int*>).toBeTruthy();
            expect(AK::IsPointer<void*>).toBeTruthy();
            expect(AK::IsPointer<int>).toBeFalsy();
            expect(AK::IsPointer<int&>).toBeFalsy();
        });

        it("should correctly identify references", {
            expect(AK::IsReference<int&>).toBeTruthy();
            expect(AK::IsReference<int&&>).toBeTruthy();
            expect(AK::IsReference<int>).toBeFalsy();
        });

        it("should correctly identify trivially copyable types", {
            expect(AK::IsTriviallyCopyable<int>).toBeTruthy();
            struct Simple { int x; };
            expect(AK::IsTriviallyCopyable<Simple>).toBeTruthy();
        });
    });

    return cest_result();
}
