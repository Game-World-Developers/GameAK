#pragma once

#include <cstdint>

namespace gameak::runtime {

inline constexpr uint32_t kDefaultChunkSize = 8;

enum class LayoutStrategy : uint32_t {
    AoS,
    SoA,
    AoSoA,
};

struct AoSoAConfig {
    uint32_t chunk_size{kDefaultChunkSize};
};

inline const char* layout_strategy_name(LayoutStrategy s) {
    switch (s) {
        case LayoutStrategy::AoS:   return "AoS";
        case LayoutStrategy::SoA:   return "SoA";
        case LayoutStrategy::AoSoA: return "AoSoA";
    }
    return "Unknown";
}

} // namespace gameak::runtime
