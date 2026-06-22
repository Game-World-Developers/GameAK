#pragma once

#include "scheduler_base.h"
#include "GameAk/Core/flat_vector.h"

#include <algorithm>
#include <functional>

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

    PriorityFn priority_fn_;
    core::flat_vector<Command> pending_;
    core::flat_vector<CommandId> cancelled_;
    core::flat_vector<RejectedCommand, 4> rejected_details_;
    core::flat_vector<Command> history_;
    size_t executed_{0};
    size_t rejected_{0};
    size_t skipped_{0};
};

} // namespace gameak::runtime
