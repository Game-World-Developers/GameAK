#pragma once

#include "command.h"
#include "block_type.h"
#include "data_block.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <unordered_map>

namespace gameak::runtime::detail {

class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;
    virtual CommandType type() const = 0;
    virtual core::Result<void> validate(
        const CommandPayload& payload,
        const std::unordered_map<core::Identity, DataBlock>& blocks,
        const std::unordered_map<uint32_t, BlockTypeDescriptor>& types) = 0;
    virtual core::Result<void> execute(
        CommandPayload& payload,
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity) = 0;
};

} // namespace gameak::runtime::detail
