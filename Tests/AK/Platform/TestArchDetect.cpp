#include <AK/Platform/ArchDetect.hpp>
#include <cest.h>

#ifdef GAMEAK_ARCH_64_BIT
constexpr bool kArch64Bit = true;
#else
constexpr bool kArch64Bit = false;
#endif

int main() {
  describe("GameAK::ArchDetect", {
    it("should detect a 64-bit architecture",
       { expect(kArch64Bit).toBeTruthy(); });
  });

  return cest_result();
}
