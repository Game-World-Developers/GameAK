#include <AK/Core/TypeTraits.hpp>
#include <cest.h>

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
  });

  return cest_result();
}
