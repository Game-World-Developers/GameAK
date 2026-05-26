#include <AK/Core/Types.hpp>
#include <cest.h>

int main() {
  describe("GameAK::Types", {
    it("should have correct sizes for integer types", {
      expect(sizeof(GameAK::i8)).toBe(1);
      expect(sizeof(GameAK::u8)).toBe(1);
      expect(sizeof(GameAK::i16)).toBe(2);
      expect(sizeof(GameAK::u16)).toBe(2);
      expect(sizeof(GameAK::i32)).toBe(4);
      expect(sizeof(GameAK::u32)).toBe(4);
      expect(sizeof(GameAK::i64)).toBe(8);
      expect(sizeof(GameAK::u64)).toBe(8);
    });

    it("should have correct sizes for floating point types", {
      expect(sizeof(GameAK::f32)).toBe(4);
      expect(sizeof(GameAK::f64)).toBe(8);
    });

    it("should have correct sizes for size and pointer types", {
      expect(sizeof(GameAK::usize)).toBe(sizeof(void *));
      expect(sizeof(GameAK::isize)).toBe(sizeof(void *));
      expect(sizeof(GameAK::uptr)).toBe(sizeof(void *));
      expect(sizeof(GameAK::iptr)).toBe(sizeof(void *));
    });

    it("should have correct properties for byte type", {
      expect(sizeof(GameAK::byte)).toBe(1);
      expect(std::is_enum_v<GameAK::byte>).toBeTruthy();
    });

    it("should have correct values for memory constants", {
      expect(GameAK::KiB).toBe(1024ull);
      expect(GameAK::MiB).toBe(1024ull * 1024);
      expect(GameAK::GiB).toBe(1024ull * 1024 * 1024);
    });

    it("[Invariant] memory constants relate correctly", {
      expect(GameAK::MiB).toBe(1024 * GameAK::KiB);
      expect(GameAK::GiB).toBe(1024 * GameAK::MiB);
      expect(GameAK::TiB).toBe(1024 * GameAK::GiB);
    });

    it("should have correct TiB constant", {
      expect(GameAK::TiB).toBe(1099511627776ull);
    });

    it("[Invariant] usize matches pointer width", {
      expect(sizeof(GameAK::usize)).toBe(sizeof(void *));
    });

    it("[Invariant] uptr matches pointer width", {
      expect(sizeof(GameAK::uptr)).toBe(sizeof(void *));
    });
  });

  return cest_result();
}
