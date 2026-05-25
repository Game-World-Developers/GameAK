#include <AK/Memory/ArenaAllocator.hpp>
#include <cest.h>
#include <csetjmp>
#include <limits>
#include <signal.h>

namespace {

static sigjmp_buf trap_jmp_buf;

extern "C" void trap_handler(int) { siglongjmp(trap_jmp_buf, 1); }

struct SavedSigaction {
  struct sigaction old_act;
  void install() {
    struct sigaction trap_act;
    trap_act.sa_handler = trap_handler;
    sigemptyset(&trap_act.sa_mask);
    trap_act.sa_flags = 0;
    sigaction(SIGTRAP, &trap_act, &old_act);
  }
  void restore() { sigaction(SIGTRAP, &old_act, nullptr); }
};

} // namespace

int main() {
  describe("GameAK::ArenaAllocator", {
    it("should construct over a buffer and report initial state", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      expect(arena.capacity()).toBe(sizeof(buffer));
      expect(arena.used()).toBe(0UL);
      expect(arena.remaining()).toBe(sizeof(buffer));
    });

    it("should allocate memory from the arena", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      void *p = arena.allocate(64, 1);
      expect(p != nullptr).toBeTruthy();
      expect(arena.used()).toBe(64UL);
      expect(arena.remaining()).toBe(sizeof(buffer) - 64);
    });

    it("should align allocations correctly", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      (void)arena.allocate(1, 1);
      void *p = arena.allocate(8, 8);
      expect(p != nullptr).toBeTruthy();
      expect((reinterpret_cast<GameAK::uptr>(p) & 7)).toBe(0UL);
    });

    it("should return nullptr when allocation exceeds capacity", {
      GameAK::byte buffer[64];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      void *p = arena.allocate(128, 1);
      expect(p == nullptr).toBeTruthy();
    });

    it("should return nullptr when remaining space is insufficient", {
      GameAK::byte buffer[64];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      (void)arena.allocate(60, 1);
      void *p = arena.allocate(8, 1);
      expect(p == nullptr).toBeTruthy();
    });

    it("should support save and restore checkpoints", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      auto checkpoint = arena.save();
      expect(arena.used()).toBe(0UL);
      (void)arena.allocate(128, 1);
      expect(arena.used()).toBe(128UL);
      arena.restore(checkpoint);
      expect(arena.used()).toBe(0UL);
    });

    it("should support reset", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      (void)arena.allocate(256, 1);
      expect(arena.used()).toBe(256UL);
      arena.reset();
      expect(arena.used()).toBe(0UL);
    });

    it("should correctly identify owned pointers", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      void *inside = arena.allocate(64, 1);
      expect(arena.owns(inside)).toBeTruthy();
      expect(arena.owns(buffer)).toBeTruthy();
      int outside = 0;
      expect(arena.owns(&outside)).toBeFalsy();
    });

    it("should correctly predict allocation feasibility with can_alloc", {
      GameAK::byte buffer[64];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      expect(arena.can_alloc(64, 1)).toBeTruthy();
      expect(arena.can_alloc(65, 1)).toBeFalsy();
      (void)arena.allocate(60, 1);
      expect(arena.can_alloc(4, 1)).toBeTruthy();
      expect(arena.can_alloc(8, 1)).toBeFalsy();
    });

    it("should allocate typed storage via template allocate<T>", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      int *arr = arena.allocate<int>(10);
      expect(arr != nullptr).toBeTruthy();
      expect(arena.used()).toBe(sizeof(int) * 10);
    });

    it("should allocate a single object via template allocate<T>()", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      int *p = arena.allocate<int>();
      expect(p != nullptr).toBeTruthy();
      expect(arena.used()).toBe(sizeof(int));
    });

    it("should restore to a nested checkpoint correctly", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));
      auto cp0 = arena.save();
      (void)arena.allocate(64, 1);
      auto cp1 = arena.save();
      (void)arena.allocate(64, 1);
      expect(arena.used()).toBe(128UL);
      arena.restore(cp1);
      expect(arena.used()).toBe(64UL);
      arena.restore(cp0);
      expect(arena.used()).toBe(0UL);
    });

    it("[Critical Bug: ArenaAllocator::allocate com size = USIZE_MAX]", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));

      (void)arena.allocate(8, 8);
      expect(arena.used()).toBe(8UL);

      GameAK::usize max = std::numeric_limits<GameAK::usize>::max();
      void *p = arena.allocate(max - 4, 1);
      expect(p == nullptr).toBeTruthy();
      expect(arena.used()).toBe(8UL);
    });

    it("[Critical Bug: ArenaAllocator::allocate<int>(USIZE_MAX / sizeof(int) + "
       "1)]",
       {
         GameAK::byte buffer[1024];
         GameAK::ArenaAllocator arena(buffer, sizeof(buffer));

         GameAK::usize max = std::numeric_limits<GameAK::usize>::max();
         GameAK::usize over = max / sizeof(int) + 1;
         int *p = arena.allocate<int>(over);
         expect(p == nullptr).toBeTruthy();
         expect(arena.used()).toBe(0UL);
       });

    it("[Critical Bug: ArenaAllocator: alignment non-power-of-2]", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));

      expect(arena.can_alloc(64, 0)).toBeFalsy();
      expect(arena.can_alloc(64, 3)).toBeFalsy();
      expect(arena.can_alloc(64, 5)).toBeFalsy();
      expect(arena.can_alloc(64, 6)).toBeFalsy();
      expect(arena.can_alloc(64, 7)).toBeFalsy();
      expect(arena.can_alloc(64, 9)).toBeFalsy();
      expect(arena.can_alloc(64, 15)).toBeFalsy();
    });

    it("[Critical Bug: ArenaAllocator: restore com checkpoint > m_offset]", {
      SavedSigaction saved;
      saved.install();

      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));

      (void)arena.allocate(64, 1);
      expect(arena.used()).toBe(64UL);

      if (sigsetjmp(trap_jmp_buf, 1) == 0) {
        arena.restore(128);
      }

      expect(arena.used()).toBe(64UL);

      saved.restore();
    });

    it("[Critical Bug: ArenaAllocator: zero-size allocation]", {
      GameAK::byte buffer[1024];
      GameAK::ArenaAllocator arena(buffer, sizeof(buffer));

      void *p = arena.allocate(0, 1);
      expect(p != nullptr).toBeTruthy();
      expect(arena.used()).toBe(0UL);
    });
  });

  return cest_result();
}
