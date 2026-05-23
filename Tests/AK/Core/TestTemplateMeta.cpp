#include "cest.h"
#include "AK/Core/TypeTraits.hpp"

namespace {
struct MoveTracker {
    bool moved = false;
    MoveTracker() = default;
    MoveTracker(const MoveTracker&) = delete;
    MoveTracker(MoveTracker&& other) noexcept {
        other.moved = true;
    }
};

template <typename T>
auto test_enable_if(T) -> AK::EnableIf<AK::IsIntegral<T>, bool> {
    return true;
}

template <typename T>
auto test_enable_if(T) -> AK::EnableIf<AK::IsFloatingPoint<T>, bool> {
    return false;
}
}

int main() {
    describe("AK::TemplateMeta", {
        it("should correctly move objects", {
            MoveTracker a;
            MoveTracker b = AK::Move(a);
            expect(a.moved).toBeTruthy();
        });

        it("should correctly forward objects", {
            int x = 5;
            int& lref = AK::Forward<int&>(x);
            expect(&x).toBe(&lref);

            int&& rref = AK::Forward<int>(x);
            expect(&x).toBe(&rref);
        });

        it("should correctly handle EnableIf (SFINAE)", {
            expect(test_enable_if(10)).toBeTruthy();
            expect(test_enable_if(10.5f)).toBeFalsy();
        });
    });

    return cest_result();
}
