#pragma once

#include "gameak/core/Identity.h"
#include "gameak/core/Result.h"

#include <cstdint>
#include <variant>
#include <vector>

namespace gameak::runtime {

enum class CommandType : uint32_t {
    CreateBlock,
    DestroyBlock,
    SetField,
};

struct CommandCreateBlock {
    uint32_t type_id;
};

struct CommandDestroyBlock {
    core::Identity target;
};

struct CommandSetField {
    core::Identity target;
    size_t offset;
    std::vector<std::byte> data;
};

using CommandPayload = std::variant<CommandCreateBlock, CommandDestroyBlock, CommandSetField>;

class Command {
public:
    explicit Command(CommandPayload payload)
        : payload_{std::move(payload)} {}

    CommandType type() const;
    const CommandPayload& payload() const { return payload_; }

private:
    CommandPayload payload_;
};

} // namespace gameak::runtime
