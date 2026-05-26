# Core Types

Three headers provide the foundational type system, type traits, and macros consumed by all other GameAK modules.

## Types (`<AK/Core/Types.hpp>`)

### Summary

Fixed-width integer and floating-point aliases (`i8`–`u64`, `f32`/`f64`), pointer-width types (`usize`, `isize`, `uptr`, `iptr`), `byte`, and byte-size constants (`KiB`, `MiB`, `GiB`, `TiB`).
Enforces IEEE 754 and 64-bit pointer width via `static_assert`.

### Guarantees

- all integer types match their declared width — verified by `static_assert`
- `f32`/`f64` are IEEE 754 compliant — verified by `static_assert`
- `usize` and `uptr` match pointer width — verified by `static_assert`
- size constants are `constexpr` — evaluable at compile time

### Non-Guarantees

- no overflow protection on arithmetic with these types
- no implicit type conversion safety beyond what the underlying types provide

### Failure Semantics

- `static_assert` failures produce hard compilation errors
- no runtime failure paths exist

### Complexity

All type aliases and constants are resolved at compile time — zero runtime cost.

### Threading

Type aliases and constants are stateless — no thread-safety concerns.

### Valid Usage

```cpp
u8 buffer[MiB];           // 1 MiB buffer
usize entity_count = 4096;
f64  delta_time = 0.016;  // 60 FPS timestep

static_assert(KiB == 1024);
static_assert(sizeof(u64) == 8);
static_assert(sizeof(f32) == 4);
```

### Invariants

- `sizeof(u8) == 1`, `sizeof(u16) == 2`, ..., `sizeof(u64) == 8`
- `sizeof(f32) == 4`, `sizeof(f64) == 8`
- `sizeof(usize) == sizeof(uptr) == sizeof(void*)`

### Integration Notes

- consumed by every GameAK header
- 64-bit-only requirement keeps pointer arithmetic consistent across platforms
- `KiB`/`MiB`/`GiB`/`TiB` are binary multiples (power-of-two), not SI decimal

---

## TypeTraits (`<AK/Core/TypeTraits.hpp>`)

### Summary

GameAK-namespaced wrappers around `<type_traits>` plus `Move`, `Forward`, and `Swap` utilities.
Provides type metafunctions, CV/reference manipulation, type category queries, and triviality checks.

### Guarantees

- all metafunctions produce the same results as their `std::` equivalents
- `IsTriviallyRelocatable` is an alias for `IsTriviallyCopyable` — conservative by design
- `Move`, `Forward`, `Swap` are `constexpr` and `noexcept`

### Non-Guarantees

- type traits reflect compile-time type properties only — no runtime behavior validation
- `IsTriviallyRelocatable` is not based on `std::is_trivially_relocatable` (not yet standard)

### Failure Semantics

- no runtime failure paths
- template instantiation with incomplete types may produce hard compilation errors

### Complexity

All metafunctions are evaluated at compile time — zero runtime cost.

### Threading

All metafunctions and utilities are stateless — no thread-safety concerns.

### Valid Usage

```cpp
static_assert(IsIntegral<int>);
static_assert(!IsFloatingPoint<int>);
static_assert(IsSame<int, int>);
static_assert(IsTriviallyCopyable<int>);

int x = 1, y = 2;
Swap(x, y); // x=2, y=1

struct NonTrivial { ~NonTrivial() {} };
static_assert(!IsTriviallyDestructible<NonTrivial>);
```

### Invariants

- `IsTriviallyRelocatable<T>` is equivalent to `IsTriviallyCopyable<T>`
- `Move` casts to rvalue reference without destroying the source
- `Forward` preserves value category for perfect forwarding

### Integration Notes

- `IsTriviallyDestructible` is used by `Optional` and `PoolAllocator::release<T>()` to elide destructor calls
- `Move`/`Forward`/`Swap` are used throughout GameAK internals
- all variable templates use `inline constexpr bool` — linkage is not an issue

---

## Macros (`<AK/Core/Macros.hpp>`)

### Summary

Compiler-specific macros for alignment, inlining, branch prediction, and debug breaks.
Adapts to MSVC, Clang, and GCC across x86_64 and ARM64.

### Guarantees

- `GAMEAK_ALIGN_CACHE` expands to `alignas(64)`
- `GAMEAK_ALIGN_SIMD` expands to `alignas(32)` on x86_64, `alignas(16)` on ARM64
- `GAMEAK_FORCE_INLINE` forces inlining on all supported compilers
- `GAMEAK_DEBUG_BREAK()` halts execution in debuggers on all supported platforms
- `GAMEAK_LIKELY`/`GAMEAK_UNLIKELY` produce branch prediction hints on Clang/GCC, are no-ops on MSVC

### Non-Guarantees

- `GAMEAK_FORCE_INLINE` is a hint — the compiler may ignore it
- `GAMEAK_LIKELY`/`GAMEAK_UNLIKELY` have no effect on MSVC
- `GAMEAK_DEBUG_BREAK()` behavior is platform-specific: debugger breakpoint or `__builtin_trap`

### Failure Semantics

- no runtime failure paths — macros expand to compiler-specific constructs
- incorrect usage (e.g., alignment value that is not a power of two) produces a compilation error

### Complexity

All macros expand at compile time — zero runtime cost.

### Threading

Macros are text expansion — no thread-safety concerns.

### Valid Usage

```cpp
struct GAMEAK_ALIGN_CACHE AlignedData {
    float x, y, z, w;
};

GAMEAK_FORCE_INLINE void hot_function() {
    // ...
}

if (GAMEAK_UNLIKELY(ptr == nullptr)) {
    GAMEAK_DEBUG_BREAK();
    return;
}
```

### Invariants

- `GAMEAK_CACHE_LINE_SIZE` is always 64
- exactly one of `GAMEAK_ARCH_X86_64` or `GAMEAK_ARCH_ARM64` is defined when this header is included
- `GAMEAK_ALIGN_SIMD` adapts to the target architecture at compile time

### Integration Notes

- consumed by `ArenaAllocator`, `PoolAllocator`, and other performance-sensitive types
- `GAMEAK_DEBUG_BREAK()` is used by `Optional::value()` and `Memory::Debug` validation functions
- should be included through `AK/Core/Macros.hpp` rather than platform headers directly
