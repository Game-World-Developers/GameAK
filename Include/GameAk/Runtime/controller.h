#pragma once

#include "command_producer.h"
#include "ephemeral_producer.h"
#include "state_view.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/result.h"

#include <cstdint>
#include <functional>

namespace gameak::runtime {

using Controller = std::function<core::Result<void>(StateView&, CommandProducer&, EphemeralProducer&)>;

/// Wraps a Controller with its execution priority and optional type access declaration.
/// Higher priority values execute before lower ones.
/// Default priority is 0.
/// type_access declares which block type_ids the controller may access.
/// Controllers with disjoint type_access sets may execute in parallel.
struct ControllerEntry {
    Controller controller;
    int priority{0};
    core::flat_vector<uint32_t, 4> type_access;
};

} // namespace gameak::runtime
