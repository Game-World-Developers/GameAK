#pragma once

#include "block_type.h"
#include "data_block.h"
#include "layout_manager.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"

#include <cstddef>
#include <cstdint>
#include <unordered_map>

namespace gameak::runtime {

class StateView {
public:
    // Original constructor (AoS-only field access via get_block)
    explicit StateView(const std::unordered_map<core::Identity, DataBlock>& blocks,
                       const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
                       float time_delta = 0.016f)
        : blocks_{blocks}, types_{types}, time_delta_{time_delta} {}

    // Layout-aware constructor (enables field<> across AoS/SoA/AoSoA)
    explicit StateView(const std::unordered_map<core::Identity, DataBlock>& blocks,
                       const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
                       const LayoutManager& layout_mgr,
                       const std::unordered_map<core::Identity, uint32_t>& identity_types,
                       float time_delta = 0.016f)
        : blocks_{blocks}, types_{types}, layout_mgr_{&layout_mgr},
          identity_types_{&identity_types}, time_delta_{time_delta} {}

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

    // ── Type-safe field access by name ──────────────────────────────
    // Works across all layouts (AoS, SoA, AoSoA).
    // Returns nullptr if identity, type, or field name is not found.
    template <typename T>
    const T* field(core::Identity identity, const char* field_name) const {
        if (!identity_types_ || !layout_mgr_) return nullptr;
        auto type_it = identity_types_->find(identity);
        if (type_it == identity_types_->end()) return nullptr;
        uint32_t type_id = type_it->second;

        auto desc_it = types_.find(type_id);
        if (desc_it == types_.end()) return nullptr;
        auto& desc = desc_it->second;

        auto field_it = desc.field_index.find(field_name);
        if (field_it == desc.field_index.end()) return nullptr;
        size_t field_idx = field_it->second;

        auto bytes = layout_mgr_->read_field(
            identity, type_id, field_idx, types_, blocks_);
        if (!bytes) return nullptr;
        return reinterpret_cast<const T*>(bytes);
    }

    // ── Type-safe field access by member pointer ────────────────────
    // Usage: view.field(player_id, &Player::hp)
    template <typename T, typename U>
    const U& field(core::Identity identity, U T::*member) const {
        size_t offset = reinterpret_cast<size_t>(
            &(static_cast<T*>(nullptr)->*member));

        if (!identity_types_ || !layout_mgr_) {
            // Fallback to AoS direct access
            auto* block = get_block(identity);
            if (!block) {
                static U s_sentinel{};
                return s_sentinel;
            }
            return block->field<U>(offset);
        }

        auto type_it = identity_types_->find(identity);
        if (type_it == identity_types_->end()) {
            static U s_sentinel{};
            return s_sentinel;
        }
        uint32_t type_id = type_it->second;

        auto desc_it = types_.find(type_id);
        if (desc_it == types_.end()) {
            static U s_sentinel{};
            return s_sentinel;
        }
        auto& desc = desc_it->second;

        // Find field by offset
        auto off_it = desc.offset_index.find(offset);
        if (off_it == desc.offset_index.end()) {
            static U s_sentinel{};
            return s_sentinel;
        }
        size_t field_idx = off_it->second;

        auto bytes = layout_mgr_->read_field(
            identity, type_id, field_idx, types_, blocks_);
        if (!bytes) {
            static U s_sentinel{};
            return s_sentinel;
        }
        return *reinterpret_cast<const U*>(bytes);
    }

private:
    const std::unordered_map<core::Identity, DataBlock>& blocks_;
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types_;
    const LayoutManager* layout_mgr_{nullptr};
    const std::unordered_map<core::Identity, uint32_t>* identity_types_{nullptr};
    float time_delta_{0.016f};
};

} // namespace gameak::runtime
