#pragma once

#include "command.h"
#include "data_block.h"
#include "command_dispatcher.h"
#include "GameAk/Core/error.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/identity.h"
#include "GameAk/Core/rb_tree.h"

#include <cstdint>

namespace gameak::runtime {

template <typename Derived>
class SchedulerBase {
public:
    void enqueue(Command command) {
        derived().enqueue_impl(std::move(command));
    }

    core::Result<void> process_pending(
        core::rb_tree<core::Identity, DataBlock>& blocks,
        core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity) {
        return derived().process_pending_impl(blocks, types, next_identity);
    }

    void cancel(CommandId id) {
        derived().cancel_impl(id);
    }

    core::flat_vector<RejectedCommand, 4> take_rejected() {
        return derived().take_rejected_impl();
    }

    void reset_counts() { derived().reset_counts_impl(); }

    size_t pending_count() const { return derived().pending_count_impl(); }
    size_t executed_count() const { return derived().executed_count_impl(); }
    size_t rejected_count() const { return derived().rejected_count_impl(); }
    size_t skipped_count() const { return derived().skipped_count_impl(); }

    const core::flat_vector<Command, 1>& history() const { return derived().history_impl(); }
    void clear_history() { derived().clear_history_impl(); }

protected:
    ~SchedulerBase() = default;

    /// Shared per-command processing logic (validate → execute → track results).
    /// Returns false if the command was cancelled and should be skipped.
    bool process_command(
        Command& command,
        core::rb_tree<core::Identity, DataBlock>& blocks,
        core::rb_tree<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity,
        core::flat_vector<CommandId, 4>& cancelled,
        core::flat_vector<RejectedCommand, 4>& rejected_details,
        core::flat_vector<Command, 1>& history,
        size_t& executed,
        size_t& rejected,
        size_t& skipped)
    {
        for (size_t i = 0; i < cancelled.size(); ++i) {
            if (cancelled[i] == command.id()) {
                cancelled.erase(cancelled.begin() + i);
                skipped++;
                return false;
            }
        }

        auto validation = detail::validate_command(command, blocks, types);
        if (!validation) {
            rejected++;
            rejected_details.push_back({command.id(), validation.error()});
            return false;
        }

        auto execution = detail::execute_command(command, blocks, types, next_identity);
        if (!execution) {
            rejected++;
            rejected_details.push_back({command.id(), execution.error()});
        } else {
            history.push_back(command);
            executed++;
        }
        return true;
    }

private:
    Derived& derived() { return static_cast<Derived&>(*this); }
    const Derived& derived() const { return static_cast<const Derived&>(*this); }
};

} // namespace gameak::runtime
