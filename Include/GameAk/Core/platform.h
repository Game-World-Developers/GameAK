#pragma once

#include <cstdint>
#include <string>

namespace gameak::core {

enum class Architecture : uint8_t {
    Unknown,
    X86_64,
    AArch64,
    Wasm32,
    Wasm64,
};

enum class Compiler : uint8_t {
    Unknown,
    Clang,
    GCC,
    MSVC,
};

enum class OperatingSystem : uint8_t {
    Unknown,
    Linux,
    Windows,
    macOS,
    BSD,
};

struct PlatformInfo {
    Architecture arch{Architecture::Unknown};
    Compiler compiler{Compiler::Unknown};
    OperatingSystem os{OperatingSystem::Unknown};

    bool has_sse2{false};
    bool has_avx2{false};
    bool has_avx512{false};
    bool has_neon{false};
    bool has_wasm_simd{false};

    std::string to_string() const;
};

inline PlatformInfo detect_platform() {
    PlatformInfo info;

#if defined(__x86_64__) || defined(_M_AMD64)
    info.arch = Architecture::X86_64;
#elif defined(__aarch64__) || defined(_M_ARM64)
    info.arch = Architecture::AArch64;
#elif defined(__wasm32__)
    info.arch = Architecture::Wasm32;
#elif defined(__wasm64__)
    info.arch = Architecture::Wasm64;
#endif

#if defined(__clang__)
    info.compiler = Compiler::Clang;
#elif defined(__GNUC__) || defined(__GNUG__)
    info.compiler = Compiler::GCC;
#elif defined(_MSC_VER)
    info.compiler = Compiler::MSVC;
#endif

#if defined(__linux__)
    info.os = OperatingSystem::Linux;
#elif defined(_WIN32)
    info.os = OperatingSystem::Windows;
#elif defined(__APPLE__)
    info.os = OperatingSystem::macOS;
#elif defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
    info.os = OperatingSystem::BSD;
#endif

#if defined(__SSE2__) || defined(_M_AMD64) || defined(__x86_64__)
    info.has_sse2 = true;
#endif
#if defined(__AVX2__)
    info.has_avx2 = true;
#endif
#if defined(__AVX512F__)
    info.has_avx512 = true;
#endif
#if defined(__ARM_NEON) || defined(__ARM_NEON__)
    info.has_neon = true;
#endif
#if defined(__wasm_simd128__)
    info.has_wasm_simd = true;
#endif

    return info;
}

inline std::string PlatformInfo::to_string() const {
    std::string result;

    switch (os) {
        case OperatingSystem::Linux:   result += "Linux"; break;
        case OperatingSystem::Windows: result += "Windows"; break;
        case OperatingSystem::macOS:   result += "macOS"; break;
        case OperatingSystem::BSD:     result += "BSD"; break;
        default: result += "Unknown OS"; break;
    }

    result += " / ";

    switch (compiler) {
        case Compiler::Clang: result += "Clang"; break;
        case Compiler::GCC:   result += "GCC"; break;
        case Compiler::MSVC:  result += "MSVC"; break;
        default: result += "Unknown compiler"; break;
    }

    result += " / ";

    switch (arch) {
        case Architecture::X86_64:  result += "x86_64"; break;
        case Architecture::AArch64: result += "AArch64"; break;
        case Architecture::Wasm32:  result += "Wasm32"; break;
        case Architecture::Wasm64:  result += "Wasm64"; break;
        default: result += "Unknown arch"; break;
    }

    if (has_sse2)    result += " +SSE2";
    if (has_avx2)    result += " +AVX2";
    if (has_avx512)  result += " +AVX512";
    if (has_neon)    result += " +NEON";
    if (has_wasm_simd) result += " +WasmSIMD";

    return result;
}

} // namespace gameak::core
