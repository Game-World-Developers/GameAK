#pragma once

#include "block_type.h"
#include "data_block.h"
#include "GameAk/Core/identity.h"

#include <cstdint>
#include <unordered_map>

namespace gameak::runtime {

struct Snapshot {
    std::unordered_map<core::Identity, DataBlock> blocks;
    std::unordered_map<uint32_t, BlockTypeDescriptor> types;
    uint64_t next_identity{0};
};

} // namespace gameak::runtime
