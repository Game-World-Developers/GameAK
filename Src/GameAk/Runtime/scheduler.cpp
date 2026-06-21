#include "GameAk/Runtime/fifo_scheduler.h"
#include <utility>

namespace gameak::runtime {

void FifoScheduler::enqueue_impl(Command command) {
    pending_.push_back(std::move(command));
}

core::Result<void> FifoScheduler::process_pending_impl(
    core::rb_tree<core::Identity, DataBlock>& blocks,
    core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
    uint64_t& next_identity) {

    for (auto& command : pending_) {
        process_command(command, blocks, types, next_identity,
                        cancelled_, rejected_details_, history_,
                        executed_, rejected_, skipped_);
    }
    pending_.clear();
    return {};
}

} // namespace gameak::runtime
