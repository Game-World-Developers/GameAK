#pragma once

#include <cstddef>
#include <new>

namespace gameak::core::inline_policy {

#ifdef __cpp_lib_hardware_interference_size
    inline constexpr size_t kCacheLineSize = std::hardware_destructive_interference_size;
#else
    inline constexpr size_t kCacheLineSize = 64;
#endif

template <typename T>
struct default_inline_n {
    static constexpr size_t kTarget = sizeof(T) == 1 ? 64 : 32;
    static constexpr size_t value = (kTarget + sizeof(T) - 1) / sizeof(T);
};

template <>
struct default_inline_n<uint64_t> {
    static constexpr size_t value = 2;
};

} // namespace gameak::core::inline_policy

#define GAME_AK_INLINE_POLICY(T, N)                                            \
    template <> struct ::gameak::core::inline_policy::default_inline_n<T> {    \
        static constexpr ::size_t value = N;                                   \
    }
