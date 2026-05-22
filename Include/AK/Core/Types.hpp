#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>

namespace AK {

using i8 = std::int8_t;
using u8 = std::uint8_t;

using i16 = std::int16_t;
using u16 = std::uint16_t;

using i32 = std::int32_t;
using u32 = std::uint32_t;

using i64 = std::int64_t;
using u64 = std::uint64_t;

using f32 = float;
using f64 = double;

using usize = std::size_t;
using isize = std::ptrdiff_t;

using uptr = std::uintptr_t;
using iptr = std::intptr_t;

using byte = std::byte;

static_assert(sizeof(u8) == 1);
static_assert(sizeof(u16) == 2);
static_assert(sizeof(u32) == 4);
static_assert(sizeof(u64) == 8);

static_assert(sizeof(f32) == 4);
static_assert(sizeof(f64) == 8);

static_assert(sizeof(usize) == sizeof(void *));
static_assert(sizeof(uptr) == sizeof(void *));

static_assert(std::numeric_limits<f32>::is_iec559);
static_assert(std::numeric_limits<f64>::is_iec559);

constexpr usize KiB = 1024;
constexpr usize MiB = 1024ull * KiB;
constexpr usize GiB = 1024ull * MiB;

} // namespace AK
