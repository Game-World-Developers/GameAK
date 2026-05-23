/// @file
/// @brief Compiler detection.
///
/// Automatically detects the compiler at compile time and defines
/// one of the following:
///   @c GAMEAK_COMPILER_MSVC   — Microsoft Visual C++
///   @c GAMEAK_COMPILER_CLANG  — Clang / Clang-cl
///   @c GAMEAK_COMPILER_GCC    — GCC
///
/// Clang is checked before GCC because Clang also defines @c __GNUC__
/// for compatibility reasons, which would cause a false GCC detection.

#pragma once

#if defined(_MSC_VER) && !defined(__clang__)
#define GAMEAK_COMPILER_MSVC
#elif defined(__clang__)
#define GAMEAK_COMPILER_CLANG
#elif defined(__GNUC__)
#define GAMEAK_COMPILER_GCC
#else
#error "GameAK: Unsupported compiler. GameAK requires MSVC, Clang, or GCC."
#endif
