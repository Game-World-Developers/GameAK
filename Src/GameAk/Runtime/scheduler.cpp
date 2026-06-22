#include "GameAk/Runtime/fifo_scheduler.h"
#include <utility>

namespace gameak::runtime {

void FifoScheduler::enqueue_impl(Command command) {
    pending_.push_back(std::move(command));
}

core::Result<void> FifoScheduler::process_pending_impl(
    CommandContext& ctx) {

    for (auto& command : pending_) {
        process_command(command, cancelled_, rejected_details_, history_,
                        executed_, rejected_, skipped_, ctx);
    }
    pending_.clear();
    return {};
}

} // namespace gameak::runtime
