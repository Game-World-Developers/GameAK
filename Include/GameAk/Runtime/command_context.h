#pragma once

#include "block_type.h"
#include "data_block.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/rb_tree.h"

#include <cstdint>

namespace gameak::runtime {

class LayoutManager;

struct CommandContext {
    core::rb_tree<core::Identity, DataBlock>& blocks;
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types;
    uint64_t& next_identity;
    core::rb_tree<core::Identity, uint32_t>& identity_types;
    core::rb_tree<uint32_t, size_t>& type_counts;
    LayoutManager* layout_mgr{nullptr};
};

} // namespace gameak::runtime
