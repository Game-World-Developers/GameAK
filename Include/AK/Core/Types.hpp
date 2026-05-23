/// @file
/// @brief Fundamental integer, floating-point, and size types for GameAK.
///
/// Provides a stable set of fixed-width type aliases (@c i8, @c u8, @c i16,
/// @c u16, @c i32, @c u32, @c i64, @c u64, @c f32, @c f64), pointer-width
/// types (@c usize, @c isize, @c uptr, @c iptr), and byte-size constants
/// (@c KiB, @c MiB, @c GiB, @c TiB).
///
/// Compile-time assertions guarantee the expected sizes and IEEE 754
/// conformance for deterministic simulation.

#pragma once

#include <AK/Platform/ArchDetect.hpp>

#include <cstddef>
#include <cstdint>
#include <limits>

/// @todo Separate architecture-specific types to support 32-bit, 64-bit,
///       128-bit and multiple CPU architectures.

namespace GameAK {

/// Signed 8-bit integer.
using i8 = std::int8_t;
/// Unsigned 8-bit integer.
using u8 = std::uint8_t;
/// Signed 16-bit integer.
using i16 = std::int16_t;
/// Unsigned 16-bit integer.
using u16 = std::uint16_t;
/// Signed 32-bit integer.
using i32 = std::int32_t;
/// Unsigned 32-bit integer.
using u32 = std::uint32_t;
/// Signed 64-bit integer.
using i64 = std::int64_t;
/// Unsigned 64-bit integer.
using u64 = std::uint64_t;

/// 32-bit IEEE 754 floating-point.
using f32 = float;
/// 64-bit IEEE 754 floating-point.
using f64 = double;

/// Unsigned pointer-width size type (64-bit on supported platforms).
using usize = std::size_t;
/// Signed pointer-width difference type.
using isize = std::ptrdiff_t;

/// Unsigned integer capable of holding a pointer.
using uptr = std::uintptr_t;
/// Signed integer capable of holding a pointer.
using iptr = std::intptr_t;

/// Individual byte (C++17 @c std::byte).
using byte = std::byte;

/// 1 kibibyte (2^10).
constexpr usize KiB = usize{1024};
/// 1 mebibyte (2^20).
constexpr usize MiB = usize{1024} * KiB;
/// 1 gibibyte (2^30).
constexpr usize GiB = usize{1024} * MiB;
/// 1 tebibyte (2^40).
constexpr usize TiB = usize{1024} * GiB;

static_assert(sizeof(usize) >= 8,
              "GameAK: Requires a 64-bit platform (sizeof(usize) >= 8)");

static_assert(sizeof(u8) == 1, "GameAK: u8 must be exactly 1 byte");
static_assert(sizeof(u16) == 2, "GameAK: u16 must be exactly 2 bytes");
static_assert(sizeof(u32) == 4, "GameAK: u32 must be exactly 4 bytes");
static_assert(sizeof(u64) == 8, "GameAK: u64 must be exactly 8 bytes");

static_assert(sizeof(f32) == 4, "GameAK: f32 must be exactly 4 bytes");
static_assert(sizeof(f64) == 8, "GameAK: f64 must be exactly 8 bytes");

static_assert(std::numeric_limits<f32>::is_iec559,
              "GameAK: f32 must conform to IEEE 754 — required for "
              "deterministic simulation");
static_assert(std::numeric_limits<f64>::is_iec559,
              "GameAK: f64 must conform to IEEE 754 — required for "
              "deterministic simulation");

static_assert(sizeof(usize) == sizeof(void *),
              "GameAK: usize must match pointer width — mmap offsets and arena "
              "arithmetic depend on this");
static_assert(
    sizeof(uptr) == sizeof(void *),
    "GameAK: uptr must match pointer width — required for safe pointer "
    "<-> integer casts");

} // namespace GameAK
