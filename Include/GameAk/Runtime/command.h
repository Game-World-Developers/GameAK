#pragma once

#include "GameAk/Core/error.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <variant>
#include <vector>

namespace gameak::runtime {

template <typename SchedulerType> class Runtime;

using CommandId = uint64_t;

struct RejectedCommand {
    CommandId id;
    core::Error error;
};

enum class CommandType : uint32_t {
    CreateBlock,
    DestroyBlock,
    SetField,
    ResizeBlock,
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

struct CommandResizeBlock {
    std::vector<core::Identity> targets;
    size_t new_size;
};

using CommandPayload = std::variant<CommandCreateBlock, CommandDestroyBlock, CommandSetField, CommandResizeBlock>;

class Command {
    template <typename> friend class Runtime;
public:
    explicit Command(CommandPayload payload, CommandId id = 0)
        : payload_{std::move(payload)}, id_{id} {}

    CommandType type() const;
    const CommandPayload& payload() const { return payload_; }
    CommandId id() const { return id_; }

private:
    void set_id(CommandId id) { id_ = id; }

    CommandPayload payload_;
    CommandId id_{0};
};

} // namespace gameak::runtime
