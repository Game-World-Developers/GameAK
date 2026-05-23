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

#define AK_CACHE_LINE_SIZE 64
#define AK_ALIGN(n) alignas(n)
#define AK_ALIGN_CACHE alignas(AK_CACHE_LINE_SIZE)

#if defined(GAMEAK_COMPILER_MSVC)
#define AK_FORCE_INLINE __forceinline
#define AK_NO_INLINE __declspec(noinline)
#elif defined(GAMEAK_COMPILER_CLANG) || defined(GAMEAK_COMPILER_GCC)
#define AK_FORCE_INLINE __attribute__((always_inline)) inline
#define AK_NO_INLINE __attribute__((noinline))
#endif

#if defined(GAMEAK_COMPILER_CLANG) || defined(GAMEAK_COMPILER_GCC)
#define AK_LIKELY(x) __builtin_expect(!!(x), 1)
#define AK_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define AK_LIKELY(x) (x)
#define AK_UNLIKELY(x) (x)
#endif

#if defined(GAMEAK_ARCH_X86_64)
#define AK_ALIGN_SIMD alignas(32)
#elif defined(GAMEAK_ARCH_ARM64)
#define AK_ALIGN_SIMD alignas(16)
#endif
