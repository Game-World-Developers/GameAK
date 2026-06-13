#pragma once

#include "gameak/core/Error.h"
#include "gameak/core/Identity.h"
#include "gameak/core/Result.h"

#include <cstdint>
#include <variant>
#include <vector>

namespace gameak::runtime {

using CommandId = uint64_t;

struct RejectedCommand {
    CommandId id;
    core::Error error;
};

enum class CommandType : uint32_t {
    CreateBlock,
    DestroyBlock,
    SetField,
};

struct CommandCreateBlock {
    uint32_t type_id;
};

struct CommandDestroyBlock {
    std::vector<core::Identity> targets;
};

struct CommandSetField {
    std::vector<core::Identity> targets;
    size_t offset;
    std::vector<std::byte> data;
};

using CommandPayload = std::variant<CommandCreateBlock, CommandDestroyBlock, CommandSetField>;

class Command {
public:
    explicit Command(CommandPayload payload, CommandId id = 0)
        : payload_{std::move(payload)}, id_{id} {}

    CommandType type() const;
    const CommandPayload& payload() const { return payload_; }
    CommandId id() const { return id_; }
    void set_id(CommandId id) { id_ = id; }

private:
    CommandPayload payload_;
    CommandId id_{0};
};

} // namespace gameak::runtime
