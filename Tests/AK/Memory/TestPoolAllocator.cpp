#include <AK/Memory/PoolAllocator.hpp>
#include <cest.h>
#include <cstring>

namespace {
struct PoolTracker {
  static int count;
  GameAK::u64 dummy;
  PoolTracker() : dummy(0) { ++count; }
  ~PoolTracker() { --count; }
};
int PoolTracker::count = 0;
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
      GameAK::PoolAllocator pool(buffer, sizeof(buffer), sizeof(PoolTracker), alignof(PoolTracker));
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
  });

  return cest_result();
}
