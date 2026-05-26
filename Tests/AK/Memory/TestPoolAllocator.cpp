#include <AK/Memory/PoolAllocator.hpp>
#include <cest.h>
#include <csetjmp>
#include <cstring>
#include <signal.h>

namespace {

static sigjmp_buf trap_jmp_buf;

extern "C" void trap_handler(int) { siglongjmp(trap_jmp_buf, 1); }

struct PoolTracker {
  static int count;
  GameAK::u64 dummy;
  PoolTracker() : dummy(0) { ++count; }
  ~PoolTracker() { --count; }
};
int PoolTracker::count = 0;

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
  describe("GameAK::PoolAllocator", {
    it("should construct over a buffer and report initial state", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      expect(pool.block_count()).toBe(8UL);
      expect(pool.block_size()).toBe(32UL);
      expect(pool.free_count()).toBe(8UL);
      expect(pool.used_count()).toBe(0UL);
      expect(pool.is_empty()).toBeTruthy();
      expect(pool.is_full()).toBeFalsy();
    });

    it("should acquire blocks sequentially", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      void *b1 = pool.acquire();
      void *b2 = pool.acquire();
      void *b3 = pool.acquire();
      expect(b1 != nullptr).toBeTruthy();
      expect(b2 != nullptr).toBeTruthy();
      expect(b3 != nullptr).toBeTruthy();
      expect(b1 != b2).toBeTruthy();
      expect(b2 != b3).toBeTruthy();
      expect(pool.used_count()).toBe(3UL);
      expect(pool.free_count()).toBe(5UL);
    });

    it("should return nullptr when pool is exhausted", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      for (GameAK::usize i = 0; i < 4; ++i) {
        expect(pool.acquire() != nullptr).toBeTruthy();
      }
      expect(pool.acquire() == nullptr).toBeTruthy();
      expect(pool.is_full()).toBeTruthy();
    });

    it("should release a block back to the pool", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      void *b1 = pool.acquire();
      void *b2 = pool.acquire();
      (void)b2;
      expect(pool.free_count()).toBe(2UL);
      pool.release(b1);
      expect(pool.free_count()).toBe(3UL);
      void *reacquired = pool.acquire();
      expect(reacquired == b1).toBeTruthy();
    });

    it("should support typed acquire<T>", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      int *p = pool.acquire<int>();
      expect(p != nullptr).toBeTruthy();
      expect(pool.used_count()).toBe(1UL);
    });

    it("should call destructor on typed release<T> for non-trivial types", {
      PoolTracker::count = 0;

      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), sizeof(PoolTracker),
                                 alignof(PoolTracker));
      auto *obj = pool.acquire<PoolTracker>();
      new (obj) PoolTracker();
      expect(PoolTracker::count).toBe(1);
      pool.release(obj);
      expect(PoolTracker::count).toBe(0);
    });

    it("should support reset", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      void *unused1 = pool.acquire();
      void *unused2 = pool.acquire();
      (void)unused1;
      (void)unused2;
      expect(pool.used_count()).toBe(2UL);
      expect(pool.free_count()).toBe(2UL);
      pool.reset();
      expect(pool.used_count()).toBe(0UL);
      expect(pool.free_count()).toBe(4UL);
      expect(pool.is_empty()).toBeTruthy();
    });

    it("should correctly identify owned pointers", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      void *inside = pool.acquire();
      expect(pool.owns(inside)).toBeTruthy();
      expect(pool.owns(buffer)).toBeTruthy();
      int outside = 0;
      expect(pool.owns(&outside)).toBeFalsy();
    });

    it("should reject foreign pointers on release", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      int foreign = 0;
      expect(pool.owns(&foreign)).toBeFalsy();
    });

    it("should release null as a no-op", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      pool.release(nullptr);
      expect(pool.free_count()).toBe(8UL);
    });

    it("should support acquire-release-acquire cycle", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      void *blocks[4];
      for (int i = 0; i < 4; ++i) {
        blocks[i] = pool.acquire();
      }
      for (int i = 0; i < 4; ++i) {
        pool.release(blocks[i]);
      }
      expect(pool.is_empty()).toBeTruthy();
      for (int i = 0; i < 4; ++i) {
        void *b = pool.acquire();
        expect(b != nullptr).toBeTruthy();
      }
      expect(pool.is_full()).toBeTruthy();
    });

    it("should provide distinct non-overlapping blocks", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 64, 8);
      void *b1 = pool.acquire();
      void *b2 = pool.acquire();
      void *b3 = pool.acquire();
      expect(b1 != nullptr).toBeTruthy();
      expect(b2 != nullptr).toBeTruthy();
      expect(b3 != nullptr).toBeTruthy();
      std::memset(b1, 0xAA, 64);
      std::memset(b2, 0xBB, 64);
      std::memset(b3, 0xCC, 64);
    });

    it("[Invariant] free_count + used_count == block_count", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);

      expect(pool.free_count() + pool.used_count()).toBe(pool.block_count());

      void *b1 = pool.acquire();
      (void)b1;
      expect(pool.free_count() + pool.used_count()).toBe(pool.block_count());

      void *b2 = pool.acquire();
      (void)b2;
      expect(pool.free_count() + pool.used_count()).toBe(pool.block_count());

      pool.release(b1);
      expect(pool.free_count() + pool.used_count()).toBe(pool.block_count());
    });

    it("[Invariant] is_empty when free_count == block_count", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      expect(pool.is_empty()).toBeTruthy();
      expect(pool.free_count()).toBe(pool.block_count());
    });

    it("[Invariant] is_full when used_count == block_count", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      for (GameAK::usize i = 0; i < 4; ++i) (void)pool.acquire();
      expect(pool.is_full()).toBeTruthy();
      expect(pool.used_count()).toBe(pool.block_count());
    });

    it("[Property] acquire after release returns the same address", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      void *blocks[8];
      for (int i = 0; i < 8; ++i) blocks[i] = pool.acquire();
      for (int i = 0; i < 8; ++i) pool.release(blocks[i]);
      for (int i = 0; i < 8; ++i) {
        void *re = pool.acquire();
        expect(re != nullptr).toBeTruthy();
      }
    });

    it("[Stress: PoolAllocator cyclic acquire/release 100000x]", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);

      int ok = 0;
      for (int cycle = 0; cycle < 100000; ++cycle) {
        void *b1 = pool.acquire();
        void *b2 = pool.acquire();
        void *b3 = pool.acquire();
        void *b4 = pool.acquire();
        if (b1 && b2 && b3 && b4) ok++;
        if (pool.acquire() == nullptr) ok++;

        pool.release(b1);
        pool.release(b2);
        pool.release(b3);
        pool.release(b4);
        if (pool.is_empty()) ok++;
      }
      expect(ok).toBe(300000);
    });

    it("[Critical Bug: PoolAllocator::reset() corrupts the memory when"
       "m_block_count == 0]",
       {
         SavedSigaction saved;
         saved.install();

         GameAK::byte buffer[16];
         alignas(GameAK::PoolAllocator)
             GameAK::byte pool_storage[sizeof(GameAK::PoolAllocator)];

         if (sigsetjmp(trap_jmp_buf, 1) == 0) {
           new (pool_storage)
               GameAK::PoolAllocator(buffer, sizeof(buffer), 32, 8);
         }

         saved.restore();

         GameAK::PoolAllocator *pool =
             reinterpret_cast<GameAK::PoolAllocator *>(pool_storage);
         expect(pool->block_count()).toBe(0UL);

         pool->reset();
         expect(pool->block_count()).toBe(0UL);
         expect(pool->free_count()).toBe(0UL);
       });

    it("[Critical Bug: PoolAllocator: double-release]", {
      GameAK::byte buffer[128];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);

      void *b1 = pool.acquire();
      void *b2 = pool.acquire();
      void *b3 = pool.acquire();
      void *b4 = pool.acquire();
      (void)b2;
      (void)b3;
      (void)b4;
      expect(pool.is_full()).toBeTruthy();

      pool.release(b1);
      expect(pool.free_count()).toBe(1UL);

      pool.release(b1);
      expect(pool.free_count()).toBe(1UL);

      void *r1 = pool.acquire();
      void *r2 = pool.acquire();
      expect(r1 != nullptr).toBeTruthy();
      expect(r2 == nullptr).toBeTruthy();
      expect(r1 == b1).toBeTruthy();
    });

    it("[Critical Bug: PoolAllocator: release of pointer misaligned]", {
      GameAK::byte buffer[256];
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);
      void *block = pool.acquire();
      expect(block != nullptr).toBeTruthy();

      void *misaligned = static_cast<GameAK::byte *>(block) + 4;
      expect(pool.owns(misaligned)).toBeFalsy();
    });

    it("[Critical Bug: PoolAllocator::is_valid_block non-power-of-two block_size]", {
      GameAK::byte buffer[256];

      GameAK::PoolAllocator pool_valid_32(buffer, sizeof(buffer), 32, 8);
      expect(pool_valid_32.block_count()).toBe(8UL);

      GameAK::PoolAllocator pool_valid_64(buffer, sizeof(buffer), 64, 16);
      expect(pool_valid_64.block_count()).toBe(4UL);

      SavedSigaction saved;
      saved.install();

      alignas(GameAK::PoolAllocator)
          GameAK::byte pool_storage[sizeof(GameAK::PoolAllocator)];

      if (sigsetjmp(trap_jmp_buf, 1) == 0) {
        new (pool_storage)
            GameAK::PoolAllocator(buffer, sizeof(buffer), 48, 16);
      }

      saved.restore();

      GameAK::PoolAllocator *pool_48 =
          reinterpret_cast<GameAK::PoolAllocator *>(pool_storage);
      expect(pool_48->block_count()).toBe(0UL);
    });

    it("[Critical Bug: PoolAllocator: acquire-release-acquire-release cíclico "
       "(1000x)]",
       {
         GameAK::byte buffer[128];
         GameAK::PoolAllocator pool(buffer, sizeof(buffer), 32, 8);

         int ok = 0;
         for (int cycle = 0; cycle < 1000; ++cycle) {
           void *b1 = pool.acquire();
           void *b2 = pool.acquire();
           void *b3 = pool.acquire();
           void *b4 = pool.acquire();
           if (b1 && b2 && b3 && b4) ok++;
           if (pool.acquire() == nullptr) ok++;

           pool.release(b1);
           pool.release(b2);
           pool.release(b3);
           pool.release(b4);
           if (pool.is_empty()) ok++;
         }
         expect(ok).toBe(3000);
       });
  });

  return cest_result();
}
