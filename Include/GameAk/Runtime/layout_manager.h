#pragma once

#include "block_type.h"
#include "data_block.h"
#include "layout_strategy.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/rb_tree.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <span>

namespace gameak::runtime {

class LayoutManager {
public:
    struct FieldArray {
        core::flat_vector<std::byte> data;
        size_t field_size{0};
        size_t field_offset{0};
        size_t field_alignment{1};
    };

    struct SoAStorage {
        core::flat_vector<core::Identity> identities;
        core::flat_vector<FieldArray, 4> fields;
        core::rb_tree<core::Identity, size_t> identity_index;
    };

    struct AoSoAChunk {
        core::flat_vector<core::Identity> identities;
        core::flat_vector<FieldArray, 4> fields;
        core::rb_tree<core::Identity, size_t> identity_index;
    };

    struct AoSoAStorage {
        uint32_t chunk_size{kDefaultChunkSize};
        core::flat_vector<AoSoAChunk> chunks;
    };

    struct ArchetypeChunk {
        core::flat_vector<core::Identity> identities;
        core::flat_vector<FieldArray, 4> fields;
    };

    struct ArchetypeStorage {
        uint32_t chunk_size{kDefaultArchetypeChunkSize};
        core::flat_vector<ArchetypeChunk> chunks;
        core::rb_tree<core::Identity, std::pair<uint32_t, uint32_t>> sparse_set;
    };

    // ── SoA ────────────────────────────────────────────────────────
    void convert_to_soa(uint32_t type_id,
                        core::rb_tree<core::Identity, DataBlock>& blocks,
                        core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    void convert_from_soa(uint32_t type_id,
                          core::rb_tree<core::Identity, DataBlock>& blocks,
                          core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    // ── AoSoA ──────────────────────────────────────────────────────
    void convert_to_aosoa(uint32_t type_id,
                          core::rb_tree<core::Identity, DataBlock>& blocks,
                          core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    void convert_from_aosoa(uint32_t type_id,
                            core::rb_tree<core::Identity, DataBlock>& blocks,
                            core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    // ── Archetype ──────────────────────────────────────────────────
    void convert_to_archetype(uint32_t type_id,
                              core::rb_tree<core::Identity, DataBlock>& blocks,
                              core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    void convert_from_archetype(uint32_t type_id,
                                core::rb_tree<core::Identity, DataBlock>& blocks,
                                core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    // ── Queries ────────────────────────────────────────────────────
    bool has_storage(uint32_t type_id) const {
        return layout_storage_.contains(type_id) || layout_storage_aosoa_.contains(type_id) ||
               layout_storage_archetype_.contains(type_id);
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

    size_t archetype_block_count(uint32_t type_id) const {
        auto it = layout_storage_archetype_.find(type_id);
        if (it == layout_storage_archetype_.end()) return 0;
        size_t total = 0;
        for (auto& chunk : it->second.chunks) {
            total += chunk.identities.size();
        }
        return total;
    }

    size_t archetype_chunk_count(uint32_t type_id) const {
        auto it = layout_storage_archetype_.find(type_id);
        if (it == layout_storage_archetype_.end()) return 0;
        return it->second.chunks.size();
    }

    std::span<const core::Identity> archetype_chunk_identities(uint32_t type_id, size_t chunk_index) const {
        auto it = layout_storage_archetype_.find(type_id);
        if (it == layout_storage_archetype_.end()) return {};
        if (chunk_index >= it->second.chunks.size()) return {};
        return it->second.chunks[chunk_index].identities;
    }

    std::span<const std::byte> archetype_field_data(uint32_t type_id, size_t chunk_index, size_t field_index) const {
        auto it = layout_storage_archetype_.find(type_id);
        if (it == layout_storage_archetype_.end()) return {};
        if (chunk_index >= it->second.chunks.size()) return {};
        auto& chunk = it->second.chunks[chunk_index];
        if (field_index >= chunk.fields.size()) return {};
        return chunk.fields[field_index].data;
    }

    size_t archetype_field_count(uint32_t type_id) const {
        auto it = layout_storage_archetype_.find(type_id);
        if (it == layout_storage_archetype_.end()) return 0;
        if (it->second.chunks.empty()) return 0;
        return it->second.chunks[0].fields.size();
    }

    /// Read all values of a field across all entities of a type.
    /// Returns a contiguous span of field data from dense storage.
    /// Works with SoA, single-chunk AoSoA, and single-chunk Archetype.
    /// Returns empty span for multi-chunk or AoS layouts (callers should use
    /// find_blocks_by_type or convert to a dense layout first).
    std::span<const std::byte> bulk_field_data(
        uint32_t type_id,
        size_t field_index,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
        const core::rb_tree<core::Identity, DataBlock>& blocks) const
    {
        (void)blocks;
        auto tit = types.find(type_id);
        if (tit == types.end()) return {};
        auto& desc = tit->second;
        if (field_index >= desc.fields.size() && !desc.fields.empty()) return {};

        // SoA — already contiguous
        auto sit = layout_storage_.find(type_id);
        if (sit != layout_storage_.end()) {
            auto& storage = sit->second;
            if (field_index < storage.fields.size()) {
                return storage.fields[field_index].data;
            }
        }

        // AoSoA — single chunk only
        auto ait = layout_storage_aosoa_.find(type_id);
        if (ait != layout_storage_aosoa_.end()) {
            auto& storage = ait->second;
            if (storage.chunks.size() == 1 && field_index < storage.chunks[0].fields.size()) {
                return storage.chunks[0].fields[field_index].data;
            }
        }

        // Archetype — single chunk only
        auto arit = layout_storage_archetype_.find(type_id);
        if (arit != layout_storage_archetype_.end()) {
            auto& storage = arit->second;
            if (storage.chunks.size() == 1 && field_index < storage.chunks[0].fields.size()) {
                return storage.chunks[0].fields[field_index].data;
            }
        }

        return {};
    }

    /// Read a single field value from any layout (AoS, SoA, AoSoA, Archetype).
    /// Returns nullptr if the identity or field is not found.
    /// @param types  The Runtime's block type descriptors (needed for AoS field offsets).
    const std::byte* read_field(
        core::Identity identity,
        uint32_t type_id,
        size_t field_index,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
        const core::rb_tree<core::Identity, DataBlock>& blocks) const
    {
        // Check AoS first (blocks in the main map)
        auto bit = blocks.find(identity);
        if (bit != blocks.end()) {
            auto tit = types.find(type_id);
            if (tit != types.end()) {
                auto& desc = tit->second;
                if (field_index < desc.fields.size()) {
                    size_t offset = desc.fields[field_index].offset;
                    return static_cast<const std::byte*>(bit->second.data()) + offset;
                }
            }
            // No field descriptors — return raw block data
            return static_cast<const std::byte*>(bit->second.data());
        }

        // Check SoA
        auto sit = layout_storage_.find(type_id);
        if (sit != layout_storage_.end()) {
            auto& storage = sit->second;
            auto iit = storage.identity_index.find(identity);
            if (iit != storage.identity_index.end()) {
                if (field_index < storage.fields.size()) {
                    auto& fa = storage.fields[field_index];
                    size_t pos = iit->second * fa.field_size;
                    if (pos + fa.field_size <= fa.data.size()) {
                        return fa.data.data() + pos;
                    }
                }
            }
        }

        // Check AoSoA
        auto ait = layout_storage_aosoa_.find(type_id);
        if (ait != layout_storage_aosoa_.end()) {
            for (auto& chunk : ait->second.chunks) {
                auto cit = chunk.identity_index.find(identity);
                if (cit != chunk.identity_index.end()) {
                    if (field_index < chunk.fields.size()) {
                        auto& fa = chunk.fields[field_index];
                        size_t pos = cit->second * fa.field_size;
                        if (pos + fa.field_size <= fa.data.size()) {
                            return fa.data.data() + pos;
                        }
                    }
                }
            }
        }

        // Check Archetype
        auto arit = layout_storage_archetype_.find(type_id);
        if (arit != layout_storage_archetype_.end()) {
            auto sit = arit->second.sparse_set.find(identity);
            if (sit != arit->second.sparse_set.end()) {
                auto [chunk_idx, slot_idx] = sit->second;
                if (chunk_idx < arit->second.chunks.size()) {
                    auto& chunk = arit->second.chunks[chunk_idx];
                    if (field_index < chunk.fields.size()) {
                        auto& fa = chunk.fields[field_index];
                        size_t pos = slot_idx * fa.field_size;
                        if (pos + fa.field_size <= fa.data.size()) {
                            return fa.data.data() + pos;
                        }
                    }
                }
            }
        }

        return nullptr;
    }

private:
    void ensure_soa_storage(uint32_t type_id,
                            const core::rb_tree<uint32_t, BlockTypeDescriptor>& types);
    void ensure_aosoa_storage(uint32_t type_id,
                              const core::rb_tree<uint32_t, BlockTypeDescriptor>& types);
    void ensure_archetype_storage(uint32_t type_id,
                                  const core::rb_tree<uint32_t, BlockTypeDescriptor>& types);

    core::rb_tree<uint32_t, SoAStorage> layout_storage_;
    core::rb_tree<uint32_t, AoSoAStorage> layout_storage_aosoa_;
    core::rb_tree<uint32_t, ArchetypeStorage> layout_storage_archetype_;
};

// ── Implementation ──────────────────────────────────────────────────

inline void LayoutManager::ensure_soa_storage(
    uint32_t type_id,
    const core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
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

    layout_storage_.insert(type_id, std::move(storage));
}

inline void LayoutManager::convert_to_soa(
    uint32_t type_id,
    core::rb_tree<core::Identity, DataBlock>& blocks,
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    auto tit = types.find(type_id);
    if (tit == types.end()) return;
    auto& desc = tit->second;
    if (desc.layout == LayoutStrategy::SoA) return;

    ensure_soa_storage(type_id, types);
    auto sit = layout_storage_.find(type_id);
    if (sit == layout_storage_.end()) return;
    auto& storage = sit->second;

    core::flat_vector<std::pair<core::Identity, DataBlock>> existing;
    for (auto& [id, block] : blocks) {
        if (block.type_id() == type_id) {
            existing.emplace_back(id, std::move(block));
        }
    }

    for (auto& [id, _] : existing) {
        (void)_;
        blocks.erase(id);
    }

    for (size_t ei = 0; ei < existing.size(); ++ei) {
        auto& [id, block] = existing[ei];
        storage.identities.push_back(id);
        storage.identity_index.insert(id, ei);
        for (size_t fi = 0; fi < storage.fields.size(); ++fi) {
            auto& fa = storage.fields[fi];
            size_t old_pos  = fa.field_offset;
            size_t copy_size = std::min(fa.field_size, block.size() - old_pos);
            size_t new_pos  = ei * fa.field_size;
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
    core::rb_tree<core::Identity, DataBlock>& blocks,
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    auto tit = types.find(type_id);
    if (tit == types.end()) return;
    auto& desc = tit->second;
    if (desc.layout != LayoutStrategy::SoA) return;

    auto it = layout_storage_.find(type_id);
    if (it == layout_storage_.end()) return;

    auto& storage = it->second;

    for (size_t i = 0; i < storage.identities.size(); ++i) {
        core::Identity id = storage.identities[i];
        DataBlock block{id, type_id, desc.size, desc.alignment};
        for (size_t fi = 0; fi < storage.fields.size(); ++fi) {
            auto& fa = storage.fields[fi];
            size_t src_pos  = i * fa.field_size;
            size_t copy_size = std::min(fa.field_size, desc.size - fa.field_offset);
            if (src_pos + copy_size <= fa.data.size()) {
                std::memcpy(static_cast<std::byte*>(block.data()) + fa.field_offset,
                            fa.data.data() + src_pos, copy_size);
            }
        }
        blocks.insert(id, std::move(block));
    }

    layout_storage_.erase(type_id);
    desc.layout = LayoutStrategy::AoS;
}

inline void LayoutManager::ensure_aosoa_storage(
    uint32_t type_id,
    const core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    if (layout_storage_aosoa_.contains(type_id)) return;

    auto it = types.find(type_id);
    if (it == types.end()) return;

    auto& desc = it->second;
    AoSoAStorage storage;
    storage.chunk_size = desc.aosoa_config.chunk_size;
    if (storage.chunk_size == 0) storage.chunk_size = kDefaultChunkSize;

    layout_storage_aosoa_.insert(type_id, std::move(storage));
}

inline void LayoutManager::convert_to_aosoa(
    uint32_t type_id,
    core::rb_tree<core::Identity, DataBlock>& blocks,
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    auto tit = types.find(type_id);
    if (tit == types.end()) return;
    auto& desc = tit->second;
    if (desc.layout == LayoutStrategy::AoSoA) return;

    ensure_aosoa_storage(type_id, types);
    auto ait = layout_storage_aosoa_.find(type_id);
    if (ait == layout_storage_aosoa_.end()) return;
    auto& storage = ait->second;

    core::flat_vector<std::pair<core::Identity, DataBlock>> existing;
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
            chunk.identity_index.insert(id, local_idx);
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
    core::rb_tree<core::Identity, DataBlock>& blocks,
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    auto tit = types.find(type_id);
    if (tit == types.end()) return;
    auto& desc = tit->second;
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
            blocks.insert(id, std::move(block));
        }
    }

    layout_storage_aosoa_.erase(type_id);
    desc.layout = LayoutStrategy::AoS;
}

inline void LayoutManager::ensure_archetype_storage(
    uint32_t type_id,
    const core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    if (layout_storage_archetype_.contains(type_id)) return;

    auto it = types.find(type_id);
    if (it == types.end()) return;

    auto& desc = it->second;
    ArchetypeStorage storage;
    storage.chunk_size = desc.archetype_config.chunk_size;
    if (storage.chunk_size == 0) storage.chunk_size = kDefaultArchetypeChunkSize;

    layout_storage_archetype_.insert(type_id, std::move(storage));
}

inline void LayoutManager::convert_to_archetype(
    uint32_t type_id,
    core::rb_tree<core::Identity, DataBlock>& blocks,
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    auto tit = types.find(type_id);
    if (tit == types.end()) return;
    auto& desc = tit->second;
    if (desc.layout == LayoutStrategy::Archetype) return;

    ensure_archetype_storage(type_id, types);
    auto ait = layout_storage_archetype_.find(type_id);
    if (ait == layout_storage_archetype_.end()) return;
    auto& storage = ait->second;

    core::flat_vector<std::pair<core::Identity, DataBlock>> existing;
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
        desc.layout = LayoutStrategy::Archetype;
        return;
    }

    const uint32_t chunk_size = storage.chunk_size;

    for (size_t base = 0; base < existing.size(); base += chunk_size) {
        ArchetypeChunk chunk;
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
            storage.sparse_set.insert(id, {static_cast<uint32_t>(storage.chunks.size()), static_cast<uint32_t>(local_idx)});
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

    desc.layout = LayoutStrategy::Archetype;
}

inline void LayoutManager::convert_from_archetype(
    uint32_t type_id,
    core::rb_tree<core::Identity, DataBlock>& blocks,
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types)
{
    auto tit = types.find(type_id);
    if (tit == types.end()) return;
    auto& desc = tit->second;
    if (desc.layout != LayoutStrategy::Archetype) return;

    auto it = layout_storage_archetype_.find(type_id);
    if (it == layout_storage_archetype_.end()) return;

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
            blocks.insert(id, std::move(block));
        }
    }

    layout_storage_archetype_.erase(type_id);
    desc.layout = LayoutStrategy::AoS;
}

} // namespace gameak::runtime
