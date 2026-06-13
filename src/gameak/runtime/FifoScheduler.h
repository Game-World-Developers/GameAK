#pragma once

#include "Scheduler.h"

#include <queue>

namespace gameak::runtime {

class FifoScheduler : public Scheduler {
public:
    void enqueue(Command command) override;
    core::Result<void> process_pending(
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity) override;

    size_t pending_count() const override { return queue_.size(); }
    size_t executed_count() const override { return executed_; }
    size_t rejected_count() const override { return rejected_; }

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
