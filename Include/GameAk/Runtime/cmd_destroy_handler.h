#pragma once

#include "i_command_handler.h"

namespace gameak::runtime::detail {

class DestroyBlockHandler : public ICommandHandler {
    CommandType type() const override { return CommandType::DestroyBlock; }

    core::Result<void> validate(
        const CommandPayload& payload,
        const core::rb_tree<core::Identity, DataBlock>& blocks,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>&) override
    {
        const auto& p = std::get<CommandDestroyBlock>(payload);
        for (auto target : p.targets) {
            if (!target.is_valid()) {
                return core::Error{core::ErrorCode::InvalidIdentity, "Target identity is invalid"};
            }
            if (!blocks.contains(target)) {
                return core::Error{core::ErrorCode::BlockNotFound, "Target block not found"};
            }
        }
        return {};
    }

    core::Result<void> execute(
        CommandPayload& payload,
        CommandContext& ctx) override
    {
        auto& p = std::get<CommandDestroyBlock>(payload);
        for (auto target : p.targets) {
            auto bit = ctx.blocks.find(target);
            if (bit == ctx.blocks.end()) continue;
            uint32_t tid = bit->second.type_id();
            ctx.identity_types.erase(target);
            ctx.blocks.erase(target);
            auto tc = ctx.type_counts.find(tid);
            if (tc != ctx.type_counts.end() && tc->second > 0) tc->second--;
        }
        return {};
    }
};

} // namespace gameak::runtime::detail
