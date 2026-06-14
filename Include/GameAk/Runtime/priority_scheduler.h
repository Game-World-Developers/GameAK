#pragma once

#include "scheduler_base.h"

#include <algorithm>
#include <functional>
#include <queue>
#include <unordered_set>
#include <vector>

namespace gameak::runtime {

class PriorityScheduler : public SchedulerBase<PriorityScheduler> {
    friend class SchedulerBase<PriorityScheduler>;
public:
    using PriorityFn = std::function<int(const Command&)>;

    explicit PriorityScheduler(PriorityFn fn = nullptr)
        : priority_fn_{std::move(fn)} {}

    void set_priority_fn(PriorityFn fn) { priority_fn_ = std::move(fn); }

private:
    void enqueue_impl(Command command) {
        pending_.push_back(std::move(command));
    }

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

    size_t pending_count_impl() const { return pending_.size(); }
    size_t executed_count_impl() const { return executed_; }
    size_t rejected_count_impl() const { return rejected_; }
    size_t skipped_count_impl() const { return skipped_; }

    const std::vector<Command>& history_impl() const { return history_; }
    void clear_history_impl() { history_.clear(); }

    core::Result<void> validate(const Command& command,
                                const std::unordered_map<core::Identity, DataBlock>& blocks,
                                const std::unordered_map<uint32_t, BlockTypeDescriptor>& types);
    core::Result<void> execute(Command& command,
                               std::unordered_map<core::Identity, DataBlock>& blocks,
                               const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
                               uint64_t& next_identity);

    PriorityFn priority_fn_;
    std::vector<Command> pending_;
    std::unordered_set<CommandId> cancelled_;
    std::vector<RejectedCommand> rejected_details_;
    std::vector<Command> history_;
    size_t executed_{0};
    size_t rejected_{0};
    size_t skipped_{0};
};

} // namespace gameak::runtime
