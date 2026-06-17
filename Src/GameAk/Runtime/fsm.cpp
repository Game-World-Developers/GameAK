#include "GameAk/Runtime/fsm.h"
#include "GameAk/Runtime/data_block.h"

#include <memory>
#include <queue>
#include <unordered_map>

namespace gameak::runtime {

FsmBuilder& FsmBuilder::initial_state(FsmState state) {
    initial_state_ = std::move(state);
    has_initial_ = true;
    return *this;
}

FsmBuilder& FsmBuilder::add_state(FsmState state) {
    states_.insert(std::move(state));
    return *this;
}

FsmBuilder& FsmBuilder::add_transition(FsmState from, FsmEventType event, FsmState to) {
    transitions_[std::move(from)][std::move(event)] = std::move(to);
    return *this;
}

FsmBuilder& FsmBuilder::on_entry(FsmState state, FsmAction action) {
    entry_actions_[std::move(state)] = std::move(action);
    return *this;
}

FsmBuilder& FsmBuilder::on_exit(FsmState state, FsmAction action) {
    exit_actions_[std::move(state)] = std::move(action);
    return *this;
}

Controller FsmBuilder::build() {
    struct FsmStateMachine {
        FsmState current_state;
        std::unordered_set<FsmState> states;
        std::unordered_map<FsmState, std::unordered_map<FsmEventType, FsmState>> transitions;
        std::unordered_map<FsmState, FsmAction> entry_actions;
        std::unordered_map<FsmState, FsmAction> exit_actions;
        std::queue<FsmEventType> event_queue;

        void enqueue_event(FsmEventType event) {
            event_queue.push(std::move(event));
        }
    };

    if (!has_initial_ && !states_.empty()) {
        initial_state_ = *states_.begin();
    }

    auto fsm = std::make_shared<FsmStateMachine>();
    fsm->current_state = initial_state_;
    fsm->states = std::move(states_);
    fsm->transitions = std::move(transitions_);
    fsm->entry_actions = std::move(entry_actions_);
    fsm->exit_actions = std::move(exit_actions_);

    // Ensure initial state is in the set
    fsm->states.insert(fsm->current_state);

    return [fsm](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) -> core::Result<void> {
        (void)view;

        // Process all queued events
        while (!fsm->event_queue.empty()) {
            auto event = std::move(fsm->event_queue.front());
            fsm->event_queue.pop();

            auto trans_it = fsm->transitions.find(fsm->current_state);
            if (trans_it == fsm->transitions.end()) continue;

            auto event_it = trans_it->second.find(event);
            if (event_it == trans_it->second.end()) continue;

            FsmState next_state = event_it->second;

            // Exit action
            auto exit_it = fsm->exit_actions.find(fsm->current_state);
            if (exit_it != fsm->exit_actions.end()) {
                exit_it->second(view, producer, ephem);
            }

            // Transition
            fsm->current_state = std::move(next_state);

            // Entry action
            auto entry_it = fsm->entry_actions.find(fsm->current_state);
            if (entry_it != fsm->entry_actions.end()) {
                entry_it->second(view, producer, ephem);
            }
        }

        return {};
    };
}

} // namespace gameak::runtime
