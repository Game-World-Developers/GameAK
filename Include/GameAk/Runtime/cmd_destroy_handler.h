#pragma once

#include "i_command_handler.h"

namespace gameak::runtime::detail {

class DestroyBlockHandler : public ICommandHandler {
    CommandType type() const override { return CommandType::DestroyBlock; }

    core::Result<void> validate(
        const CommandPayload& payload,
        const std::unordered_map<core::Identity, DataBlock>& blocks,
        const std::unordered_map<uint32_t, BlockTypeDescriptor>&) override
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
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>&,
        uint64_t&) override
    {
        auto& p = std::get<CommandDestroyBlock>(payload);
        for (auto target : p.targets) {
            blocks.erase(target);
        }
        return {};
    }
};

} // namespace gameak::runtime::detail
