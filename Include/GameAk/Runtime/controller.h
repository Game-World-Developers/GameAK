#pragma once

#include "command_producer.h"
#include "ephemeral_producer.h"
#include "state_view.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <functional>

namespace gameak::runtime {

using Controller = std::function<core::Result<void>(StateView&, CommandProducer&, EphemeralProducer&)>;

/// Wraps a Controller with its execution priority.
/// Higher priority values execute before lower ones.
/// Default priority is 0.
struct ControllerEntry {
    Controller controller;
    int priority{0};
};

} // namespace gameak::runtime
