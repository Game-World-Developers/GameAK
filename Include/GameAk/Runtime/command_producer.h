#pragma once

#include "command.h"
#include "layout_strategy.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"

#include <cstddef>
#include <cstdint>
#include <functional>

namespace gameak::runtime {

class CommandProducer {
public:
    using SubmitFn = std::function<core::Result<CommandId>(Command)>;

    explicit CommandProducer(SubmitFn submit_fn)
        : submit_fn_(std::move(submit_fn)) {}

    core::Result<void> produce(Command command);

    // ── Conversation-style convenience methods ──────────────────────

    core::Result<void> create(uint32_t type_id) {
        return produce(Command::create_block(type_id));
    }

    core::Result<void> destroy(core::Identity target) {
        return produce(Command::destroy_block(target));
    }

    core::Result<void> resize(core::Identity target, size_t new_size) {
        return produce(Command::resize_block(target, new_size));
    }

    core::Result<void> convert(uint32_t type_id, LayoutStrategy new_layout) {
        return produce(Command::convert_layout(type_id, new_layout));
    }

    template <typename T>
    core::Result<void> set(core::Identity target, size_t offset, const T& value) {
        return produce(Command::set_field(target, offset, value));
    }

    core::Result<void> set_raw(core::Identity target, size_t offset,
                                const std::byte* data, size_t size) {
        return produce(Command::set_field_raw(target, offset, data, size));
    }

private:
    SubmitFn submit_fn_;
};

} // namespace gameak::runtime
