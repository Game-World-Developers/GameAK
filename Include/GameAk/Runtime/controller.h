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
