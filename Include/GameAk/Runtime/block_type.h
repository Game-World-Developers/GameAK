#pragma once

#include "layout_strategy.h"

#include <cstdint>
#include <string>
#include <vector>

namespace gameak::runtime {

struct FieldDescriptor {
    const char* name;
    size_t offset;
    size_t size;
    size_t alignment;
};

struct BlockTypeDescriptor {
    uint32_t type_id;
    size_t size;
    size_t alignment;
    const char* name;
    LayoutStrategy layout{LayoutStrategy::AoS};
    AoSoAConfig aosoa_config{};
    std::vector<FieldDescriptor> fields;
    bool ephemeral{false};
};

} // namespace gameak::runtime
