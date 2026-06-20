#pragma once

#include "i_command_handler.h"

#include <cstring>

namespace gameak::runtime::detail {

class SetFieldHandler : public ICommandHandler {
    CommandType type() const override { return CommandType::SetField; }

    core::Result<void> validate(
        const CommandPayload& payload,
        const std::unordered_map<core::Identity, DataBlock>& blocks,
        const std::unordered_map<uint32_t, BlockTypeDescriptor>&) override
    {
        const auto& p = std::get<CommandSetField>(payload);
        for (auto target : p.targets) {
            if (!target.is_valid()) {
                return core::Error{core::ErrorCode::InvalidIdentity, "Target identity is invalid"};
            }
            auto it = blocks.find(target);
            if (it == blocks.end()) {
                return core::Error{core::ErrorCode::BlockNotFound, "Target block not found"};
            }
            if (p.offset + p.data.size() > it->second.size()) {
                return core::Error{core::ErrorCode::CommandInvalid, "Field exceeds block size"};
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
        auto& p = std::get<CommandSetField>(payload);
        for (auto target : p.targets) {
            auto it = blocks.find(target);
            if (it == blocks.end()) continue;
            std::memcpy(static_cast<std::byte*>(it->second.data()) + p.offset,
                        p.data.data(), p.data.size());
        }
        return {};
    }
};

} // namespace gameak::runtime::detail
