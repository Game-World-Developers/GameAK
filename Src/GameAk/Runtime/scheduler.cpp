#include "GameAk/Runtime/fifo_scheduler.h"
#include <utility>

namespace gameak::runtime {

void FifoScheduler::enqueue_impl(Command command) {
    queue_.push(std::move(command));
}

core::Result<void> FifoScheduler::process_pending_impl(
    std::unordered_map<core::Identity, DataBlock>& blocks,
    std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity) {

    while (!queue_.empty()) {
        auto command = std::move(queue_.front());
        queue_.pop();
        process_command(command, blocks, types, next_identity,
                        cancelled_, rejected_details_, history_,
                        executed_, rejected_, skipped_);
    }
    return {};
}

} // namespace gameak::runtime
