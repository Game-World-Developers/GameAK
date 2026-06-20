#pragma once

#include "scheduler_base.h"

#include <queue>
#include <unordered_set>
#include <vector>

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

    void reset_counts_impl() {
        executed_ = 0;
        rejected_ = 0;
        skipped_ = 0;
    }

    void cancel_impl(CommandId id) { cancelled_.insert(id); }

    std::vector<RejectedCommand> take_rejected_impl() {
        return std::move(rejected_details_);
    }

    size_t pending_count_impl() const { return queue_.size(); }
    size_t executed_count_impl() const { return executed_; }
    size_t rejected_count_impl() const { return rejected_; }
    size_t skipped_count_impl() const { return skipped_; }

    const std::vector<Command>& history_impl() const { return history_; }
    void clear_history_impl() { history_.clear(); }

    std::queue<Command> queue_;
    std::unordered_set<CommandId> cancelled_;
    std::vector<RejectedCommand> rejected_details_;
    std::vector<Command> history_;
    size_t executed_{0};
    size_t rejected_{0};
    size_t skipped_{0};
};

} // namespace gameak::runtime
