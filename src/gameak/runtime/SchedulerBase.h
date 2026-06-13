#pragma once

#include "Command.h"
#include "DataBlock.h"
#include "gameak/core/Error.h"
#include "gameak/core/Identity.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace gameak::runtime {

template <typename Derived>
class SchedulerBase {
public:
    void enqueue(Command command) {
        derived().enqueue_impl(std::move(command));
    }

    core::Result<void> process_pending(
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity) {
        return derived().process_pending_impl(blocks, types, next_identity);
    }

    void cancel(CommandId id) {
        derived().cancel_impl(id);
    }

    std::vector<RejectedCommand> take_rejected() {
        return derived().take_rejected_impl();
    }

    size_t pending_count() const { return derived().pending_count_impl(); }
    size_t executed_count() const { return derived().executed_count_impl(); }
    size_t rejected_count() const { return derived().rejected_count_impl(); }
    size_t skipped_count() const { return derived().skipped_count_impl(); }

protected:
    ~SchedulerBase() = default;

private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

} // namespace gameak::runtime
