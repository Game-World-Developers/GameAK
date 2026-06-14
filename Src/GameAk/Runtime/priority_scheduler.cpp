#include "GameAk/Runtime/priority_scheduler.h"
#include "GameAk/Runtime/scheduler_ops.h"
#include <algorithm>

namespace gameak::runtime {

core::Result<void> PriorityScheduler::validate(
    const Command& command,
    const std::unordered_map<core::Identity, DataBlock>& blocks,
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types) {
    return detail::validate_command(command, blocks, types);
}

core::Result<void> PriorityScheduler::execute(
    Command& command,
    std::unordered_map<core::Identity, DataBlock>& blocks,
    const std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity) {
    return detail::execute_command(command, blocks, types, next_identity);
}

core::Result<void> PriorityScheduler::process_pending_impl(
    std::unordered_map<core::Identity, DataBlock>& blocks,
    std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity) {

    // Sort by priority (higher = first)
    std::sort(pending_.begin(), pending_.end(),
        [this](const Command& a, const Command& b) {
            int pa = priority_fn_ ? priority_fn_(a) : 0;
            int pb = priority_fn_ ? priority_fn_(b) : 0;
            return pa > pb;
        });

    for (auto& command : pending_) {
        if (cancelled_.contains(command.id())) {
            cancelled_.erase(command.id());
            skipped_++;
            continue;
        }

        auto validation = validate(command, blocks, types);
        if (!validation) {
            rejected_++;
            rejected_details_.push_back({command.id(), validation.error()});
            continue;
        }

        auto execution = execute(command, blocks, types, next_identity);
        if (!execution) {
            rejected_++;
            rejected_details_.push_back({command.id(), execution.error()});
        } else {
            history_.push_back(command);
            executed_++;
        }
    }

    pending_.clear();
    return {};
}

} // namespace gameak::runtime
