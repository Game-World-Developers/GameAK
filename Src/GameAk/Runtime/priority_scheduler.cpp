#include "GameAk/Runtime/priority_scheduler.h"
#include <algorithm>

namespace gameak::runtime {

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
        process_command(command, blocks, types, next_identity,
                        cancelled_, rejected_details_, history_,
                        executed_, rejected_, skipped_);
    }

    pending_.clear();
    return {};
}

} // namespace gameak::runtime
