#include <AK/Memory/AllocatorConcept.hpp>
#include <AK/Memory/ArenaAllocator.hpp>
#include <cest.h>

namespace {

static int g_constructed = 0;
static int g_destroyed = 0;

struct NonTrivial {
  int value;
  NonTrivial() : value(0) { ++g_constructed; }
  NonTrivial(int v) : value(v) { ++g_constructed; }
  ~NonTrivial() { ++g_destroyed; }
};

} // namespace

int main() {
  describe("GameAK::Memory::construct_at", {
    it("should construct an object in arena storage", {
      GameAK::byte buffer[256];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      int* p = GameAK::Memory::construct_at<int>(arena, 42);
      expect(p != nullptr).toBeTruthy();
      expect(*p).toBe(42);
    });

    it("should construct a NonTrivial object calling its constructor", {
      g_constructed = 0;
      GameAK::byte buffer[256];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      NonTrivial* p = GameAK::Memory::construct_at<NonTrivial>(arena, 99);
      expect(p != nullptr).toBeTruthy();
      expect(p->value).toBe(99);
      expect(g_constructed).toBe(1);
    });

    it("should return nullptr when arena is exhausted", {
      GameAK::byte buffer[1];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      int* p = GameAK::Memory::construct_at<int>(arena, 42);
      expect(p == nullptr).toBeTruthy();
    });

    it("[Edge: construct_at with zero-size allocation]", {
      struct Empty {};
      GameAK::byte buffer[8];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      Empty* p = GameAK::Memory::construct_at<Empty>(arena);
      expect(p != nullptr).toBeTruthy();
    });
  });

  describe("GameAK::Memory::destroy_at", {
    it("should be a no-op for trivially destructible types", {
      GameAK::byte buffer[64];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      int* p = GameAK::Memory::construct_at<int>(arena, 7);
      GameAK::Memory::destroy_at(p);
      expect(true).toBeTruthy();
    });

    it("should call destructor for non-trivially destructible types", {
      g_constructed = 0;
      g_destroyed = 0;
      GameAK::byte buffer[256];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      NonTrivial* p = GameAK::Memory::construct_at<NonTrivial>(arena, 5);
      expect(g_constructed).toBe(1);
      expect(g_destroyed).toBe(0);
      GameAK::Memory::destroy_at(p);
      expect(g_destroyed).toBe(1);
    });

    it("should be safe to call with nullptr", {
      GameAK::Memory::destroy_at<int>(nullptr);
      expect(true).toBeTruthy();
    });

    it("should be safe to call with nullptr for non-trivial type", {
      g_destroyed = 0;
      GameAK::Memory::destroy_at<NonTrivial>(nullptr);
      expect(g_destroyed).toBe(0);
    });
  });

  return cest_result();
}
