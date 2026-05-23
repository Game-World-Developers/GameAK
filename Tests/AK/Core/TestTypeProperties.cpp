#include <AK/Core/TypeTraits.hpp>
#include <cest.h>

namespace {
enum TestEnum { A, B };
union TestUnion { int i; float f; };
struct TestClass { int x; };
} // namespace

int main() {
  describe("GameAK::TypeProperties", {
    it("should correctly identify integral types", {
      expect(GameAK::IsIntegral<int>).toBeTruthy();
      expect(GameAK::IsIntegral<char>).toBeTruthy();
      expect(GameAK::IsIntegral<long long>).toBeTruthy();
      expect(GameAK::IsIntegral<float>).toBeFalsy();
      expect(GameAK::IsIntegral<int *>).toBeFalsy();
    });

    it("should correctly identify floating point types", {
      expect(GameAK::IsFloatingPoint<float>).toBeTruthy();
      expect(GameAK::IsFloatingPoint<double>).toBeTruthy();
      expect(GameAK::IsFloatingPoint<int>).toBeFalsy();
    });

    it("should correctly identify pointers", {
      expect(GameAK::IsPointer<int *>).toBeTruthy();
      expect(GameAK::IsPointer<void *>).toBeTruthy();
      expect(GameAK::IsPointer<int>).toBeFalsy();
      expect(GameAK::IsPointer<int &>).toBeFalsy();
    });

    it("should correctly identify references", {
      expect(GameAK::IsReference<int &>).toBeTruthy();
      expect(GameAK::IsReference<int &&>).toBeTruthy();
      expect(GameAK::IsReference<int>).toBeFalsy();
    });

    it("should correctly identify trivially copyable types", {
      expect(GameAK::IsTriviallyCopyable<int>).toBeTruthy();
      struct Simple {
        int x;
      };
      expect(GameAK::IsTriviallyCopyable<Simple>).toBeTruthy();
    });

    it("should correctly identify void types", {
      static_assert(GameAK::IsVoid<void>);
      static_assert(!GameAK::IsVoid<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify nullptr types", {
      static_assert(GameAK::IsNullptr<decltype(nullptr)>);
      static_assert(!GameAK::IsNullptr<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify arithmetic types", {
      static_assert(GameAK::IsArithmetic<int>);
      static_assert(GameAK::IsArithmetic<float>);
      static_assert(GameAK::IsArithmetic<char>);
      static_assert(!GameAK::IsArithmetic<int *>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify lvalue and rvalue references", {
      static_assert(GameAK::IsLValueReference<int &>);
      static_assert(!GameAK::IsLValueReference<int &&>);
      static_assert(GameAK::IsRValueReference<int &&>);
      static_assert(!GameAK::IsRValueReference<int &>);
      static_assert(!GameAK::IsLValueReference<int>);
      static_assert(!GameAK::IsRValueReference<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify enum types", {
      static_assert(GameAK::IsEnum<TestEnum>);
      static_assert(!GameAK::IsEnum<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify union types", {
      static_assert(GameAK::IsUnion<TestUnion>);
      static_assert(!GameAK::IsUnion<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify class types", {
      static_assert(GameAK::IsClass<TestClass>);
      static_assert(!GameAK::IsClass<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify array types", {
      static_assert(GameAK::IsArray<int[10]>);
      static_assert(GameAK::IsArray<char[5]>);
      static_assert(!GameAK::IsArray<int *>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify const-qualified types", {
      static_assert(GameAK::IsConst<const int>);
      static_assert(GameAK::IsConst<const volatile int>);
      static_assert(!GameAK::IsConst<int>);
      static_assert(!GameAK::IsConst<int &>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify trivially destructible types", {
      static_assert(GameAK::IsTriviallyDestructible<int>);
      static_assert(GameAK::IsTriviallyDestructible<float>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify trivially default constructible types", {
      static_assert(GameAK::IsTriviallyDefaultConstructible<int>);
      struct Trivial {
        int x;
      };
      static_assert(GameAK::IsTriviallyDefaultConstructible<Trivial>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify trivially copy constructible types", {
      static_assert(GameAK::IsTriviallyCopyConstructible<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify trivially move constructible types", {
      static_assert(GameAK::IsTriviallyMoveConstructible<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify trivially relocatable types", {
      static_assert(GameAK::IsTriviallyRelocatable<int>);
      static_assert(GameAK::IsTriviallyRelocatable<float>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify default constructible types", {
      static_assert(GameAK::IsDefaultConstructible<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify move constructible types", {
      static_assert(GameAK::IsMoveConstructible<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify copy constructible types", {
      static_assert(GameAK::IsCopyConstructible<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify move assignable types", {
      static_assert(GameAK::IsMoveAssignable<int>);
      expect(true).toBeTruthy();
    });

    it("should correctly identify copy assignable types", {
      static_assert(GameAK::IsCopyAssignable<int>);
      expect(true).toBeTruthy();
    });
  });

  return cest_result();
}
