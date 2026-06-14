#pragma once

#include "command.h"
#include "data_block.h"

#include <cstring>

namespace gameak::runtime::detail {

inline core::Result<void> validate_command(
    const Command& command,
    const std::unordered_map<core::Identity, DataBlock>& blocks,
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types) {

    switch (command.type()) {
        case CommandType::CreateBlock: {
            const auto& payload = std::get<CommandCreateBlock>(command.payload());
            if (!types.contains(payload.type_id)) {
                return core::Error{core::ErrorCode::TypeNotRegistered, "Block type not registered"};
            }
            return {};
        }
        case CommandType::DestroyBlock: {
            const auto& payload = std::get<CommandDestroyBlock>(command.payload());
            for (auto target : payload.targets) {
                if (!target.is_valid()) {
                    return core::Error{core::ErrorCode::InvalidIdentity, "Target identity is invalid"};
                }
                if (!blocks.contains(target)) {
                    return core::Error{core::ErrorCode::BlockNotFound, "Target block not found"};
                }
            }
            return {};
        }
        case CommandType::ResizeBlock: {
            const auto& payload = std::get<CommandResizeBlock>(command.payload());
            for (auto target : payload.targets) {
                if (!target.is_valid()) {
                    return core::Error{core::ErrorCode::InvalidIdentity, "Target identity is invalid"};
                }
                if (!blocks.contains(target)) {
                    return core::Error{core::ErrorCode::BlockNotFound, "Target block not found"};
                }
            }
            return {};
        }
        case CommandType::SetField: {
            const auto& payload = std::get<CommandSetField>(command.payload());
            for (auto target : payload.targets) {
                if (!target.is_valid()) {
                    return core::Error{core::ErrorCode::InvalidIdentity, "Target identity is invalid"};
                }
                auto it = blocks.find(target);
                if (it == blocks.end()) {
                    return core::Error{core::ErrorCode::BlockNotFound, "Target block not found"};
                }
                if (payload.offset + payload.data.size() > it->second.size()) {
                    return core::Error{core::ErrorCode::CommandInvalid, "Field exceeds block size"};
                }
            }
            return {};
        }
    }
    return core::Error{core::ErrorCode::InternalError, "Unknown command type"};
}

inline core::Result<void> execute_command(
    Command& command,
    std::unordered_map<core::Identity, DataBlock>& blocks,
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity) {

    switch (command.type()) {
        case CommandType::CreateBlock: {
            const auto& payload = std::get<CommandCreateBlock>(command.payload());
            auto it = types.find(payload.type_id);
            if (it == types.end()) {
                return core::Error{core::ErrorCode::TypeNotRegistered};
            }
            auto& desc = it->second;
            core::Identity id{++next_identity};
            blocks.emplace(id, DataBlock{id, desc.type_id, desc.size, desc.alignment});
            return {};
        }
        case CommandType::DestroyBlock: {
            const auto& payload = std::get<CommandDestroyBlock>(command.payload());
            for (auto target : payload.targets) {
                blocks.erase(target);
            }
            return {};
        }
        case CommandType::ResizeBlock: {
            const auto& payload = std::get<CommandResizeBlock>(command.payload());
            for (auto target : payload.targets) {
                auto it = blocks.find(target);
                if (it == blocks.end()) {
                    return core::Error{core::ErrorCode::BlockNotFound};
                }
                it->second.resize(payload.new_size);
            }
            return {};
        }
        case CommandType::SetField: {
            auto& payload = std::get<CommandSetField>(command.payload());
            for (auto target : payload.targets) {
                auto it = blocks.find(target);
                if (it == blocks.end()) {
                    return core::Error{core::ErrorCode::BlockNotFound};
                }
                std::memcpy(static_cast<std::byte*>(it->second.data()) + payload.offset,
                            payload.data.data(), payload.data.size());
            }
            return {};
        }
    }
    return core::Error{core::ErrorCode::InternalError};
}

} // namespace gameak::runtime::detail
