#pragma once

#include "block_type.h"
#include "data_block.h"
#include "GameAk/Core/identity.h"

#include <cstdint>
#include <unordered_map>

namespace gameak::runtime {

class StateView {
public:
    explicit StateView(const std::unordered_map<core::Identity, DataBlock>& blocks,
                       const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
                       float time_delta = 0.016f)
        : blocks_{blocks}, types_{types}, time_delta_{time_delta} {}

    bool has_block(core::Identity identity) const {
        return blocks_.contains(identity);
    }

    const DataBlock* get_block(core::Identity identity) const {
        auto it = blocks_.find(identity);
        return it != blocks_.end() ? &it->second : nullptr;
    }

    size_t block_count() const { return blocks_.size(); }

    bool has_type(uint32_t type_id) const {
        return types_.contains(type_id);
    }

    const BlockTypeDescriptor* type_info(uint32_t type_id) const {
        auto it = types_.find(type_id);
        return it != types_.end() ? &it->second : nullptr;
    }

    float time_delta() const { return time_delta_; }

private:
    const std::unordered_map<core::Identity, DataBlock>& blocks_;
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types_;
    float time_delta_{0.016f};
};

} // namespace gameak::runtime
