#pragma once

#include "Command.h"
#include "gameak/core/Identity.h"
#include "gameak/core/Result.h"

#include <cstdint>
#include <functional>
#include <unordered_map>

namespace gameak::runtime {

class DataBlock;
struct BlockTypeDescriptor;
class StateView;
class CommandProducer;

using Controller = std::function<core::Result<void>(StateView&, CommandProducer&)>;

class StateView {
public:
    explicit StateView(const std::unordered_map<core::Identity, DataBlock>& blocks,
                       const std::unordered_map<uint32_t, BlockTypeDescriptor>& types)
        : blocks_{blocks}, types_{types} {}

    bool has_block(core::Identity identity) const {
        return blocks_.contains(identity);
    }

private:
    const std::unordered_map<core::Identity, DataBlock>& blocks_;
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types_;
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

} // namespace gameak::runtime
