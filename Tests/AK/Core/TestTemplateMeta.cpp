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

struct Base {};
struct Derived : Base {};
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

    it("[Edge] EnableIf with void default works for SFINAE", {
      auto check_int =
          [](auto x) -> GameAK::EnableIf<GameAK::IsIntegral<decltype(x)>> {
        (void)x;
      };
      check_int(42);
    });

    it("should have correct IntegralConstant value and types", {
      static_assert(GameAK::IntegralConstant<int, 42>::value == 42);
      static_assert(GameAK::IntegralConstant<bool, true>::value);
      static_assert(
          std::is_same_v<GameAK::IntegralConstant<int, 0>::value_type, int>);
      constexpr auto ic = GameAK::IntegralConstant<int, 99>{};
      static_assert(ic() == 99);
      int converted = GameAK::IntegralConstant<int, 77>{};
      expect(converted).toBe(77);
    });

    it("should have correct TrueType and FalseType", {
      static_assert(GameAK::TrueType::value);
      static_assert(!GameAK::FalseType::value);
      expect(GameAK::TrueType::value).toBeTruthy();
      expect(GameAK::FalseType::value).toBeFalsy();
    });

    it("should correctly use TypeIdentity", {
      static_assert(std::is_same_v<GameAK::TypeIdentity<int>::type, int>);
      static_assert(std::is_same_v<GameAK::TypeIdentity<float>::type, float>);
      expect((std::is_same_v<GameAK::TypeIdentity<int>::type, int>))
          .toBeTruthy();
      expect((std::is_same_v<GameAK::TypeIdentity<float>::type, float>))
          .toBeTruthy();
    });

    it("should correctly select types with Conditional", {
      static_assert(
          std::is_same_v<GameAK::Conditional<true, int, float>::type, int>);
      static_assert(
          std::is_same_v<GameAK::Conditional<false, int, float>::type, float>);
      static_assert(
          std::is_same_v<GameAK::ConditionalT<true, int, float>, int>);
      static_assert(
          std::is_same_v<GameAK::ConditionalT<false, int, float>, float>);
      expect((std::is_same_v<GameAK::Conditional<true, int, float>::type, int>))
          .toBeTruthy();
      expect(
          (std::is_same_v<GameAK::Conditional<false, int, float>::type, float>))
          .toBeTruthy();
      expect((std::is_same_v<GameAK::ConditionalT<true, int, float>, int>))
          .toBeTruthy();
      expect((std::is_same_v<GameAK::ConditionalT<false, int, float>, float>))
          .toBeTruthy();
    });

    it("should correctly check type equality with IsSame", {
      static_assert(GameAK::IsSame<int, int>);
      static_assert(!GameAK::IsSame<int, float>);
      static_assert(GameAK::IsSame<void, void>);
      expect((GameAK::IsSame<int, int>)).toBeTruthy();
      expect((GameAK::IsSame<int, float>)).toBeFalsy();
      expect((GameAK::IsSame<void, void>)).toBeTruthy();
    });

    it("should correctly check base-of relationships", {
      static_assert(GameAK::IsBaseOf<Base, Derived>);
      static_assert(GameAK::IsBaseOf<Base, Base>);
      static_assert(!GameAK::IsBaseOf<Derived, Base>);
      expect((GameAK::IsBaseOf<Base, Derived>)).toBeTruthy();
      expect((GameAK::IsBaseOf<Base, Base>)).toBeTruthy();
      expect((GameAK::IsBaseOf<Derived, Base>)).toBeFalsy();
    });

    it("should correctly check convertibility", {
      static_assert(GameAK::IsConvertible<int, float>);
      static_assert(GameAK::IsConvertible<Derived, Base>);
      static_assert(!GameAK::IsConvertible<Base, Derived>);
      static_assert(GameAK::IsConvertible<float, double>);
      expect((GameAK::IsConvertible<int, float>)).toBeTruthy();
      expect((GameAK::IsConvertible<Derived, Base>)).toBeTruthy();
      expect((GameAK::IsConvertible<Base, Derived>)).toBeFalsy();
      expect((GameAK::IsConvertible<float, double>)).toBeTruthy();
    });

    it("should correctly swap two values", {
      int a = 1, b = 2;
      GameAK::Swap(a, b);
      expect(a).toBe(2);
      expect(b).toBe(1);
    });

    it("[Invariant] IsSame is reflexive", {
      static_assert(GameAK::IsSame<int, int>);
      static_assert(GameAK::IsSame<void, void>);
      static_assert(GameAK::IsSame<double, double>);
    });

    it("[Invariant] IsBaseOf is reflexive", {
      static_assert(GameAK::IsBaseOf<Base, Base>);
      static_assert(GameAK::IsBaseOf<Derived, Derived>);
    });
  });

  return cest_result();
}
