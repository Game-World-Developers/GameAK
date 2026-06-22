#pragma once

#include "scheduler_base.h"
#include "GameAk/Core/flat_vector.h"

namespace gameak::runtime {

class FifoScheduler : public SchedulerBase<FifoScheduler> {
    friend class SchedulerBase<FifoScheduler>;
public:
private:
    void enqueue_impl(Command command);
    core::Result<void> process_pending_impl(
        CommandContext& ctx);

    void reset_counts_impl() {
        executed_ = 0;
        rejected_ = 0;
        skipped_ = 0;
    }

    void cancel_impl(CommandId id) { cancelled_.push_back(id); }

    core::flat_vector<RejectedCommand, 4> take_rejected_impl() {
        return std::move(rejected_details_);
    }

    size_t pending_count_impl() const { return pending_.size(); }
    size_t executed_count_impl() const { return executed_; }
    size_t rejected_count_impl() const { return rejected_; }
    size_t skipped_count_impl() const { return skipped_; }

    const core::flat_vector<Command>& history_impl() const { return history_; }
    void clear_history_impl() { history_.clear(); }

    core::flat_vector<Command, 4> pending_;
    core::flat_vector<CommandId> cancelled_;
    core::flat_vector<RejectedCommand, 4> rejected_details_;
    core::flat_vector<Command> history_;
    size_t executed_{0};
    size_t rejected_{0};
    size_t skipped_{0};
};

} // namespace gameak::runtime
