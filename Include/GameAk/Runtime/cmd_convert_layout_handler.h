#pragma once

#include "i_command_handler.h"

namespace gameak::runtime::detail {

class ConvertLayoutHandler : public ICommandHandler {
    CommandType type() const override { return CommandType::ConvertLayout; }

    core::Result<void> validate(
        const CommandPayload& payload,
        const std::unordered_map<core::Identity, DataBlock>&,
        const std::unordered_map<uint32_t, BlockTypeDescriptor>& types) override
    {
        const auto& p = std::get<CommandConvertLayout>(payload);
        if (!types.contains(p.type_id)) {
            return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered for layout conversion"};
        }
        return {};
    }

    core::Result<void> execute(
        CommandPayload& payload,
        std::unordered_map<core::Identity, DataBlock>&,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t&) override
    {
        auto& p = std::get<CommandConvertLayout>(payload);
        auto it = types.find(p.type_id);
        if (it == types.end()) {
            return core::Error{core::ErrorCode::TypeNotRegistered};
        }
        auto& desc = it->second;

        if (desc.layout == p.new_layout) return {};

        if (p.new_layout == LayoutStrategy::AoS) {
            if (desc.layout == LayoutStrategy::SoA) {
                desc.layout = LayoutStrategy::AoS;
            } else if (desc.layout == LayoutStrategy::AoSoA) {
                desc.layout = LayoutStrategy::AoS;
            }
            return {};
        }

        desc.layout = p.new_layout;
        return {};
    }
};

} // namespace gameak::runtime::detail
