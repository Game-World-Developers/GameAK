#pragma once

#include <cstdint>

namespace gameak::runtime {

inline constexpr uint32_t kDefaultChunkSize = 8;
inline constexpr uint32_t kDefaultArchetypeChunkSize = 64;

enum class LayoutStrategy : uint32_t {
    AoS,
    SoA,
    AoSoA,
    Archetype,
};

struct AoSoAConfig {
    uint32_t chunk_size{kDefaultChunkSize};
};

struct ArchetypeConfig {
    uint32_t chunk_size{kDefaultArchetypeChunkSize};
};

inline const char* layout_strategy_name(LayoutStrategy s) {
    switch (s) {
        case LayoutStrategy::AoS:       return "AoS";
        case LayoutStrategy::SoA:       return "SoA";
        case LayoutStrategy::AoSoA:     return "AoSoA";
        case LayoutStrategy::Archetype: return "Archetype";
    }
    return "Unknown";
}

} // namespace gameak::runtime
