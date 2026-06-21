#pragma once

#include "command.h"
#include "block_type.h"
#include "data_block.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/rb_tree.h"
#include "GameAk/Core/result.h"

#include <cstdint>

namespace gameak::runtime::detail {

class ICommandHandler {
public:
    virtual ~ICommandHandler() = default;
    virtual CommandType type() const = 0;
    virtual core::Result<void> validate(
        const CommandPayload& payload,
        const core::rb_tree<core::Identity, DataBlock>& blocks,
        const core::rb_tree<uint32_t, BlockTypeDescriptor>& types) = 0;
    virtual core::Result<void> execute(
        CommandPayload& payload,
        core::rb_tree<core::Identity, DataBlock>& blocks,
        core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity) = 0;
};

} // namespace gameak::runtime::detail
