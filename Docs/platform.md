# Platform Detection

Three headers provide compile-time platform detection for CPU architecture, compiler, and operating system. Consumed by `Macros.hpp` to adapt compiler-specific behavior.

## ArchDetect (`<AK/Platform/ArchDetect.hpp>`)

### Summary

Detects CPU architecture at compile time and defines the corresponding macro.
Exactly one of `GAMEAK_ARCH_X86_64` or `GAMEAK_ARCH_ARM64` is defined.
`GAMEAK_ARCH_64_BIT` is always defined — GameAK requires 64-bit platforms.

### Guarantees

- exactly one architecture macro is defined after inclusion
- `GAMEAK_ARCH_64_BIT` is always defined
- unrecognized architectures produce a `#error`

### Non-Guarantees

- 32-bit architectures are unconditionally rejected
- no runtime CPU feature detection (e.g., AVX2, NEON) — this is handled by the Backend module

### Failure Semantics

- unsupported architecture produces a hard compilation error via `#error`

### Complexity

All detection is evaluated at compile time — zero runtime cost.

### Valid Usage

```cpp
#if defined(GAMEAK_ARCH_X86_64)
    // x86-64 specific code
#elif defined(GAMEAK_ARCH_ARM64)
    // ARM64 specific code
#endif
```

### Invariants

- inclusion order does not affect detection results
- exactly one of `GAMEAK_ARCH_X86_64` or `GAMEAK_ARCH_ARM64` is always defined

---

## CompilerDetect (`<AK/Platform/CompilerDetect.hpp>`)

### Summary

Detects the compiler at compile time and defines exactly one macro.
Detection order: MSVC first (with clang-cl guard), then Clang, then GCC (fallback).

### Guarantees

- exactly one compiler macro is defined after inclusion
- clang-cl is correctly identified as `GAMEAK_COMPILER_MSVC` (Clang on `_MSC_VER` is not treated as `GAMEAK_COMPILER_CLANG`)
- unrecognized compilers produce a `#error`

### Non-Guarantees

- no support for ICC, XL C/C++, or other non-MSVC/Clang/GCC compilers

### Failure Semantics

- unsupported compiler produces a hard compilation error via `#error`

### Complexity

All detection is evaluated at compile time — zero runtime cost.

### Valid Usage

```cpp
#if defined(GAMEAK_COMPILER_CLANG)
    // Clang-specific workaround
#elif defined(GAMEAK_COMPILER_GCC)
    // GCC-specific workaround
#endif
```

### Invariants

- detection order is: `_MSC_VER && !__clang__` → MSVC, `__clang__` → Clang, `__GNUC__` → GCC
- exactly one macro is always defined

---

## OsDetect (`<AK/Platform/OsDetect.hpp>`)

### Summary

Detects the operating system at compile time and defines exactly one macro.
Windows is checked first (some Windows toolchains define POSIX compatibility macros).

### Guarantees

- exactly one OS macro is defined after inclusion
- unrecognized operating systems produce a `#error`

### Non-Guarantees

- no support for FreeBSD, OpenBSD, or other non-Windows/Linux/macOS systems
- no runtime OS detection

### Failure Semantics

- unsupported OS produces a hard compilation error via `#error`

### Complexity

All detection is evaluated at compile time — zero runtime cost.

### Valid Usage

```cpp
#if defined(GAMEAK_OS_WINDOWS)
    // Windows-specific code
#elif defined(GAMEAK_OS_LINUX)
    // Linux-specific code
#endif
```

### Invariants

- detection order is: `_WIN32` → Windows, `__linux__` → Linux, `__APPLE__` → macOS
- exactly one macro is always defined

## Integration Notes

- platform detection headers are primarily consumed by `Macros.hpp` — include `Macros.hpp` instead of platform headers directly
- `GAMEAK_ALIGN_SIMD` uses architecture detection to select SIMD alignment
- `GAMEAK_DEBUG_BREAK()` uses architecture detection for platform-specific breakpoint instructions
- `GAMEAK_FORCE_INLINE`, `GAMEAK_NO_INLINE`, `GAMEAK_LIKELY`/`GAMEAK_UNLIKELY` use compiler detection
- OS detection is reserved for future OS-specific features (virtual memory, thread affinity)
