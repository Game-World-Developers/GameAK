# Core Types

Three headers provide the foundational type system and macros used by all other GameAK modules.

## Types (`<AK/Core/Types.hpp>`)

Fixed-width integer and floating-point aliases, plus byte-size constants.

### Integer Types

| Alias | Underlying | Size |
|-------|-----------|------|
| `i8` | `std::int8_t` | 1 byte |
| `u8` | `std::uint8_t` | 1 byte |
| `i16` | `std::int16_t` | 2 bytes |
| `u16` | `std::uint16_t` | 2 bytes |
| `i32` | `std::int32_t` | 4 bytes |
| `u32` | `std::uint32_t` | 4 bytes |
| `i64` | `std::int64_t` | 8 bytes |
| `u64` | `std::uint64_t` | 8 bytes |

### Floating-Point Types

| Alias | Underlying | Size | Constraint |
|-------|-----------|------|------------|
| `f32` | `float` | 4 bytes | IEEE 754 (`is_iec559`) |
| `f64` | `double` | 8 bytes | IEEE 754 (`is_iec559`) |

### Pointer-Width Types

| Alias | Underlying | Use Case |
|-------|-----------|----------|
| `usize` | `std::size_t` | Sizes, counts, offsets |
| `isize` | `std::ptrdiff_t` | Signed differences |
| `uptr` | `std::uintptr_t` | Pointer-to-integer casts |
| `iptr` | `std::intptr_t` | Signed pointer-width integer |

### Other

| Alias | Underlying |
|-------|-----------|
| `byte` | `std::byte` |

### Size Constants

```cpp
constexpr usize KiB = 1024;
constexpr usize MiB = 1024 * KiB; // 1,048,576
constexpr usize GiB = 1024 * MiB; // 1,073,741,824
constexpr usize TiB = 1024 * GiB; // 1,099,511,627,776
```

### Example

```cpp
#include <AK/Core/Types.hpp>

static_assert(GameAK::KiB == 1024);

GameAK::u8 buffer[GameAK::MiB];      // 1 MB buffer
GameAK::usize count = 100;
GameAK::f64 delta = 0.016;           // 60 FPS timestep
```

## TypeTraits (`<AK/Core/TypeTraits.hpp>`)

GameAK-namespaced wrappers around `<type_traits>` plus `Move`/`Forward`/`Swap` utilities.

### Metafunctions

```cpp
template<typename T, T V>
struct IntegralConstant;              // compile-time integral constant
using TrueType  = IntegralConstant<bool, true>;
using FalseType = IntegralConstant<bool, false>;

template<typename T>
using TypeIdentity<T> = T;            // identity metafunction

template<bool B, typename T, typename F>
using Conditional = std::conditional_t<B, T, F>;

template<bool B, typename T = void>
using EnableIf = std::enable_if_t<B, T>;
```

### CV/Reference Manipulation

```cpp
RemoveRef<T>       → std::remove_reference_t<T>
RemoveConst<T>     → std::remove_const_t<T>
RemoveCV<T>        → std::remove_cv_t<T>
RemoveCVRef<T>     → std::remove_cvref_t<T>
AddConst<T>        → std::add_const_t<T>
AddLValueRef<T>    → std::add_lvalue_reference_t<T>
AddRValueRef<T>    → std::add_rvalue_reference_t<T>
Decay<T>           → std::decay_t<T>
```

### Type Queries

All are `inline constexpr bool` variable templates:

```cpp
IsSame<T, U>                   // std::is_same_v
IsBaseOf<Base, Derived>        // std::is_base_of_v
IsConvertible<From, To>        // std::is_convertible_v
IsVoid<T>                      // std::is_void_v
IsIntegral<T>                  // std::is_integral_v
IsFloatingPoint<T>             // std::is_floating_point_v
IsPointer<T>                   // std::is_pointer_v
IsEnum<T>                      // std::is_enum_v
IsClass<T>                     // std::is_class_v
IsTriviallyCopyable<T>         // std::is_trivially_copyable_v
IsTriviallyDestructible<T>     // std::is_trivially_destructible_v
IsTriviallyRelocatable<T>      // alias for IsTriviallyCopyable<T>
// ... and more
```

### Utility Functions

```cpp
// Move: casts to rvalue reference
template<typename T>
constexpr RemoveRef<T>&& Move(T&& t) noexcept;

// Forward: perfect forwarding with safety guard
template<typename T>
constexpr T&& Forward(RemoveRef<T>& t) noexcept;
template<typename T>
constexpr T&& Forward(RemoveRef<T>&& t) noexcept;

// Swap: move-based
template<typename T>
constexpr void Swap(T& a, T& b) noexcept;
```

### Example

```cpp
#include <AK/Core/TypeTraits.hpp>

static_assert(GameAK::IsIntegral<int>);
static_assert(!GameAK::IsFloatingPoint<int>);
static_assert(GameAK::IsSame<int, int>);

struct NonTrivial { ~NonTrivial() {} };
static_assert(!GameAK::IsTriviallyDestructible<NonTrivial>);

int x = 1, y = 2;
GameAK::Swap(x, y); // x=2, y=1
```

## Macros (`<AK/Core/Macros.hpp>`)

Portable compiler-specific macros and optimization hints.

### Alignment

| Macro | Effect |
|-------|--------|
| `GAMEAK_CACHE_LINE_SIZE` | 64 (bytes) |
| `GAMEAK_ALIGN(n)` | `alignas(n)` |
| `GAMEAK_ALIGN_CACHE` | `alignas(64)` |
| `GAMEAK_ALIGN_SIMD` | `alignas(32)` on x86_64, `alignas(16)` on ARM64 |

### Optimization Hints

| Macro | Effect |
|-------|--------|
| `GAMEAK_FORCE_INLINE` | `always_inline` attribute (or `__forceinline` on MSVC) |
| `GAMEAK_NO_INLINE` | `noinline` attribute |
| `GAMEAK_LIKELY(x)` | `__builtin_expect(x, true)` — prediction that x is true |
| `GAMEAK_UNLIKELY(x)` | `__builtin_expect(x, false)` — prediction that x is false |

### Debug

| Macro | Effect |
|-------|--------|
| `GAMEAK_DEBUG_BREAK()` | Triggers debugger breakpoint (platform-specific: `int 0x03`, `brk #0`, `__debugbreak()`, or `__builtin_trap()`) |

### Example

```cpp
#include <AK/Core/Macros.hpp>

struct GAMEAK_ALIGN_CACHE AlignedData {
  float x, y, z, w;
};

if (GAMEAK_UNLIKELY(ptr == nullptr)) {
  GAMEAK_DEBUG_BREAK();
  return;
}
```
