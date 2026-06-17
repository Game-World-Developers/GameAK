#pragma once

#include "controller.h"
#include "command.h"
#include "GameAk/Core/result.h"

#include <functional>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace gameak::runtime {

using FsmState = std::string;
using FsmEventType = std::string;

using FsmAction = std::function<void(StateView&, CommandProducer&, EphemeralProducer&)>;

struct FsmTransitionDef {
    FsmState from;
    FsmEventType event;
    FsmState to;
};

class FsmBuilder {
public:
    FsmBuilder& initial_state(FsmState state);

    FsmBuilder& add_state(FsmState state);

    FsmBuilder& add_transition(FsmState from, FsmEventType event, FsmState to);

    FsmBuilder& on_entry(FsmState state, FsmAction action);

    FsmBuilder& on_exit(FsmState state, FsmAction action);

    /// Build the FSM Controller.
    /// Returns a callable compatible with the Controller signature.
    Controller build();

private:
    FsmState initial_state_;
    bool has_initial_{false};
    std::unordered_set<FsmState> states_;
    std::unordered_map<FsmState, std::unordered_map<FsmEventType, FsmState>> transitions_;
    std::unordered_map<FsmState, FsmAction> entry_actions_;
    std::unordered_map<FsmState, FsmAction> exit_actions_;
};

/// Command to submit an event to an FSM-managed block.
/// The FSM Controller reads events from a queue attached to a Data Block.
struct CommandSubmitFsmEvent {
    core::Identity target_block;
    FsmEventType event;
};

} // namespace gameak::runtime
