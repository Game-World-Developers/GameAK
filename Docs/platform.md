# Platform Detection

Three headers provide compile-time platform detection. They are consumed by `Macros.hpp` to adapt compiler-specific behavior.

## ArchDetect (`<AK/Platform/ArchDetect.hpp>`)

Detects the CPU architecture and defines the corresponding macro:

| Architecture | Macro Defined | Check |
|-------------|--------------|-------|
| x86-64 | `GAMEAK_ARCH_X86_64` | `__x86_64__` or `_M_X64` |
| ARM64 | `GAMEAK_ARCH_ARM64` | `__aarch64__` or `_M_ARM64` |
| Other | — | `#error` |

Additionally, `GAMEAK_ARCH_64_BIT` is always defined (GameAK only supports 64-bit platforms).

```cpp
#include <AK/Platform/ArchDetect.hpp>

#if defined(GAMEAK_ARCH_X86_64)
  // x86-64 specific code
#elif defined(GAMEAK_ARCH_ARM64)
  // ARM64 specific code
#endif
```

## CompilerDetect (`<AK/Platform/CompilerDetect.hpp>`)

Detects the compiler and defines exactly one macro:

| Compiler | Macro Defined | Check |
|----------|--------------|-------|
| MSVC | `GAMEAK_COMPILER_MSVC` | `_MSC_VER` and not `__clang__` |
| Clang | `GAMEAK_COMPILER_CLANG` | `__clang__` |
| GCC | `GAMEAK_COMPILER_GCC` | `__GNUC__` (fallback) |
| Other | — | `#error` |

The detection order matters: MSVC is checked first (with a guard against clang-cl false positives), then Clang (checked before GCC because Clang also defines `__GNUC__` for compatibility), then GCC.

```cpp
#include <AK/Platform/CompilerDetect.hpp>

#if defined(GAMEAK_COMPILER_CLANG)
  // Clang-specific workaround
#endif
```

## OsDetect (`<AK/Platform/OsDetect.hpp>`)

Detects the operating system and defines exactly one macro:

| OS | Macro Defined | Check |
|----|--------------|-------|
| Windows | `GAMEAK_OS_WINDOWS` | `_WIN32` |
| Linux | `GAMEAK_OS_LINUX` | `__linux__` |
| macOS | `GAMEAK_OS_MACOS` | `__APPLE__` |
| Other | — | `#error` |

Windows is checked first because some Windows toolchains define POSIX compatibility macros.

```cpp
#include <AK/Platform/OsDetect.hpp>

#if defined(GAMEAK_OS_WINDOWS)
  // Windows-specific code
#elif defined(GAMEAK_OS_LINUX)
  // Linux-specific code
#endif
```

## How They Are Consumed

These platform detection headers are primarily consumed by `Macros.hpp`:

```
ArchDetect.hpp ──┐
CompilerDetect.hpp ── Macros.hpp
OsDetect.hpp ────┘
```

`Macros.hpp` uses the architecture defines for `GAMEAK_ALIGN_SIMD` and `GAMEAK_DEBUG_BREAK()`, and the compiler defines for `GAMEAK_FORCE_INLINE`, `GAMEAK_NO_INLINE`, `GAMEAK_LIKELY`/`GAMEAK_UNLIKELY`, and `GAMEAK_DEBUG_BREAK()`.

The OS detection is currently reserved for future OS-specific features (e.g., virtual memory allocation, thread affinity).
