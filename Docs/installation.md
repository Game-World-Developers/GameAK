# Installation

## Requirements

- C++20 compiler (Clang 14+ recommended, GCC 12+)
- 64-bit platform (x86_64 or ARM64)
- GNU Make
- No external dependencies

## Build

```bash
# Build and run tests in debug mode (default)
make

# Build and run tests in release mode
make release

# Build with address/undefined/leak sanitizers
make sanitize

# Build only the static library (no tests)
make lib

# Build a specific mode + lib in one command
make lib MODE=release
```

Output:
- Static library: `build/lib/libGameAK.a`
- Object files: `build/obj/src/`
- Test binaries: `build/bin/tests/`

## Install

```bash
# Install to /usr/local (default)
sudo make install

# Install to a custom prefix
make install INSTALL_PREFIX=/opt/GameAK

# Install for packaging (DESTDIR support)
make install INSTALL_PREFIX=/usr DESTDIR=/tmp/staging
```

What gets installed:

```
$INSTALL_PREFIX/
├── include/
│   └── AK/
│       ├── Backend/
│       │   ├── Backend.hpp
│       │   └── ScalarBackend.hpp
│       ├── Core/
│       │   ├── Bits/
│       │   │   ├── BitArray.hpp
│       │   │   ├── BitMask.hpp
│       │   │   ├── BitOps.hpp
│       │   │   └── BitPack.hpp
│       │   ├── Macros.hpp
│       │   ├── Optional.hpp
│       │   ├── Types.hpp
│       │   └── TypeTraits.hpp
│       ├── Memory/
│       │   ├── AllocatorConcept.hpp
│       │   ├── ArenaAllocator.hpp
│       │   ├── MemoryDebug.hpp
│       │   └── PoolAllocator.hpp
│       └── Platform/
│           ├── ArchDetect.hpp
│           ├── CompilerDetect.hpp
│           └── OsDetect.hpp
├── lib/
│   ├── libGameAK.a
│   └── pkgconfig/
│       └── GameAK.pc
```

## Uninstall

```bash
sudo make uninstall

# Or with custom prefix
make uninstall INSTALL_PREFIX=/opt/GameAK
```

## pkg-config

After installation, a `GameAK.pc` file is placed in the pkgconfig directory. If you installed to a non-standard prefix, add it to `PKG_CONFIG_PATH`:

```bash
export PKG_CONFIG_PATH=/opt/GameAK/lib/pkgconfig:$PKG_CONFIG_PATH
pkg-config --cflags --libs GameAK
# Output: -I/opt/GameAK/include -fno-exceptions -fno-rtti -std=c++20 -L/opt/GameAK/lib -lGameAK
```

## Clean

```bash
make clean   # removes build/
```

## Build Profiles

| Profile | Flags |
|---------|-------|
| `debug` (default) | `-O0 -g -DGAMEAK_DEBUG_VALIDATE` |
| `release` | `-O3 -DNDEBUG` |
| `sanitize` | `-O0 -g -DGAMEAK_DEBUG_VALIDATE -fsanitize=address,undefined,leak` |

The `GAMEAK_DEBUG_VALIDATE` define enables runtime validation checks in allocators (alignment, bounds, double-free detection). These are compiled out entirely in release builds.
