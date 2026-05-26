# Memory Module

## AllocatorConcept (`<AK/Memory/AllocatorConcept.hpp>`)

### Summary

Three C++20 concepts defining allocator interfaces: `Allocator` requires `allocate` and `owns`; `ArenaAllocatorC` and `PoolAllocatorC` extend for checkpoint and pool semantics respectively.

### Guarantees

- concepts are evaluated at compile time
- no runtime overhead from concept checks
- constraints use `std::same_as` for exact return type matching

### Non-Guarantees

- concepts do not validate runtime behavior (exhaustion, alignment, ownership)
- a type satisfying `Allocator` may still return nullptr on allocation

### Failure Semantics

- concept violation produces a hard compilation error
- no runtime validation layer exists in the concept definitions

### Complexity

| Operation | Complexity |
|-----------|-------------|
| concept satisfaction check | compile-time O(1) |

### Threading

Concepts are stateless — no thread-safety concerns.

### Valid Usage

```cpp
template<typename A>
requires GameAK::Memory::Allocator<A>
void use_allocator(A& alloc) {
    void* p = alloc.allocate(256, 16);
}
```

### Invariants

- concepts test interface shape only, not behavioral properties

---

## ArenaAllocator (`<AK/Memory/ArenaAllocator.hpp>`)

### Summary

ArenaAllocator performs monotonic linear allocations over a fixed memory region.
Allocations are never individually freed.
Memory is reclaimed through `reset()` or `restore()`.

### Guarantees

- allocation order is deterministic and sequential
- returned memory satisfies the requested alignment
- allocator never performs heap allocations
- `allocate()` is O(1) and branchless in the successful path
- `save()`/`restore()`/`reset()` are O(1)
- checkpoint is a single `usize` value — trivially copyable and composable
- `owns()` is a range check — O(1)

### Non-Guarantees

- allocator is not thread-safe
- memory is not zero-initialized
- `restore()` does not invoke destructors
- pointer lifetime is not tracked
- no per-allocation metadata is stored
- allocations are not individually reclaimable

### Failure Semantics

- returns `nullptr` if remaining capacity is insufficient for the requested size and alignment
- zero-sized allocations return `nullptr`
- invalid alignment (non-power-of-two) triggers `GAMEAK_DEBUG_BREAK()` in debug builds
- behavior is undefined if `checkpoint` was produced by a different allocator instance
- behavior is undefined if `checkpoint` exceeds `m_capacity`

### Memory Behavior

- allocations are contiguous and monotonic
- `reset()` invalidates all previously returned pointers
- `restore()` invalidates allocations made after the checkpoint
- allocator never relocates memory
- no per-allocation bookkeeping overhead
- alignment padding is inserted between allocations as needed

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `allocate` | O(1) |
| `save` | O(1) |
| `restore` | O(1) |
| `reset` | O(1) |
| `owns` | O(1) |
| `can_alloc` | O(1) |

### Threading

- external synchronization is required for concurrent access
- concurrent read-only operations (`save`, `remaining`, `capacity`, `owns`) are safe only if no concurrent writes occur

### Valid Usage

```cpp
byte buffer[KB(64)];
ArenaAllocator arena(buffer, sizeof(buffer));

Transform* transforms = arena.allocate<Transform>(1024);

auto checkpoint = arena.save();
char* temp = arena.allocate<char>(256);
arena.restore(checkpoint); // temp is reclaimed
```

### Invalid Usage

```cpp
arena.allocate(128, 3); // alignment 3 is not a power of two

auto cp1 = arena.save();
auto cp2 = arena.save();
arena.restore(cp1);
arena.restore(cp2); // invalid: cp2 exceeds current offset
```

### Invariants

- `m_offset` never exceeds `m_capacity`
- `allocate()` never decreases `m_offset`
- `restore()` never increases `m_offset`
- `save()` returns a value in `[0, m_capacity]`
- `used() + remaining() == capacity()` is always true

### Integration Notes

- suitable for deterministic simulation runtimes
- compatible with mmap-backed scratch arenas
- checkpoint/restore pairs enable scoped temporary allocation without nesting limits
- no hidden heap allocations occur during any operation

---

## PoolAllocator (`<AK/Memory/PoolAllocator.hpp>`)

### Summary

PoolAllocator manages fixed-size blocks using an intrusive free list.
Blocks are acquired and released in O(1).
No per-block metadata is stored for allocated blocks.

### Guarantees

- `acquire()` and `release()` are O(1)
- `release()` pushes the block back onto the free list — no searching or coalescing
- no heap allocations occur during any operation
- free list pointer reuses the first `sizeof(void*)` bytes of each free block
- `owns()` validates both range and alignment

### Non-Guarantees

- allocator is not thread-safe
- `release()` does not validate that the pointer was acquired from this pool
- releasing a block that is already free produces an incorrect free list
- `reset()` is O(n) — it rebuilds the entire free list
- no destructors are called by `release(void*)` (use `release<T>()` for destruction)

### Failure Semantics

- returns `nullptr` from `acquire()` when `free_count() == 0`
- `release(nullptr)` is ignored (no-op)
- `release()` of an out-of-range pointer produces undefined behavior
- double-release produces a corrupted free list
- invalid constructor parameters (block_size < `sizeof(void*)`, non-power-of-two alignment) trigger `GAMEAK_DEBUG_BREAK()` in debug builds

### Memory Behavior

- all blocks are fixed-size, determined at construction
- free blocks store a next-pointer in their first `sizeof(void*)` bytes
- allocated blocks use the full `block_size` for user data
- `reset()` re-initializes the free list over the entire buffer, invalidating all previously acquired blocks
- buffer layout is sequential: blocks are packed at `block_alignment` stride starting from the buffer base

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `acquire` | O(1) |
| `release` | O(1) |
| `owns` | O(1) |
| `reset` | O(n) |
| construction | O(n) |

### Threading

- external synchronization is required for concurrent access
- concurrent `acquire()` / `release()` from multiple threads is unsafe

### Valid Usage

```cpp
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float life;
};

byte buffer[KB(16)];
PoolAllocator pool(buffer, sizeof(buffer),
                   sizeof(Particle), alignof(Particle));

Particle* p = pool.acquire<Particle>();
if (p) {
    new (p) Particle{};
}
pool.release(p);
```

### Invalid Usage

```cpp
PoolAllocator pool(buffer, sizeof(buffer), 4, 4);
// block_size = 4 is less than sizeof(void*) — invalid on 64-bit platforms

void* a = pool.acquire();
pool.release(a);
pool.release(a); // double-free: corrupts free list

void* foreign = malloc(64);
pool.release(foreign); // undefined behavior — pointer not owned by pool
```

### Invariants

- `free_count() + used_count() == block_count()`
- `block_count()` is constant after construction
- free list is a valid singly-linked list through free blocks
- every block address is within `[m_buffer, m_buffer + block_count * block_size)`

### Integration Notes

- suitable for fixed-size object pools (entities, particles, messages)
- compatible with ECS archetype chunk allocation
- no external dependencies — free list is self-contained within the buffer
- `release<T>()` conditionally invokes destructors via `IsTriviallyDestructible` — zero overhead for trivial types

---

## MemoryDebug (`<AK/Memory/MemoryDebug.hpp>`)

### Summary

Three debug-only validation functions for allocator invariants.
All are no-ops when `GAMEAK_DEBUG_VALIDATE` is not defined.

### Guarantees

- `validate_alignment` triggers `GAMEAK_DEBUG_BREAK()` if alignment is not a power of two
- `validate_offset` triggers `GAMEAK_DEBUG_BREAK()` if offset exceeds capacity
- `validate_pointer_in_range` triggers `GAMEAK_DEBUG_BREAK()` if pointer is outside `[buffer, buffer + capacity)`
- all functions are eliminated entirely in release builds

### Non-Guarantees

- validation is not present in release builds — defects silently pass
- `validate_pointer_in_range` does not detect dangling pointers, only out-of-range pointers

### Failure Semantics

- violation triggers `GAMEAK_DEBUG_BREAK()` — debugger interruption or `__builtin_trap()`
- no exception is thrown
- no error code is returned

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `validate_alignment` | O(1) |
| `validate_offset` | O(1) |
| `validate_pointer_in_range` | O(1) |

### Threading

- functions are safe for concurrent call — they only read parameters and conditionally halt execution
- no mutable state is accessed

### Valid Usage

```cpp
void* allocate_aligned(usize size, usize alignment) {
    Memory::Debug::validate_alignment(alignment);
    return allocator.allocate(size, alignment);
}
```

### Invariants

- functions are no-ops in the absence of `GAMEAK_DEBUG_VALIDATE`
- `validate_alignment` accepts only power-of-two values

### Integration Notes

- intended for debug-mode assertion layers in allocator implementations
- `GAMEAK_DEBUG_VALIDATE` should be defined in debug/test builds, undefined in release builds
