/// @file
/// @brief Compiler-specific macros and optimization hints.
///
/// Provides portable abstractions over compiler intrinsics and attributes.
/// All detection is delegated to the Platform headers — this file only
/// consumes their defines and maps them to usable macros.

#pragma once

#include <AK/Platform/ArchDetect.hpp>
#include <AK/Platform/CompilerDetect.hpp>
#include <AK/Platform/OsDetect.hpp>

#define GameAK_CACHE_LINE_SIZE 64
#define GameAK_ALIGN(n) alignas(n)
#define GameAK_ALIGN_CACHE alignas(GameAK_CACHE_LINE_SIZE)

#if defined(GAMEAK_COMPILER_MSVC)
#define GameAK_FORCE_INLINE __forceinline
#define GameAK_NO_INLINE __declspec(noinline)
#elif defined(GAMEAK_COMPILER_CLANG) || defined(GAMEAK_COMPILER_GCC)
#define GameAK_FORCE_INLINE __attribute__((always_inline)) inline
#define GameAK_NO_INLINE __attribute__((noinline))
#endif

#if defined(GAMEAK_COMPILER_CLANG) || defined(GAMEAK_COMPILER_GCC)
#define GameAK_LIKELY(x) __builtin_expect(!!(x), 1)
#define GameAK_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define GameAK_LIKELY(x) (x)
#define GameAK_UNLIKELY(x) (x)
#endif

#if defined(GAMEAK_ARCH_X86_64)
#define GameAK_ALIGN_SIMD alignas(32)
#elif defined(GAMEAK_ARCH_ARM64)
#define GameAK_ALIGN_SIMD alignas(16)
#endif

#if defined(GAMEAK_COMPILER_MSVC)
#define AK_DEBUG_BREAK() __debugbreak()
#elif defined(GAMEAK_COMPILER_CLANG) || defined(GAMEAK_COMPILER_GCC)
#if defined(GAMEAK_ARCH_X86_64)
#define AK_DEBUG_BREAK() __asm__ volatile("int $0x03")
#elif defined(GAMEAK_ARCH_ARM64)
#define AK_DEBUG_BREAK() __asm__ volatile("brk #0")
#endif
#endif
