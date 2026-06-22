#pragma once

#include "platform.h"
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <atomic>

namespace gameak::core {

// ---- OsPlatform ----

template<typename Derived>
struct OsPlatform {
    void debug_output(const char* msg) {
        static_cast<Derived*>(this)->debug_output_impl(msg);
    }

    [[noreturn]] void abort() {
        static_cast<Derived*>(this)->abort_impl();
    }
};

struct GenericOs : OsPlatform<GenericOs> {
    void debug_output_impl(const char* msg) {
        std::fputs(msg, stderr);
        std::fputc('\n', stderr);
    }

    [[noreturn]] void abort_impl() {
        std::abort();
    }
};

// ---- CompilerPlatform ----

template<typename Derived>
struct CompilerPlatform {
    void compiler_barrier() {
        static_cast<Derived*>(this)->compiler_barrier_impl();
    }
};

struct GenericCompiler : CompilerPlatform<GenericCompiler> {
    void compiler_barrier_impl() {
        std::atomic_signal_fence(std::memory_order_seq_cst);
    }
};

// ---- CpuPlatform ----

template<typename Derived>
struct CpuPlatform {
    void memcopy(void* dst, const void* src, size_t n) {
        static_cast<Derived*>(this)->memcopy_impl(dst, src, n);
    }

    void memzero(void* dst, size_t n) {
        static_cast<Derived*>(this)->memzero_impl(dst, n);
    }

    size_t cache_line_size() const {
        return static_cast<const Derived*>(this)->cache_line_size_impl();
    }
};

struct GenericCpu : CpuPlatform<GenericCpu> {
    void memcopy_impl(void* dst, const void* src, size_t n) {
        std::memcpy(dst, src, n);
    }

    void memzero_impl(void* dst, size_t n) {
        std::memset(dst, 0, n);
    }

    size_t cache_line_size_impl() const {
        return 64;
    }
};

// ---- Platform aggregation ----
//
// CRTP must not be used outside platform specialization layers.

struct Platform {
    PlatformInfo info;
    GenericOs os;
    GenericCompiler compiler;
    GenericCpu cpu;

    Platform() : info(detect_platform()) {}

    size_t cache_line_size() const { return info.cache_line_size; }

    static Platform& instance() {
        static Platform p;
        return p;
    }
};

} // namespace gameak::core
