#pragma once

#include "i_command_handler.h"

namespace gameak::runtime::detail {

class CreateBlockHandler : public ICommandHandler {
    CommandType type() const override { return CommandType::CreateBlock; }

    core::Result<void> validate(
        const CommandPayload& payload,
        const core::rb_tree<core::Identity, DataBlock>&,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types) override
    {
        const auto& p = std::get<CommandCreateBlock>(payload);
        if (!types.contains(p.type_id)) {
            return core::Error{core::ErrorCode::TypeNotRegistered};
        }
        return {};
    }

    core::Result<void> execute(
        CommandPayload& payload,
        CommandContext& ctx) override
    {
        auto& p = std::get<CommandCreateBlock>(payload);
        auto it = ctx.types.find(p.type_id);
        if (it == ctx.types.end()) {
            return core::Error{core::ErrorCode::TypeNotRegistered};
        }
        auto& desc = it->second;
        core::Identity id{++ctx.next_identity};
        ctx.blocks.insert(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
        ctx.identity_types.insert(id, p.type_id);
        auto tc = ctx.type_counts.find(p.type_id);
        if (tc != ctx.type_counts.end()) tc->second++;
        else ctx.type_counts.insert(p.type_id, 1);
        return {};
    }
};

} // namespace gameak::runtime::detail
