#pragma once

#include "command.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace gameak::runtime {

class DataBlock;
struct BlockTypeDescriptor;
class StateView;
class CommandProducer;
class EphemeralProducer;

using Controller = std::function<core::Result<void>(StateView&, CommandProducer&, EphemeralProducer&)>;

/// Wraps a Controller with its execution priority.
/// Higher priority values execute before lower ones.
/// Default priority is 0.
struct ControllerEntry {
    Controller controller;
    int priority{0};
};

class StateView {
public:
    explicit StateView(const std::unordered_map<core::Identity, DataBlock>& blocks,
                       const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
                       float time_delta = 0.016f)
        : blocks_{blocks}, types_{types}, time_delta_{time_delta} {}

    bool has_block(core::Identity identity) const {
        return blocks_.contains(identity);
    }

    float time_delta() const { return time_delta_; }

private:
    const std::unordered_map<core::Identity, DataBlock>& blocks_;
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types_;
    float time_delta_{0.016f};
};

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

class EphemeralProducer {
public:
    using CreateFn = std::function<core::Result<core::Identity>(uint32_t type_id)>;

    explicit EphemeralProducer(CreateFn create_fn)
        : create_fn_(std::move(create_fn)) {}

    core::Result<core::Identity> create(uint32_t type_id) {
        return create_fn_(type_id);
    }

private:
    CreateFn create_fn_;
};

} // namespace gameak::runtime
