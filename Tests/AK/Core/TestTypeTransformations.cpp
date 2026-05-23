#include "cest.h"
#include "AK/Core/TypeTraits.hpp"
#include <type_traits>

int main() {
    describe("AK::TypeTransformations", {
        it("should correctly remove references", {
            static_assert(std::is_same_v<AK::RemoveRef<int&>, int>);
            static_assert(std::is_same_v<AK::RemoveRef<int&&>, int>);
            static_assert(std::is_same_v<AK::RemoveRef<int>, int>);
            expect(true).toBeTruthy(); // Placeholder for runtime feedback
        });

        it("should correctly remove const", {
            static_assert(std::is_same_v<AK::RemoveConst<const int>, int>);
            static_assert(std::is_same_v<AK::RemoveConst<int>, int>);
            expect(true).toBeTruthy();
        });

        it("should correctly remove CV qualifiers", {
            static_assert(std::is_same_v<AK::RemoveCV<const volatile int>, int>);
            static_assert(std::is_same_v<AK::RemoveCV<const int>, int>);
            static_assert(std::is_same_v<AK::RemoveCV<volatile int>, int>);
            expect(true).toBeTruthy();
        });

        it("should correctly remove CV and references", {
            static_assert(std::is_same_v<AK::RemoveCVRef<const int&>, int>);
            static_assert(std::is_same_v<AK::RemoveCVRef<volatile int&&>, int>);
            expect(true).toBeTruthy();
        });

        it("should correctly add const", {
            static_assert(std::is_same_v<AK::AddConst<int>, const int>);
            static_assert(std::is_same_v<AK::AddConst<const int>, const int>);
            expect(true).toBeTruthy();
        });

        it("should correctly add references", {
            static_assert(std::is_same_v<AK::AddLValueRef<int>, int&>);
            static_assert(std::is_same_v<AK::AddRValueRef<int>, int&&>);
            expect(true).toBeTruthy();
        });

        it("should correctly decay types", {
            static_assert(std::is_same_v<AK::Decay<int[10]>, int*>);
            static_assert(std::is_same_v<AK::Decay<int(int)>, int(*)(int)>);
            static_assert(std::is_same_v<AK::Decay<const int&>, int>);
            expect(true).toBeTruthy();
        });
    });

    return cest_result();
}
