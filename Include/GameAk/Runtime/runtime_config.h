#pragma once

#include "log_level.h"

#include <functional>
#include <span>

namespace gameak::runtime {

/// Optional parallel executor for dispatching controllers within a parallel group.
/// Receives a span of tasks (one per controller in the group) that may execute concurrently.
/// If null, controllers run sequentially (default).
using ParallelTask = std::function<void()>;
using ParallelExecutor = std::function<void(std::span<const ParallelTask>)>;

struct RuntimeConfig {
    LogLevel log_level{LogLevel::Warn};

    /// Optional parallel executor.
    /// If set, controllers in the same parallel group are dispatched via this executor.
    /// The executor must ensure all tasks complete before returning.
    /// If null, controllers run sequentially (safe default).
    ParallelExecutor parallel_executor;
};

} // namespace gameak::runtime
