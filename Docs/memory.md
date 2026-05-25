# Memory Module

GameAK provides two custom allocators plus C++20 concepts for allocator interfaces. Both allocators operate on **externally-owned buffers** — they never allocate or free heap memory themselves.

## AllocatorConcept (`<AK/Memory/AllocatorConcept.hpp>`)

C++20 concepts defining allocator interfaces:

```cpp
namespace GameAK {

  template<typename A>
  concept Allocator = requires(A& a, usize size, usize alignment, const void* ptr) {
    { a.allocate(size, alignment) } -> SameAs<void*>;
    { a.owns(ptr) } -> SameAs<bool>;
  };

  template<typename A>
  concept ArenaAllocatorC = Allocator<A> && requires(A& a) {
    { a.save() } -> SameAs<usize>;
    { a.restore(usize{}) } -> SameAs<void>;
    { a.reset() } -> SameAs<void>;
  };

  template<typename A>
  concept PoolAllocatorC = Allocator<A> && requires(A& a) {
    { a.release((void*)nullptr) } -> SameAs<void>;
    { a.reset() } -> SameAs<void>;
    { a.free_count() } -> SameAs<usize>;
    { a.used_count() } -> SameAs<usize>;
  };

}
```

### Helper Functions

```cpp
// Allocate and construct a single object
template<typename T, Allocator A, typename... Args>
T* construct_at(A& alloc, Args&&... args);

// Destroy an object (no-op for trivially destructible types)
template<typename T>
void destroy_at(T* ptr);
```

## ArenaAllocator (`<AK/Memory/ArenaAllocator.hpp>`)

A bump-pointer arena allocator — the simplest and fastest allocation strategy.

### How It Works

- A contiguous buffer is divided sequentially
- An internal offset tracks the next free position
- `allocate()` aligns the offset, bumps it, and returns the previous position
- Memory is reclaimed via `reset()` (jumps back to offset 0) or via `save()`/`restore()` pairs

### API

| Method | Complexity | Description |
|--------|-----------|-------------|
| `ArenaAllocator(void* buffer, usize capacity)` | O(1) | Construct over external buffer |
| `allocate(usize size, usize alignment)` | O(1) | Aligned bump allocation |
| `allocate<T>(usize count = 1)` | O(1) | Typed allocation (alignof(T), sizeof(T) * count) |
| `save()` | O(1) | Returns current offset as checkpoint |
| `restore(usize checkpoint)` | O(1) | Rewinds to checkpoint (invalidates later allocs) |
| `reset()` | O(1) | Rewinds to offset 0 |
| `used()` | O(1) | Bytes currently allocated |
| `remaining()` | O(1) | Bytes available |
| `capacity()` | O(1) | Total buffer size |
| `owns(const void*)` | O(1) | Range check |
| `can_alloc(usize size, usize alignment)` | O(1) | Predicate (no side effects) |

### Example

```cpp
#include <AK/Memory/ArenaAllocator.hpp>

GameAK::u8 buffer[GameAK::KiB * 64];
GameAK::ArenaAllocator arena(buffer, sizeof(buffer));

// Typed allocations
int* ints = arena.allocate<int>(128);
float* positions = arena.allocate<float>(3);  // x, y, z

// Manual allocation with custom alignment
void* aligned = arena.allocate(256, 64);

// Scoped temporary allocations
auto checkpoint = arena.save();
{
  char* temp = arena.allocate<char>(512);
  // ... use temp ...
}
arena.restore(checkpoint);  // temp space reclaimed

// Reset entire arena
arena.reset();
```

### Use Cases

- Per-frame scratch allocators in game loops
- Loading assets into a temporary arena, then discarding
- Building command buffers or serialization output
- Nested arena hierarchies where child arenas borrow from a parent

## PoolAllocator (`<AK/Memory/PoolAllocator.hpp>`)

A fixed-size block pool allocator using an intrusive free list — zero metadata overhead per block.

### How It Works

- A contiguous buffer is divided into equal-sized blocks
- The first `sizeof(void*)` bytes of each **free** block store a pointer to the next free block (intrusive linked list)
- `acquire()` pops the head of the free list — O(1)
- `release()` pushes the block back onto the free list — O(1)
- No metadata is stored for **allocated** blocks

### Constraints

| Constraint | Reason |
|-----------|--------|
| `block_size >= sizeof(void*)` | Must fit the free-list pointer |
| `block_alignment >= alignof(void*)` | Free-list pointer must be aligned |
| `block_alignment` is power of two | Standard alignment contract |
| `block_size % block_alignment == 0` | Block-aligned stride |

### API

| Method | Complexity | Description |
|--------|-----------|-------------|
| `PoolAllocator(void* buffer, usize capacity, usize block_size, usize block_alignment)` | O(n) | Construct and build free list |
| `acquire()` | O(1) | Pop free block, or nullptr |
| `acquire<T>()` | O(1) | Typed convenience wrapper |
| `release(void* ptr)` | O(1) | Push block back to free list |
| `release<T>(T* ptr)` | O(1) | Destructor + release |
| `reset()` | O(n) | Rebuild free list |
| `owns(const void*)` | O(1) | Range + alignment check |
| `used_count()` | O(1) | Blocks currently acquired |
| `free_count()` | O(1) | Blocks currently free |
| `block_count()` | O(1) | Total blocks |
| `block_size()` | O(1) | Size per block |
| `is_empty()` | O(1) | All blocks free |
| `is_full()` | O(1) | All blocks acquired |

### Example

```cpp
#include <AK/Memory/PoolAllocator.hpp>

struct Particle {
  float x, y, z;
  float vx, vy, vz;
  float life;
};

GameAK::u8 buffer[GameAK::KiB * 16];
GameAK::PoolAllocator pool(buffer, sizeof(buffer), sizeof(Particle), alignof(Particle));

// Acquire
Particle* p = pool.acquire<Particle>();
if (p) {
  // placement-new if needed
  new (p) Particle{};
}

// Release (calls destructor then frees block)
pool.release(p);

// Or manually
void* block = pool.acquire();
pool.release(block);
```

### Use Cases

- Object pools for entities, particles, components
- ECS archetype chunk allocation
- Fixed-size message queues
- Network packet buffers

## MemoryDebug (`<AK/Memory/MemoryDebug.hpp>`)

Debug-only validation utilities. All functions are no-ops when `GAMEAK_DEBUG_VALIDATE` is not defined.

```cpp
namespace GameAK::Memory::Debug {

  void validate_alignment(usize alignment);
  void validate_offset(usize offset, usize capacity);
  void validate_pointer_in_range(const void* ptr, const u8* buffer, usize capacity);

}
```

- `validate_alignment` — triggers `GAMEAK_DEBUG_BREAK()` if alignment is not a power of two
- `validate_offset` — triggers `GAMEAK_DEBUG_BREAK()` if offset exceeds capacity
- `validate_pointer_in_range` — triggers `GAMEAK_DEBUG_BREAK()` if pointer is outside buffer

In release builds, the entire validation layer is eliminated by the compiler.
