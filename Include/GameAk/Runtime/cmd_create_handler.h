#pragma once

#include "i_command_handler.h"

namespace gameak::runtime::detail {

class CreateBlockHandler : public ICommandHandler {
    CommandType type() const override { return CommandType::CreateBlock; }

    core::Result<void> validate(
        const CommandPayload& payload,
        const std::unordered_map<core::Identity, DataBlock>&,
        const std::unordered_map<uint32_t, BlockTypeDescriptor>& types) override
    {
        const auto& p = std::get<CommandCreateBlock>(payload);
        if (!types.contains(p.type_id)) {
            return core::Error{core::ErrorCode::TypeNotRegistered};
        }
        return {};
    }

    core::Result<void> execute(
        CommandPayload& payload,
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity) override
    {
        auto& p = std::get<CommandCreateBlock>(payload);
        auto it = types.find(p.type_id);
        if (it == types.end()) {
            return core::Error{core::ErrorCode::TypeNotRegistered};
        }
        auto& desc = it->second;
        core::Identity id{++next_identity};
        blocks.emplace(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
        return {};
    }
};

} // namespace gameak::runtime::detail
