#pragma once

#include "layout_strategy.h"

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
    ConvertLayout,
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

struct CommandConvertLayout {
    uint32_t type_id;
    LayoutStrategy new_layout;
    AoSoAConfig aosoa_config{};
};

using CommandPayload = std::variant<CommandCreateBlock, CommandDestroyBlock,
                                     CommandSetField, CommandResizeBlock,
                                     CommandConvertLayout>;

class Command {
    template <typename> friend class Runtime;
public:
    explicit Command(CommandPayload payload, CommandId id = 0)
        : payload_{std::move(payload)}, id_{id} {}

    CommandType type() const;
    const CommandPayload& payload() const { return payload_; }
    CommandId id() const { return id_; }

    // ── Conversation-style factories ────────────────────────────────

    static Command create_block(uint32_t type_id) {
        return Command{CommandPayload{CommandCreateBlock{type_id}}};
    }

    static Command destroy_block(core::Identity target) {
        return Command{CommandPayload{CommandDestroyBlock{{target}}}};
    }

    static Command destroy_blocks(std::vector<core::Identity> targets) {
        return Command{CommandPayload{CommandDestroyBlock{std::move(targets)}}};
    }

    static Command resize_block(core::Identity target, size_t new_size) {
        return Command{CommandPayload{CommandResizeBlock{{target}, new_size}}};
    }

    static Command resize_blocks(std::vector<core::Identity> targets, size_t new_size) {
        return Command{CommandPayload{CommandResizeBlock{std::move(targets), new_size}}};
    }

    static Command convert_layout(uint32_t type_id, LayoutStrategy new_layout) {
        return Command{CommandPayload{CommandConvertLayout{type_id, new_layout}}};
    }

    template <typename T>
    static Command set_field(core::Identity target, size_t offset, const T& value) {
        const auto* ptr = reinterpret_cast<const std::byte*>(&value);
        std::vector<std::byte> data(ptr, ptr + sizeof(T));
        return Command{CommandPayload{CommandSetField{{target}, offset, std::move(data)}}};
    }

    static Command set_field_raw(core::Identity target, size_t offset,
                                  const std::byte* data, size_t size) {
        std::vector<std::byte> bytes(data, data + size);
        return Command{CommandPayload{CommandSetField{{target}, offset, std::move(bytes)}}};
    }

private:
    void set_id(CommandId id) { id_ = id; }

    CommandPayload payload_;
    CommandId id_{0};
};

} // namespace gameak::runtime
