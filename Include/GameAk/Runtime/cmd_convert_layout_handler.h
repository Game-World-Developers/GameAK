#pragma once

#include "i_command_handler.h"
#include "layout_manager.h"

namespace gameak::runtime::detail {

class ConvertLayoutHandler : public ICommandHandler {
    CommandType type() const override { return CommandType::ConvertLayout; }

    core::Result<void> validate(
        const CommandPayload& payload,
        const core::rb_tree<core::Identity, DataBlock>&,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types) override
    {
        const auto& p = std::get<CommandConvertLayout>(payload);
        if (!types.contains(p.type_id)) {
            return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered for layout conversion"};
        }
        return {};
    }

    core::Result<void> execute(
        CommandPayload& payload,
        CommandContext& ctx) override
    {
        auto& p = std::get<CommandConvertLayout>(payload);
        auto it = ctx.types.find(p.type_id);
        if (it == ctx.types.end()) {
            return core::Error{core::ErrorCode::TypeNotRegistered};
        }
        auto& desc = it->second;

        if (desc.layout == p.new_layout) return {};

        if (!ctx.layout_mgr) {
            return core::Error{core::ErrorCode::InternalError, "LayoutManager not available"};
        }

        if (p.new_layout == LayoutStrategy::SoA) {
            ctx.layout_mgr->convert_to_soa(p.type_id, ctx.blocks, ctx.types);
        } else if (p.new_layout == LayoutStrategy::AoSoA) {
            auto& config = p.aosoa_config;
            if (config.chunk_size > 0) {
                desc.aosoa_config = config;
            }
            ctx.layout_mgr->convert_to_aosoa(p.type_id, ctx.blocks, ctx.types);
        } else if (p.new_layout == LayoutStrategy::Archetype) {
            auto& config = p.archetype_config;
            if (config.chunk_size > 0) {
                desc.archetype_config = config;
            }
            ctx.layout_mgr->convert_to_archetype(p.type_id, ctx.blocks, ctx.types);
        } else if (p.new_layout == LayoutStrategy::AoS) {
            if (desc.layout == LayoutStrategy::SoA) {
                ctx.layout_mgr->convert_from_soa(p.type_id, ctx.blocks, ctx.types);
            } else if (desc.layout == LayoutStrategy::AoSoA) {
                ctx.layout_mgr->convert_from_aosoa(p.type_id, ctx.blocks, ctx.types);
            } else if (desc.layout == LayoutStrategy::Archetype) {
                ctx.layout_mgr->convert_from_archetype(p.type_id, ctx.blocks, ctx.types);
            }
        }

        return {};
    }
};

} // namespace gameak::runtime::detail
