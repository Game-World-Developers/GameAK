#include <AK/Core/TypeTraits.hpp>
#include <AK/Core/Types.hpp>
#include <AK/Memory/Janitor.hpp>
#include <cest.h>
#include <new>

namespace {

static int g_destructor_count = 0;

struct Tracker {
  int id;
  Tracker() : id(0) {}
  Tracker(int v) : id(v) {}
  ~Tracker() { ++g_destructor_count; }
};

struct Trivial {
  int x;
};

} // namespace

int main() {
  describe("GameAK::Janitor", {
    it("should default-construct to empty state", {
      Janitor<Tracker> jan;
      expect(jan.Get() == nullptr).toBeTruthy();
      expect(static_cast<bool>(jan)).toBeFalsy();
    });

    it("should wrap a pointer and expose it via Get()", {
      Tracker t;
      Janitor<Tracker> jan(&t);
      expect(jan.Get() == &t).toBeTruthy();
      expect(static_cast<bool>(jan)).toBeTruthy();
    });

    it("should release ownership without calling destructor", {
      g_destructor_count = 0;
      GameAK::byte buf[sizeof(Tracker)];
      Tracker *t = new (buf) Tracker();
      {
        Janitor<Tracker> jan(t);
        jan.Release();
        expect(jan.Get() == nullptr).toBeTruthy();
        expect(static_cast<bool>(jan)).toBeFalsy();
      }
      expect(g_destructor_count).toBe(0);
      t->~Tracker();
      expect(g_destructor_count).toBe(1);
    });

    it("should reset and call destructor", {
      g_destructor_count = 0;
      GameAK::byte buf[sizeof(Tracker)];
      Tracker *t = new (buf) Tracker();
      {
        Janitor<Tracker> jan(t);
        jan.Reset();
        expect(jan.Get() == nullptr).toBeTruthy();
        expect(g_destructor_count).toBe(1);
      }
      expect(g_destructor_count).toBe(1);
    });

    it("should call destructor when going out of scope", {
      g_destructor_count = 0;
      GameAK::byte buf[sizeof(Tracker)];
      Tracker *t = new (buf) Tracker();
      {
        Janitor<Tracker> jan(t);
        expect(g_destructor_count).toBe(0);
      }
      expect(g_destructor_count).toBe(1);
    });

    it("should support move constructor", {
      g_destructor_count = 0;
      GameAK::byte buf[sizeof(Tracker)];
      Tracker *t = new (buf) Tracker();
      {
        Janitor<Tracker> jan1(t);
        Janitor<Tracker> jan2(GameAK::Move(jan1));
        expect(jan1.Get() == nullptr).toBeTruthy();
        expect(jan2.Get() == t).toBeTruthy();
      }
      expect(g_destructor_count).toBe(1);
    });

    it("should support move assignment", {
      g_destructor_count = 0;
      GameAK::byte buf1[sizeof(Tracker)];
      GameAK::byte buf2[sizeof(Tracker)];
      Tracker *t1 = new (buf1) Tracker();
      Tracker *t2 = new (buf2) Tracker();
      {
        Janitor<Tracker> jan1(t1);
        Janitor<Tracker> jan2(t2);
        jan2 = GameAK::Move(jan1);
        expect(jan1.Get() == nullptr).toBeTruthy();
        expect(jan2.Get() == t1).toBeTruthy();
      }
      expect(g_destructor_count).toBe(2);
    });

    it("[Edge: move assignment replaces old managed object]", {
      g_destructor_count = 0;
      GameAK::byte buf1[sizeof(Tracker)];
      GameAK::byte buf2[sizeof(Tracker)];
      Tracker *old_t = new (buf1) Tracker();
      Tracker *new_t = new (buf2) Tracker();
      {
        Janitor<Tracker> jan_old(old_t);
        Janitor<Tracker> jan_new(new_t);
        jan_new = GameAK::Move(jan_old);
      }
      expect(g_destructor_count).toBe(2);
    });

    it("[Edge: Reset() on empty Janitor is safe]", {
      Janitor<Tracker> jan;
      jan.Reset();
      expect(true).toBeTruthy();
    });

    it("[Edge: Release() on empty Janitor is safe]", {
      Janitor<Tracker> jan;
      jan.Release();
      expect(true).toBeTruthy();
    });

    it("[Edge: Janitor with trivial type compiles and runs]", {
      Trivial t{42};
      {
        Janitor<Trivial> jan(&t);
        expect(jan.Get()->x).toBe(42);
      }
      expect(true).toBeTruthy();
    });
  });

  return cest_result();
}
