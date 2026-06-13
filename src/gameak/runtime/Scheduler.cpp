#include "FifoScheduler.h"
#include <cstring>
#include <utility>

namespace gameak::runtime {

void FifoScheduler::enqueue(Command command) {
    queue_.push(std::move(command));
}

core::Result<void> FifoScheduler::validate(
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
            if (!payload.target.is_valid()) {
                return core::Error{core::ErrorCode::InvalidIdentity, "Target identity is invalid"};
            }
            if (!blocks.contains(payload.target)) {
                return core::Error{core::ErrorCode::BlockNotFound, "Target block not found"};
            }
            return {};
        }
        case CommandType::SetField: {
            const auto& payload = std::get<CommandSetField>(command.payload());
            if (!payload.target.is_valid()) {
                return core::Error{core::ErrorCode::InvalidIdentity, "Target identity is invalid"};
            }
            auto it = blocks.find(payload.target);
            if (it == blocks.end()) {
                return core::Error{core::ErrorCode::BlockNotFound, "Target block not found"};
            }
            if (payload.offset + payload.data.size() > it->second.size()) {
                return core::Error{core::ErrorCode::CommandInvalid, "Field exceeds block size"};
            }
            return {};
        }
    }
    return core::Error{core::ErrorCode::InternalError, "Unknown command type"};
}

core::Result<void> FifoScheduler::execute(
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
            blocks.erase(payload.target);
            return {};
        }
        case CommandType::SetField: {
            auto& payload = std::get<CommandSetField>(command.payload());
            auto it = blocks.find(payload.target);
            if (it == blocks.end()) {
                return core::Error{core::ErrorCode::BlockNotFound};
            }
            std::memcpy(static_cast<std::byte*>(it->second.data()) + payload.offset,
                        payload.data.data(), payload.data.size());
            return {};
        }
    }
    return core::Error{core::ErrorCode::InternalError};
}

core::Result<void> FifoScheduler::process_pending(
    std::unordered_map<core::Identity, DataBlock>& blocks,
    std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity) {

    while (!queue_.empty()) {
        auto command = std::move(queue_.front());
        queue_.pop();

        auto validation = validate(command, blocks, types);
        if (!validation) {
            rejected_++;
            continue;
        }

        auto execution = execute(command, blocks, types, next_identity);
        if (!execution) {
            rejected_++;
        } else {
            executed_++;
        }
    }
    return {};
}

} // namespace gameak::runtime
