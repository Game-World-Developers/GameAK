#include <AK/Core/TypeTraits.hpp>
#include <cest.h>

namespace {
struct MoveTracker {
  bool moved = false;
  MoveTracker() = default;
  MoveTracker(const MoveTracker &) = delete;
  MoveTracker(MoveTracker &&other) noexcept { other.moved = true; }
};

template <typename T>
auto test_enable_if(T) -> GameAK::EnableIf<GameAK::IsIntegral<T>, bool> {
  return true;
}

template <typename T>
auto test_enable_if(T) -> GameAK::EnableIf<GameAK::IsFloatingPoint<T>, bool> {
  return false;
}
} // namespace

int main() {
  describe("GameAK::TemplateMeta", {
    it("should correctly move objects", {
      MoveTracker a;
      MoveTracker b = GameAK::Move(a);
      expect(a.moved).toBeTruthy();
    });

    it("should correctly forward objects", {
      int x = 5;
      int &lref = GameAK::Forward<int &>(x);
      expect(&x).toBe(&lref);

      int &&rref = GameAK::Forward<int>(x);
      expect(&x).toBe(&rref);
    });

    it("should correctly handle EnableIf (SFINAE)", {
      expect(test_enable_if(10)).toBeTruthy();
      expect(test_enable_if(10.5f)).toBeFalsy();
    });
  });

  return cest_result();
}
