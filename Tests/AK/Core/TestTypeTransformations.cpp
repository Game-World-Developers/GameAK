#include <AK/Core/TypeTraits.hpp>
#include <cest.h>
#include <type_traits>

int main() {
  describe("GameAK::TypeTransformations", {
    it("should correctly remove references", {
      static_assert(std::is_same_v<GameAK::RemoveRef<int &>, int>);
      static_assert(std::is_same_v<GameAK::RemoveRef<int &&>, int>);
      static_assert(std::is_same_v<GameAK::RemoveRef<int>, int>);
      expect((std::is_same_v<GameAK::RemoveRef<int &>, int>)).toBeTruthy();
      expect((std::is_same_v<GameAK::RemoveRef<int &&>, int>)).toBeTruthy();
      expect((std::is_same_v<GameAK::RemoveRef<int>, int>)).toBeTruthy();
    });

    it("should correctly remove const", {
      static_assert(std::is_same_v<GameAK::RemoveConst<const int>, int>);
      static_assert(std::is_same_v<GameAK::RemoveConst<int>, int>);
      expect((std::is_same_v<GameAK::RemoveConst<const int>, int>)).toBeTruthy();
      expect((std::is_same_v<GameAK::RemoveConst<int>, int>)).toBeTruthy();
    });

    it("should correctly remove CV qualifiers", {
      static_assert(std::is_same_v<GameAK::RemoveCV<const volatile int>, int>);
      static_assert(std::is_same_v<GameAK::RemoveCV<const int>, int>);
      static_assert(std::is_same_v<GameAK::RemoveCV<volatile int>, int>);
      expect((std::is_same_v<GameAK::RemoveCV<const volatile int>, int>)).toBeTruthy();
      expect((std::is_same_v<GameAK::RemoveCV<const int>, int>)).toBeTruthy();
      expect((std::is_same_v<GameAK::RemoveCV<volatile int>, int>)).toBeTruthy();
    });

    it("should correctly remove CV and references", {
      static_assert(std::is_same_v<GameAK::RemoveCVRef<const int &>, int>);
      static_assert(std::is_same_v<GameAK::RemoveCVRef<volatile int &&>, int>);
      expect((std::is_same_v<GameAK::RemoveCVRef<const int &>, int>)).toBeTruthy();
      expect((std::is_same_v<GameAK::RemoveCVRef<volatile int &&>, int>)).toBeTruthy();
    });

    it("should correctly add const", {
      static_assert(std::is_same_v<GameAK::AddConst<int>, const int>);
      static_assert(std::is_same_v<GameAK::AddConst<const int>, const int>);
      expect((std::is_same_v<GameAK::AddConst<int>, const int>)).toBeTruthy();
      expect((std::is_same_v<GameAK::AddConst<const int>, const int>)).toBeTruthy();
    });

    it("should correctly add references", {
      static_assert(std::is_same_v<GameAK::AddLValueRef<int>, int &>);
      static_assert(std::is_same_v<GameAK::AddRValueRef<int>, int &&>);
      expect((std::is_same_v<GameAK::AddLValueRef<int>, int &>)).toBeTruthy();
      expect((std::is_same_v<GameAK::AddRValueRef<int>, int &&>)).toBeTruthy();
    });

    it("should correctly decay types", {
      static_assert(std::is_same_v<GameAK::Decay<int[10]>, int *>);
      static_assert(std::is_same_v<GameAK::Decay<int(int)>, int (*)(int)>);
      static_assert(std::is_same_v<GameAK::Decay<const int &>, int>);
      expect((std::is_same_v<GameAK::Decay<int[10]>, int *>)).toBeTruthy();
      expect((std::is_same_v<GameAK::Decay<int(int)>, int (*)(int)>)).toBeTruthy();
      expect((std::is_same_v<GameAK::Decay<const int &>, int>)).toBeTruthy();
    });
  });

  return cest_result();
}
