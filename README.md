# GameAK

GameAK is a C++20 library heavily inspired by SerenityOS AK, designed for high-performance and data-oriented game development.

## Design Philosophy

- **Data-Oriented Design (DOD)** — flat and contiguous memory layouts, cache-friendly access patterns
- **No exceptions, no RTTI** — predictable codegen, no hidden cost paths
- **Explicit memory ownership** — allocators operate on externally-owned buffers
- **Zero-overhead abstractions** — you don't pay for what you don't use
- **Runtime-dispatchable SIMD** — transparent SSE/AVX/NEON acceleration via VTable
- **Deterministic** — IEEE 754 enforced via static_assert, predictable performance

## Quick Start

```bash
# Build (debug)
make

# Build + run tests
make tests MODE=release

# Build static library only
make lib

# Install to a prefix
make install INSTALL_PREFIX=/opt/GameAK
```

Minimal usage:

```cpp
#include <AK/Core/Types.hpp>
#include <AK/Memory/ArenaAllocator.hpp>
#include <AK/Backend/Backend.hpp>

int main() {
  GameAK::Backend::init();

  u8 buffer[GameAK::KiB];
  GameAK::ArenaAllocator arena(buffer, sizeof(buffer));

  int* numbers = arena.allocate<int>(10);
  numbers[0] = 42;
}
```

Compile:

```bash
g++ -std=c++20 -I/path/to/GameAK/include -fno-exceptions -fno-rtti \
    -L/path/to/GameAK/lib -lGameAK main.cpp -o main
```

Or with pkg-config:

```bash
g++ $(pkg-config --cflags --libs GameAK) main.cpp -o main
```

## Modules

| Module | Headers | Description |
|--------|---------|-------------|
| Platform | `ArchDetect`, `CompilerDetect`, `OsDetect` | Compile-time CPU/compiler/OS detection |
| Core | `Types`, `TypeTraits`, `Macros`, `Optional` | Fundamental types, metaprogramming, utilities |
| Bits | `BitOps`, `BitMask`, `BitArray`, `BitPack` | Bit-level operations and containers |
| Memory | `ArenaAllocator`, `PoolAllocator`, `AllocatorConcept` | Custom allocators |
| Backend | `Backend`, `ScalarBackend` | Runtime-dispatch VTable for memory ops |

## Documentation

See the [docs/](/Docs/) directory for detailed documentation:

- [Overview](/Docs/overview.md) — architecture and design
- [Installation](/Docs/installation.md) — build, install, pkg-config
- [Usage Guide](/Docs/usage-guide.md) — consuming GameAK in your project
- [Core Types](/Docs/core-types.md) — Types, TypeTraits, Macros
- [Bits](/Docs/bits.md) — BitOps, BitMask, BitArray, BitPack
- [Optional](/Docs/optional.md) — Optional\<T\>
- [Memory](/Docs/memory.md) — ArenaAllocator, PoolAllocator
- [Backend](/Docs/backend.md) — Runtime dispatch VTable
- [Platform](/Docs/platform.md) — Architecture, compiler, OS detection

## Requirements

- C++20 compiler (Clang 14+, GCC 12+, MSVC 2022+)
- 64-bit platform (x86_64 or ARM64)
- No external dependencies (cest.h included for testing)
