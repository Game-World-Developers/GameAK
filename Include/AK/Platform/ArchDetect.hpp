/// @file
/// @brief Platform architecture detection.
///
/// Automatically detects the target CPU architecture at compile time
/// and defines @c GAMEAK_ARCH_64_BIT when a supported 64-bit architecture
/// is found.

#pragma once

#if defined(__x86_64__) || defined(_M_X64)
#define GAMEAK_ARCH_64_BIT
#define GAMEAK_ARCH_X86_64
#elif defined(__aarch64__) || defined(_M_ARM64)
#define GAMEAK_ARCH_64_BIT
#define GAMEAK_ARCH_ARM64
#else
#error "GameAK: Unsupported architecture. GameAK requires a 64-bit platform."
#endif
