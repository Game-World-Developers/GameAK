#pragma once

#include "command.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/result.h"

#include <cstdint>

namespace gameak::runtime {

inline constexpr float kDefaultTimeDelta = 0.016f;

enum class ExecutionStatus : uint32_t {
    Success,
    PartialFailure,
    CriticalFailure,
};

struct TickResult {
    size_t commands_executed{0};
    size_t commands_rejected{0};
    size_t controllers_executed{0};
    ExecutionStatus status{ExecutionStatus::Success};
    core::flat_vector<RejectedCommand, 4> rejected_commands;
};

} // namespace gameak::runtime
