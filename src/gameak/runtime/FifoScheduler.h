#pragma once

#include "SchedulerBase.h"

#include <queue>
#include <unordered_set>

namespace gameak::runtime {

class FifoScheduler : public SchedulerBase<FifoScheduler> {
    friend class SchedulerBase<FifoScheduler>;
public:
private:
    void enqueue_impl(Command command);
    core::Result<void> process_pending_impl(
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity);

    void cancel_impl(CommandId id) { cancelled_.insert(id); }

    std::vector<RejectedCommand> take_rejected_impl() {
        return std::move(rejected_details_);
    }

    size_t pending_count_impl() const { return queue_.size(); }
    size_t executed_count_impl() const { return executed_; }
    size_t rejected_count_impl() const { return rejected_; }
    size_t skipped_count_impl() const { return skipped_; }

    core::Result<void> validate(const Command& command,
                                const std::unordered_map<core::Identity, DataBlock>& blocks,
                                const std::unordered_map<uint32_t, BlockTypeDescriptor>& types);
    core::Result<void> execute(Command& command,
                               std::unordered_map<core::Identity, DataBlock>& blocks,
                               const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
                               uint64_t& next_identity);

    std::queue<Command> queue_;
    std::unordered_set<CommandId> cancelled_;
    std::vector<RejectedCommand> rejected_details_;
    size_t executed_{0};
    size_t rejected_{0};
    size_t skipped_{0};
};

} // namespace gameak::runtime
