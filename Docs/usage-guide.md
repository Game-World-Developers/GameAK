# Usage Guide

## Including Headers

All public headers are under `Include/AK/` and included as:

```cpp
#include <AK/Core/Types.hpp>
#include <AK/Memory/ArenaAllocator.hpp>
#include <AK/Backend/Backend.hpp>
```

## Compiler Flags

GameAK requires C++20 and specific compiler flags:

```
-std=c++20 -fno-exceptions -fno-rtti
```

These are already used when building the library itself, and must also be used by any translation unit that includes GameAK headers (since headers use `GAMEAK_FORCE_INLINE` and similar macros that depend on the compilation environment).

## Backend Initialization

Before using any memory or bitset operations that go through the Backend, call:

```cpp
#include <AK/Backend/Backend.hpp>

GameAK::Backend::init();
```

This sets the global VTable to the scalar backend. In the future it will perform runtime CPU feature detection to select SSE/AVX/NEON paths.

## Complete Example

```cpp
#include <AK/Backend/Backend.hpp>
#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Bits/BitMask.hpp>
#include <AK/Core/Optional.hpp>
#include <AK/Core/Types.hpp>
#include <AK/Core/TypeTraits.hpp>
#include <AK/Memory/ArenaAllocator.hpp>
#include <AK/Memory/PoolAllocator.hpp>

// Example: entity state enum for BitMask
enum class EntityFlag : GameAK::u64 {
  Active      = 0,
  Visible     = 1,
  Collidable  = 2,
  Damaged     = 3,
};

int main() {
  // Initialize runtime backend
  GameAK::Backend::init();

  // --- Arena Allocator ---
  GameAK::u8 arena_buf[GameAK::KiB * 4];
  GameAK::ArenaAllocator arena(arena_buf, sizeof(arena_buf));

  // Typed allocation
  int* numbers = arena.allocate<int>(100);
  float* positions = arena.allocate<float>(256);

  // Checkpoint / restore for scoped work
  auto checkpoint = arena.save();
  // ... temporary allocations ...
  arena.restore(checkpoint); // reclaims temp memory

  // --- Pool Allocator ---
  GameAK::u8 pool_buf[GameAK::KiB];
  GameAK::PoolAllocator pool(pool_buf, sizeof(pool_buf), 64, 64);

  void* block = pool.acquire();
  // ... use block ...
  pool.release(block);

  // --- BitMask ---
  auto flags = GameAK::Bits::BitMask<EntityFlag>{};
  flags.set(EntityFlag::Active);
  flags.set(EntityFlag::Visible);
  if (flags.has(EntityFlag::Active)) {
    // ...
  }

  // --- BitOps ---
  static_assert(GameAK::Bits::is_power_of_two(16u));
  GameAK::usize aligned = GameAK::Bits::align_up(15u, 16u); // 16

  // --- Optional ---
  GameAK::Optional<int> maybe_value;
  maybe_value = 42;
  if (maybe_value) {
    int v = maybe_value.value(); // 42
  }

  return 0;
}
```

## Consuming via Library

### With pkg-config (recommended)

```bash
g++ $(pkg-config --cflags --libs GameAK) main.cpp -o main
```

### Manual flags

```bash
g++ -std=c++20 -I/opt/GameAK/include -fno-exceptions -fno-rtti \
    -L/opt/GameAK/lib -lGameAK main.cpp -o main
```

### With CMake

If you use CMake in your project, add GameAK as a custom target or use `find_library`:

```cmake
find_library(GAMEAK_LIB GameAK REQUIRED)
find_path(GAMEAK_INCLUDE_DIR AK/Core/Types.hpp REQUIRED)

add_executable(my_app main.cpp)
target_include_directories(my_app PRIVATE ${GAMEAK_INCLUDE_DIR})
target_link_libraries(my_app PRIVATE ${GAMEAK_LIB})
target_compile_options(my_app PRIVATE -fno-exceptions -fno-rtti)
```

## Multi-Architecture Considerations

GameAK supports x86_64 and ARM64, but requires a 64-bit platform (`sizeof(usize) >= 8` is enforced via `static_assert`). The SIMD alignment macro (`GAMEAK_ALIGN_SIMD`) adapts automatically: 32 bytes on x86_64, 16 bytes on ARM64.

## Debug vs Release

In debug builds (`GAMEAK_DEBUG_VALIDATE` defined):
- Allocator validation (alignment, bounds, double-free detection) is active
- `Optional::value()` triggers `GAMEAK_DEBUG_BREAK()` on empty access
- Most performance overhead is limited to assertion-style checks

In release builds (`NDEBUG` defined):
- All validation is compiled out
- `Optional::value()` empty access is undefined behavior
- Maximum performance
