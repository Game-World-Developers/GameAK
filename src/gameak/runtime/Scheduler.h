#pragma once

#include "Command.h"
#include "DataBlock.h"
#include "gameak/core/Error.h"
#include "gameak/core/Identity.h"

#include <queue>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

class Scheduler {
public:
    void enqueue(Command command);
    core::Result<void> process_pending(
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity);

    size_t pending_count() const { return queue_.size(); }
    size_t executed_count() const { return executed_; }
    size_t rejected_count() const { return rejected_; }

private:
    core::Result<void> validate(const Command& command,
                                const std::unordered_map<core::Identity, DataBlock>& blocks,
                                const std::unordered_map<uint32_t, BlockTypeDescriptor>& types);
    core::Result<void> execute(Command& command,
                               std::unordered_map<core::Identity, DataBlock>& blocks,
                               const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
                               uint64_t& next_identity);

    std::queue<Command> queue_;
    size_t executed_{0};
    size_t rejected_{0};
};

} // namespace gameak::runtime
