/// @file
/// @brief Operating system detection.
///
/// Automatically detects the target OS at compile time and defines
/// one of the following:
///   @c GAMEAK_OS_WINDOWS — Microsoft Windows (32-bit or 64-bit)
///   @c GAMEAK_OS_LINUX   — Linux
///   @c GAMEAK_OS_MACOS   — Apple macOS
///
/// Windows is checked before Linux and macOS because some toolchains
/// targeting Windows may define POSIX-like macros in addition to
/// the Windows-specific ones.
#pragma once

#if defined(_WIN32) || defined(_WIN64)
#define GAMEAK_OS_WINDOWS
#elif defined(__linux__)
#define GAMEAK_OS_LINUX
#elif defined(__APPLE__) && defined(__MACH__)
#define GAMEAK_OS_MACOS
#else
#error                                                                         \
    "GameAK: Unsupported operating system. GameAK requires Windows, Linux, or macOS."
#endif
