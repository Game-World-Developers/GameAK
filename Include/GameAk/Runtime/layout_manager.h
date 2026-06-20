#pragma once

#include "block_type.h"
#include "data_block.h"
#include "layout_strategy.h"
#include "GameAk/Core/identity.h"

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <span>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

class LayoutManager {
public:
    struct FieldArray {
        std::vector<std::byte> data;
        size_t field_size{0};
        size_t field_offset{0};
        size_t field_alignment{1};
    };

    struct SoAStorage {
        std::vector<core::Identity> identities;
        std::vector<FieldArray> fields;
    };

    struct AoSoAChunk {
        std::vector<core::Identity> identities;
        std::vector<FieldArray> fields;
    };

    struct AoSoAStorage {
        uint32_t chunk_size{kDefaultChunkSize};
        std::vector<AoSoAChunk> chunks;
    };

    // ── SoA ────────────────────────────────────────────────────────
    void convert_to_soa(uint32_t type_id,
                        std::unordered_map<core::Identity, DataBlock>& blocks,
                        std::unordered_map<uint32_t, BlockTypeDescriptor>& types);

    void convert_from_soa(uint32_t type_id,
                          std::unordered_map<core::Identity, DataBlock>& blocks,
                          std::unordered_map<uint32_t, BlockTypeDescriptor>& types);

    // ── AoSoA ──────────────────────────────────────────────────────
    void convert_to_aosoa(uint32_t type_id,
                          std::unordered_map<core::Identity, DataBlock>& blocks,
                          std::unordered_map<uint32_t, BlockTypeDescriptor>& types);

    void convert_from_aosoa(uint32_t type_id,
                            std::unordered_map<core::Identity, DataBlock>& blocks,
                            std::unordered_map<uint32_t, BlockTypeDescriptor>& types);

    // ── Queries ────────────────────────────────────────────────────
    bool has_storage(uint32_t type_id) const {
        return layout_storage_.contains(type_id) || layout_storage_aosoa_.contains(type_id);
    }

    size_t soa_block_count(uint32_t type_id) const {
        auto it = layout_storage_.find(type_id);
        if (it == layout_storage_.end()) return 0;
        return it->second.identities.size();
    }

    std::span<const std::byte> soa_field_data(uint32_t type_id, size_t field_index) const {
        auto it = layout_storage_.find(type_id);
        if (it == layout_storage_.end()) return {};
        if (field_index >= it->second.fields.size()) return {};
        return it->second.fields[field_index].data;
    }

    size_t soa_field_count(uint32_t type_id) const {
        auto it = layout_storage_.find(type_id);
        if (it == layout_storage_.end()) return 0;
        return it->second.fields.size();
    }

    size_t aosoa_block_count(uint32_t type_id) const {
        auto it = layout_storage_aosoa_.find(type_id);
        if (it == layout_storage_aosoa_.end()) return 0;
        size_t total = 0;
        for (auto& chunk : it->second.chunks) {
            total += chunk.identities.size();
        }
        return total;
    }

    size_t aosoa_chunk_count(uint32_t type_id) const {
        auto it = layout_storage_aosoa_.find(type_id);
        if (it == layout_storage_aosoa_.end()) return 0;
        return it->second.chunks.size();
    }

    std::span<const core::Identity> aosoa_chunk_identities(uint32_t type_id, size_t chunk_index) const {
        auto it = layout_storage_aosoa_.find(type_id);
        if (it == layout_storage_aosoa_.end()) return {};
        if (chunk_index >= it->second.chunks.size()) return {};
        return it->second.chunks[chunk_index].identities;
    }

    std::span<const std::byte> aosoa_field_data(uint32_t type_id, size_t chunk_index, size_t field_index) const {
        auto it = layout_storage_aosoa_.find(type_id);
        if (it == layout_storage_aosoa_.end()) return {};
        if (chunk_index >= it->second.chunks.size()) return {};
        auto& chunk = it->second.chunks[chunk_index];
        if (field_index >= chunk.fields.size()) return {};
        return chunk.fields[field_index].data;
    }

    size_t aosoa_field_count(uint32_t type_id) const {
        auto it = layout_storage_aosoa_.find(type_id);
        if (it == layout_storage_aosoa_.end()) return 0;
        if (it->second.chunks.empty()) return 0;
        return it->second.chunks[0].fields.size();
    }

private:
    void ensure_soa_storage(uint32_t type_id,
                            const std::unordered_map<uint32_t, BlockTypeDescriptor>& types);
    void ensure_aosoa_storage(uint32_t type_id,
                              const std::unordered_map<uint32_t, BlockTypeDescriptor>& types);

    std::unordered_map<uint32_t, SoAStorage> layout_storage_;
    std::unordered_map<uint32_t, AoSoAStorage> layout_storage_aosoa_;
};

// ── Implementation ──────────────────────────────────────────────────

inline void LayoutManager::ensure_soa_storage(
    uint32_t type_id,
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types)
{
    if (layout_storage_.contains(type_id)) return;

    auto it = types.find(type_id);
    if (it == types.end()) return;

    auto& desc = it->second;
    SoAStorage storage;

    for (auto& field : desc.fields) {
        FieldArray fa;
        fa.field_size      = field.size;
        fa.field_offset    = field.offset;
        fa.field_alignment = field.alignment;
        storage.fields.push_back(std::move(fa));
    }

    if (desc.fields.empty()) {
        FieldArray fa;
        fa.field_size      = desc.size;
        fa.field_offset    = 0;
        fa.field_alignment = desc.alignment;
        storage.fields.push_back(std::move(fa));
    }

    layout_storage_[type_id] = std::move(storage);
}

inline void LayoutManager::convert_to_soa(
    uint32_t type_id,
    std::unordered_map<core::Identity, DataBlock>& blocks,
    std::unordered_map<uint32_t, BlockTypeDescriptor>& types)
{
    auto& desc = types[type_id];
    if (desc.layout == LayoutStrategy::SoA) return;

    ensure_soa_storage(type_id, types);
    auto& storage = layout_storage_[type_id];

    std::vector<std::pair<core::Identity, DataBlock>> existing;
    for (auto& [id, block] : blocks) {
        if (block.type_id() == type_id) {
            existing.emplace_back(id, std::move(block));
        }
    }

    for (auto& [id, _] : existing) {
        (void)_;
        blocks.erase(id);
    }

    for (auto& [id, block] : existing) {
        storage.identities.push_back(id);
        for (size_t fi = 0; fi < storage.fields.size(); ++fi) {
            auto& fa = storage.fields[fi];
            size_t old_pos  = fa.field_offset;
            size_t copy_size = std::min(fa.field_size, block.size() - old_pos);
            size_t new_pos  = (storage.identities.size() - 1) * fa.field_size + fa.field_offset;
            if (fa.data.size() < new_pos + copy_size) {
                fa.data.resize(new_pos + copy_size);
            }
            std::memcpy(fa.data.data() + new_pos,
                        static_cast<const std::byte*>(block.data()) + old_pos,
                        copy_size);
        }
    }

    desc.layout = LayoutStrategy::SoA;
}

inline void LayoutManager::convert_from_soa(
    uint32_t type_id,
    std::unordered_map<core::Identity, DataBlock>& blocks,
    std::unordered_map<uint32_t, BlockTypeDescriptor>& types)
{
    auto& desc = types[type_id];
    if (desc.layout != LayoutStrategy::SoA) return;

    auto it = layout_storage_.find(type_id);
    if (it == layout_storage_.end()) return;

    auto& storage = it->second;

    for (size_t i = 0; i < storage.identities.size(); ++i) {
        core::Identity id = storage.identities[i];
        DataBlock block{id, type_id, desc.size, desc.alignment};
        for (size_t fi = 0; fi < storage.fields.size(); ++fi) {
            auto& fa = storage.fields[fi];
            size_t src_pos  = i * fa.field_size + fa.field_offset;
            size_t copy_size = std::min(fa.field_size, desc.size - fa.field_offset);
            if (src_pos + copy_size <= fa.data.size()) {
                std::memcpy(static_cast<std::byte*>(block.data()) + fa.field_offset,
                            fa.data.data() + src_pos, copy_size);
            }
        }
        blocks.emplace(id, std::move(block));
    }

    layout_storage_.erase(type_id);
    desc.layout = LayoutStrategy::AoS;
}

inline void LayoutManager::ensure_aosoa_storage(
    uint32_t type_id,
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types)
{
    if (layout_storage_aosoa_.contains(type_id)) return;

    auto it = types.find(type_id);
    if (it == types.end()) return;

    auto& desc = it->second;
    AoSoAStorage storage;
    storage.chunk_size = desc.aosoa_config.chunk_size;
    if (storage.chunk_size == 0) storage.chunk_size = kDefaultChunkSize;

    layout_storage_aosoa_[type_id] = std::move(storage);
}

inline void LayoutManager::convert_to_aosoa(
    uint32_t type_id,
    std::unordered_map<core::Identity, DataBlock>& blocks,
    std::unordered_map<uint32_t, BlockTypeDescriptor>& types)
{
    auto& desc = types[type_id];
    if (desc.layout == LayoutStrategy::AoSoA) return;

    ensure_aosoa_storage(type_id, types);
    auto& storage = layout_storage_aosoa_[type_id];

    std::vector<std::pair<core::Identity, DataBlock>> existing;
    for (auto& [id, block] : blocks) {
        if (block.type_id() == type_id) {
            existing.emplace_back(id, std::move(block));
        }
    }

    for (auto& [id, _] : existing) {
        (void)_;
        blocks.erase(id);
    }

    if (existing.empty()) {
        desc.layout = LayoutStrategy::AoSoA;
        return;
    }

    const uint32_t chunk_size = storage.chunk_size;

    for (size_t base = 0; base < existing.size(); base += chunk_size) {
        AoSoAChunk chunk;
        size_t end   = std::min(base + chunk_size, existing.size());
        size_t count = end - base;

        for (auto& field : desc.fields) {
            FieldArray fa;
            fa.field_size      = field.size;
            fa.field_offset    = field.offset;
            fa.field_alignment = field.alignment;
            fa.data.resize(count * field.size);
            chunk.fields.push_back(std::move(fa));
        }
        if (desc.fields.empty()) {
            FieldArray fa;
            fa.field_size      = desc.size;
            fa.field_offset    = 0;
            fa.field_alignment = desc.alignment;
            fa.data.resize(count * desc.size);
            chunk.fields.push_back(std::move(fa));
        }

        for (size_t i = base; i < end; ++i) {
            auto& [id, block] = existing[i];
            size_t local_idx = i - base;
            chunk.identities.push_back(id);
            for (size_t fi = 0; fi < chunk.fields.size(); ++fi) {
                auto& fa = chunk.fields[fi];
                size_t copy_size = std::min(fa.field_size, block.size() - fa.field_offset);
                size_t dst_pos   = local_idx * fa.field_size;
                std::memcpy(fa.data.data() + dst_pos,
                            static_cast<const std::byte*>(block.data()) + fa.field_offset,
                            copy_size);
            }
        }

        storage.chunks.push_back(std::move(chunk));
    }

    desc.layout = LayoutStrategy::AoSoA;
}

inline void LayoutManager::convert_from_aosoa(
    uint32_t type_id,
    std::unordered_map<core::Identity, DataBlock>& blocks,
    std::unordered_map<uint32_t, BlockTypeDescriptor>& types)
{
    auto& desc = types[type_id];
    if (desc.layout != LayoutStrategy::AoSoA) return;

    auto it = layout_storage_aosoa_.find(type_id);
    if (it == layout_storage_aosoa_.end()) return;

    auto& storage = it->second;

    for (auto& chunk : storage.chunks) {
        for (size_t i = 0; i < chunk.identities.size(); ++i) {
            core::Identity id = chunk.identities[i];
            DataBlock block{id, type_id, desc.size, desc.alignment};
            for (size_t fi = 0; fi < chunk.fields.size(); ++fi) {
                auto& fa = chunk.fields[fi];
                size_t src_pos   = i * fa.field_size;
                size_t copy_size = std::min(fa.field_size, desc.size - fa.field_offset);
                if (src_pos + copy_size <= fa.data.size()) {
                    std::memcpy(static_cast<std::byte*>(block.data()) + fa.field_offset,
                                fa.data.data() + src_pos, copy_size);
                }
            }
            blocks.emplace(id, std::move(block));
        }
    }

    layout_storage_aosoa_.erase(type_id);
    desc.layout = LayoutStrategy::AoS;
}

} // namespace gameak::runtime
