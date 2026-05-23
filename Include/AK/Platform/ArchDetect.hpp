/// @file
/// @brief Platform architecture detection.
///
/// Automatically detects the target CPU architecture at compile time
/// and defines @c GAMEAK_ARCH_64_BIT when a supported 64-bit architecture
/// is found.

#pragma once

#if defined(__x86_64__) || defined(_M_X64) || defined(__aarch64__)
/// Defined when a supported 64-bit architecture (x86-64 or AArch64) is detected.
#define GAMEAK_ARCH_64_BIT
#else
#error "GameAK: Unsupported architecture. GameAK requires a 64-bit platform."
#endif
