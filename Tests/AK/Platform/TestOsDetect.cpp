#include <AK/Platform/OsDetect.hpp>
#include <cest.h>

#ifdef GAMEAK_OS_WINDOWS
constexpr bool kOsWindows = true;
#else
constexpr bool kOsWindows = false;
#endif

#ifdef GAMEAK_OS_LINUX
constexpr bool kOsLinux = true;
#else
constexpr bool kOsLinux = false;
#endif

#ifdef GAMEAK_OS_MACOS
constexpr bool kOsMacos = true;
#else
constexpr bool kOsMacos = false;
#endif

int main() {
  describe("GameAK::OsDetect", {
    it("should define exactly one OS macro", {
      int count = kOsWindows + kOsLinux + kOsMacos;
      expect(count).toBe(1);
    });

    it("should detect the correct operating system", {
#if defined(__linux__)
      expect(kOsLinux).toBeTruthy();
#elif defined(_WIN32) || defined(_WIN64)
      expect(kOsWindows).toBeTruthy();
#elif defined(__APPLE__) && defined(__MACH__)
      expect(kOsMacos).toBeTruthy();
#endif
    });
  });

  return cest_result();
}
