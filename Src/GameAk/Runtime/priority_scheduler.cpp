#include "GameAk/Runtime/priority_scheduler.h"
#include <algorithm>

namespace gameak::runtime {

core::Result<void> PriorityScheduler::process_pending_impl(
    CommandContext& ctx) {

    // Sort by priority (higher = first)
    std::sort(pending_.begin(), pending_.end(),
        [this](const Command& a, const Command& b) {
            int pa = priority_fn_ ? priority_fn_(a) : 0;
            int pb = priority_fn_ ? priority_fn_(b) : 0;
            return pa > pb;
        });

    for (auto& command : pending_) {
        process_command(command, cancelled_, rejected_details_, history_,
                        executed_, rejected_, skipped_, ctx);
    }

    pending_.clear();
    return {};
}

} // namespace gameak::runtime
