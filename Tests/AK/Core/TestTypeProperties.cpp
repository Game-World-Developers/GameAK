#include <AK/Core/TypeTraits.hpp>
#include <cest.h>

namespace {
enum TestEnum { A, B };
union TestUnion { int i; float f; };
struct TestClass { int x; };
struct NonTrivial { ~NonTrivial() {} };
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
      struct Simple { int x; };
      expect(GameAK::IsTriviallyCopyable<Simple>).toBeTruthy();
    });

    it("should correctly identify void types", {
      static_assert(GameAK::IsVoid<void>);
      static_assert(!GameAK::IsVoid<int>);
      expect(GameAK::IsVoid<void>).toBeTruthy();
      expect(GameAK::IsVoid<int>).toBeFalsy();
    });

    it("should correctly identify nullptr types", {
      static_assert(GameAK::IsNullptr<decltype(nullptr)>);
      static_assert(!GameAK::IsNullptr<int>);
      expect(GameAK::IsNullptr<decltype(nullptr)>).toBeTruthy();
      expect(GameAK::IsNullptr<int>).toBeFalsy();
    });

    it("should correctly identify arithmetic types", {
      static_assert(GameAK::IsArithmetic<int>);
      static_assert(GameAK::IsArithmetic<float>);
      static_assert(GameAK::IsArithmetic<char>);
      static_assert(!GameAK::IsArithmetic<int *>);
      expect(GameAK::IsArithmetic<int>).toBeTruthy();
      expect(GameAK::IsArithmetic<float>).toBeTruthy();
      expect(GameAK::IsArithmetic<char>).toBeTruthy();
      expect(GameAK::IsArithmetic<int *>).toBeFalsy();
    });

    it("should correctly identify lvalue and rvalue references", {
      static_assert(GameAK::IsLValueReference<int &>);
      static_assert(!GameAK::IsLValueReference<int &&>);
      static_assert(GameAK::IsRValueReference<int &&>);
      static_assert(!GameAK::IsRValueReference<int &>);
      static_assert(!GameAK::IsLValueReference<int>);
      static_assert(!GameAK::IsRValueReference<int>);
      expect(GameAK::IsLValueReference<int &>).toBeTruthy();
      expect(GameAK::IsLValueReference<int &&>).toBeFalsy();
      expect(GameAK::IsRValueReference<int &&>).toBeTruthy();
      expect(GameAK::IsRValueReference<int &>).toBeFalsy();
      expect(GameAK::IsLValueReference<int>).toBeFalsy();
      expect(GameAK::IsRValueReference<int>).toBeFalsy();
    });

    it("should correctly identify enum types", {
      static_assert(GameAK::IsEnum<TestEnum>);
      static_assert(!GameAK::IsEnum<int>);
      expect(GameAK::IsEnum<TestEnum>).toBeTruthy();
      expect(GameAK::IsEnum<int>).toBeFalsy();
    });

    it("should correctly identify union types", {
      static_assert(GameAK::IsUnion<TestUnion>);
      static_assert(!GameAK::IsUnion<int>);
      expect(GameAK::IsUnion<TestUnion>).toBeTruthy();
      expect(GameAK::IsUnion<int>).toBeFalsy();
    });

    it("should correctly identify class types", {
      static_assert(GameAK::IsClass<TestClass>);
      static_assert(!GameAK::IsClass<int>);
      expect(GameAK::IsClass<TestClass>).toBeTruthy();
      expect(GameAK::IsClass<int>).toBeFalsy();
    });

    it("should correctly identify array types", {
      static_assert(GameAK::IsArray<int[10]>);
      static_assert(GameAK::IsArray<char[5]>);
      static_assert(!GameAK::IsArray<int *>);
      expect(GameAK::IsArray<int[10]>).toBeTruthy();
      expect(GameAK::IsArray<char[5]>).toBeTruthy();
      expect(GameAK::IsArray<int *>).toBeFalsy();
    });

    it("should correctly identify const-qualified types", {
      static_assert(GameAK::IsConst<const int>);
      static_assert(GameAK::IsConst<const volatile int>);
      static_assert(!GameAK::IsConst<int>);
      static_assert(!GameAK::IsConst<int &>);
      expect(GameAK::IsConst<const int>).toBeTruthy();
      expect(GameAK::IsConst<const volatile int>).toBeTruthy();
      expect(GameAK::IsConst<int>).toBeFalsy();
      expect(GameAK::IsConst<int &>).toBeFalsy();
    });

    it("should correctly identify trivially destructible types", {
      static_assert(GameAK::IsTriviallyDestructible<int>);
      static_assert(GameAK::IsTriviallyDestructible<float>);
      expect(GameAK::IsTriviallyDestructible<int>).toBeTruthy();
      expect(GameAK::IsTriviallyDestructible<float>).toBeTruthy();
    });

    it("should correctly identify trivially default constructible types", {
      static_assert(GameAK::IsTriviallyDefaultConstructible<int>);
      struct Trivial { int x; };
      static_assert(GameAK::IsTriviallyDefaultConstructible<Trivial>);
      expect(GameAK::IsTriviallyDefaultConstructible<int>).toBeTruthy();
      expect(GameAK::IsTriviallyDefaultConstructible<Trivial>).toBeTruthy();
    });

    it("should correctly identify trivially copy constructible types", {
      static_assert(GameAK::IsTriviallyCopyConstructible<int>);
      expect(GameAK::IsTriviallyCopyConstructible<int>).toBeTruthy();
    });

    it("should correctly identify trivially move constructible types", {
      static_assert(GameAK::IsTriviallyMoveConstructible<int>);
      expect(GameAK::IsTriviallyMoveConstructible<int>).toBeTruthy();
    });

    it("should correctly identify trivially relocatable types", {
      static_assert(GameAK::IsTriviallyRelocatable<int>);
      static_assert(GameAK::IsTriviallyRelocatable<float>);
      expect(GameAK::IsTriviallyRelocatable<int>).toBeTruthy();
      expect(GameAK::IsTriviallyRelocatable<float>).toBeTruthy();
    });

    it("[Invariant] IsTriviallyRelocatable == IsTriviallyCopyable", {
      static_assert(GameAK::IsTriviallyRelocatable<int> == GameAK::IsTriviallyCopyable<int>);
      static_assert(GameAK::IsTriviallyRelocatable<TestClass> == GameAK::IsTriviallyCopyable<TestClass>);
      static_assert(GameAK::IsTriviallyRelocatable<NonTrivial> == GameAK::IsTriviallyCopyable<NonTrivial>);
    });

    it("should correctly identify default constructible types", {
      static_assert(GameAK::IsDefaultConstructible<int>);
      expect(GameAK::IsDefaultConstructible<int>).toBeTruthy();
    });

    it("should correctly identify move constructible types", {
      static_assert(GameAK::IsMoveConstructible<int>);
      expect(GameAK::IsMoveConstructible<int>).toBeTruthy();
    });

    it("should correctly identify copy constructible types", {
      static_assert(GameAK::IsCopyConstructible<int>);
      expect(GameAK::IsCopyConstructible<int>).toBeTruthy();
    });

    it("should correctly identify move assignable types", {
      static_assert(GameAK::IsMoveAssignable<int>);
      expect(GameAK::IsMoveAssignable<int>).toBeTruthy();
    });

    it("should correctly identify copy assignable types", {
      static_assert(GameAK::IsCopyAssignable<int>);
      expect(GameAK::IsCopyAssignable<int>).toBeTruthy();
    });

    it("[Invariant] non-trivial type is not trivially destructible", {
      static_assert(!GameAK::IsTriviallyDestructible<NonTrivial>);
    });
  });

  return cest_result();
}
