# Overview

GameAK is a low-level C++20 library providing game-oriented primitives — types, allocators, bit manipulation, and a runtime-dispatch backend.

## Architecture

The library is organized in layers, each building on the previous:

```
Platform ──► Core ──► Memory ──► Backend
  │            │         │           │
  │            │         │           └── Runtime dispatch
  │            │         │               (Scalar / SIMD)
  │            │         │
  │            │         └── Allocators
  │            │             (Arena, Pool)
  │            │
  │            └── Types, TypeTraits, Macros, Optional
  │
  └── Arch / Compiler / OS detection
```

### Platform Layer

Detects the CPU architecture (x86_64, ARM64), compiler (MSVC, Clang, GCC), and operating system (Windows, Linux, macOS) at compile time. These defines are consumed by `Macros.hpp` and reserved for future OS-specific features.

### Core Layer

- **Types** — fixed-width integer/FP aliases (`i8`–`u64`, `f32`/`f64`), `usize`/`isize`, `byte`, and KiB/MiB/GiB/TiB constants. Enforces IEEE 754 and 64-bit via `static_assert`.
- **TypeTraits** — wrappers around `<type_traits>` plus `Move`, `Forward`, `Swap` utilities.
- **Macros** — portable `FORCE_INLINE`, `LIKELY`/`UNLIKELY`, `DEBUG_BREAK`, cache-line/SIMD alignment.
- **Optional** — stack-allocated optional value without exceptions. Uses `GAMEAK_DEBUG_BREAK()` on empty access.

### Bits Module

Constexpr bit-level primitives and containers: single-bit masks, power-of-two checks, aligned rounding, popcount, type-safe enum bitmasks, non-owning bitset views (with optional bounds checking), and bit-level serialization.

### Memory Module

Two allocators that operate on externally-owned buffers (zero internal heap usage):

- **ArenaAllocator** — bump-pointer arena with checkpoint/restore. O(1) alloc, no individual free.
- **PoolAllocator** — fixed-size block pool with intrusive free list. O(1) acquire/release, zero metadata overhead per block.

Both are validated through C++20 concepts (`Allocator`, `ArenaAllocatorC`, `PoolAllocatorC`).

### Backend Module

A VTable-based runtime dispatch system for memory operations (`mem_copy`, `mem_set`, `mem_cmp`) and bulk bitset operations. Currently uses a scalar backend (backed by `std::memcpy`/`memset`/`memcmp` and simple loops). A SIMD backend stub exists for future SSE/AVX/NEON acceleration.

## Design Decisions

| Decision | Rationale |
|----------|-----------|
| `-fno-exceptions -fno-rtti` | Predictable codegen, no hidden cost paths, game-industry standard |
| Allocators use external buffers | Zero heap management; caller controls memory (mmap, static arrays, nested allocators) |
| No virtual methods | Compile-time polymorphism via templates + concepts; VTable only in Backend module |
| IEEE 754 enforcement | Deterministic simulation across platforms |
| 64-bit only | Simplifies pointer arithmetic, arena capacity, mmap offsets |
| `noexcept` everywhere | No exception unwind tables; minimal codegen |
| `IsTriviallyRelocatable = IsTriviallyCopyable` | Conservative choice for safety |

## Namespaces

- `GameAK` — top-level namespace
- `GameAK::Bits` — bit operations, BitMask, BitArray, BitPack
- `GameAK::Memory` — AllocatorConcept, MemoryDebug
- `GameAK::Memory::Debug` — debug validation utilities
- `GameAK::Backend` — VTable and runtime dispatch
- `GameAK::Backend::Detail` — scalar backend implementations
