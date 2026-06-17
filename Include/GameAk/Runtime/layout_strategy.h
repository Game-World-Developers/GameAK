#pragma once

#include <cstdint>

namespace gameak::runtime {

enum class LayoutStrategy : uint32_t {
    AoS,
    SoA,
    AoSoA,
};

struct AoSoAConfig {
    uint32_t chunk_size{8};
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
