#pragma once

#include "Command.h"
#include "DataBlock.h"
#include "gameak/core/Error.h"
#include "gameak/core/Identity.h"

#include <cstdint>
#include <unordered_map>

namespace gameak::runtime {

class Scheduler {
public:
    virtual ~Scheduler() = default;

    virtual void enqueue(Command command) = 0;
    virtual core::Result<void> process_pending(
        std::unordered_map<core::Identity, DataBlock>& blocks,
        std::unordered_map<uint32_t, BlockTypeDescriptor>& types,
        uint64_t& next_identity) = 0;

    virtual size_t pending_count() const = 0;
    virtual size_t executed_count() const = 0;
    virtual size_t rejected_count() const = 0;
};

} // namespace gameak::runtime
