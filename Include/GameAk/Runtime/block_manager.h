#pragma once

#include "block_type.h"
#include "data_block.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/rb_tree.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <functional>

namespace gameak::runtime {

class BlockManager {
public:
    BlockManager() = default;

    core::Result<core::Identity> create(
        uint32_t type_id,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity);

    core::Result<void> destroy(core::Identity identity);

    core::Result<core::Identity> create_ephemeral(
        uint32_t type_id,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity);

    void destroy_all_ephemeral(
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    bool has(core::Identity identity) const { return blocks_.contains(identity); }
    uint32_t type_id_for(core::Identity identity) const {
        auto it = identity_types_.find(identity);
        return it != identity_types_.end() ? it->second : 0;
    }
    bool knows_identity(core::Identity identity) const {
        return identity_types_.contains(identity);
    }
    const core::rb_tree<core::Identity, uint32_t>& identity_types() const {
        return identity_types_;
    }
    size_t count(uint32_t type_id) const {
        auto it = type_counts_.find(type_id);
        return it != type_counts_.end() ? it->second : 0;
    }
    size_t total_block_count() const { return blocks_.size(); }

    const DataBlock* get_block(core::Identity identity) const {
        auto it = blocks_.find(identity);
        return it != blocks_.end() ? &it->second : nullptr;
    }

    core::flat_vector<core::Identity, 4> find_by_type(uint32_t type_id) const;
    core::flat_vector<core::Identity, 4> find(
        std::function<bool(const DataBlock&)> pred) const;

    void rebuild_counts();

    // Internal: exposed for save/load and layout conversion
    const core::rb_tree<core::Identity, DataBlock>& ref_blocks() const { return blocks_; }
    core::rb_tree<core::Identity, DataBlock>& mut_blocks() { return blocks_; }
    void set_blocks(core::rb_tree<core::Identity, DataBlock> blocks) {
        blocks_ = std::move(blocks);
        rebuild_counts();
    }

private:
    core::rb_tree<core::Identity, DataBlock> blocks_;
    core::rb_tree<uint32_t, size_t> type_counts_;
    core::rb_tree<core::Identity, uint32_t> identity_types_;
};

// ── Inline implementation ──────────────────────────────────────────

inline core::Result<core::Identity> BlockManager::create(
    uint32_t type_id,
    const core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity)
{
    auto it = types.find(type_id);
    if (it == types.end()) {
        return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered"};
    }
    auto& desc = it->second;
    core::Identity id{++next_identity};
    blocks_.insert(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
    identity_types_.insert(id, type_id);
    auto tc = type_counts_.find(type_id);
    if (tc != type_counts_.end()) tc->second++;
    else type_counts_.insert(type_id, 1);
    return id;
}

inline core::Result<void> BlockManager::destroy(core::Identity identity) {
    if (!identity.is_valid()) {
        return core::Error{core::ErrorCode::InvalidIdentity, "Identity is invalid"};
    }
    auto it = blocks_.find(identity);
    if (it == blocks_.end()) {
        return core::Error{core::ErrorCode::BlockNotFound, "Block not found"};
    }
    uint32_t type_id = it->second.type_id();
    blocks_.erase(identity);
    identity_types_.erase(identity);
    auto tc = type_counts_.find(type_id);
    if (tc != type_counts_.end() && tc->second > 0) {
        tc->second--;
    }
    return {};
}

inline core::Result<core::Identity> BlockManager::create_ephemeral(
    uint32_t type_id,
    const core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity)
{
    auto it = types.find(type_id);
    if (it == types.end()) {
        return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered"};
    }
    if (!it->second.ephemeral) {
        return core::Error{core::ErrorCode::InvalidOperation,
                           "Cannot create ephemeral block for non-ephemeral type"};
    }
    auto& desc = it->second;
    core::Identity id{++next_identity};
    blocks_.insert(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
    identity_types_.insert(id, type_id);
    auto tc = type_counts_.find(type_id);
    if (tc != type_counts_.end()) tc->second++;
    else type_counts_.insert(type_id, 1);
    return id;
}

inline void BlockManager::destroy_all_ephemeral(
    const core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    core::flat_vector<core::Identity, 4> to_destroy;
    for (auto& [id, block] : blocks_) {
        auto tit = types.find(block.type_id());
        if (tit != types.end() && tit->second.ephemeral) {
            to_destroy.push_back(id);
        }
    }
    for (auto& id : to_destroy) {
        auto bit = blocks_.find(id);
        if (bit != blocks_.end()) {
            uint32_t type_id = bit->second.type_id();
            identity_types_.erase(id);
            blocks_.erase(id);
            auto tc = type_counts_.find(type_id);
            if (tc != type_counts_.end() && tc->second > 0) {
                tc->second--;
            }
        }
    }
}

inline core::flat_vector<core::Identity, 4> BlockManager::find_by_type(uint32_t type_id) const {
    return find([type_id](const DataBlock& block) {
        return block.type_id() == type_id;
    });
}

inline core::flat_vector<core::Identity, 4> BlockManager::find(
    std::function<bool(const DataBlock&)> pred) const
{
    core::flat_vector<core::Identity, 4> result;
    for (const auto& [id, block] : blocks_) {
        if (pred(block)) {
            result.push_back(id);
        }
    }
    return result;
}

inline void BlockManager::rebuild_counts() {
    type_counts_.clear();
    for (const auto& [id, block] : blocks_) {
        (void)id;
        auto tc = type_counts_.find(block.type_id());
        if (tc != type_counts_.end()) tc->second++;
        else type_counts_.insert(block.type_id(), 1);
    }
}

} // namespace gameak::runtime
