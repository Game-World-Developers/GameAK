#pragma once

#include "controller.h"
#include "command.h"
#include "GameAk/Core/result.h"

#include <functional>
#include <map>
#include <memory>
#include <queue>
#include <set>
#include <string>
#include <vector>

namespace gameak::runtime {

using FsmAction = std::function<void(StateView&, CommandProducer&, EphemeralProducer&)>;

template <typename StateT = std::string, typename EventT = std::string>
class Fsm {
    struct InternalState {
        StateT current_state;
        std::queue<EventT> event_queue;
    };

    struct TransitionTable {
        std::set<StateT> states;
        std::map<StateT, std::map<EventT, StateT>> transitions;
        std::map<StateT, FsmAction> entry_actions;
        std::map<StateT, FsmAction> exit_actions;
    };

    StateT initial_state_;
    bool has_initial_{false};
    TransitionTable table_;
    std::shared_ptr<InternalState> state_;

public:
    Fsm()
        : state_(std::make_shared<InternalState>()) {}

    Fsm& initial_state(StateT state) {
        initial_state_ = std::move(state);
        has_initial_ = true;
        return *this;
    }

    Fsm& add_state(StateT state) {
        table_.states.insert(std::move(state));
        return *this;
    }

    Fsm& add_transition(StateT from, EventT event, StateT to) {
        table_.transitions[std::move(from)][std::move(event)] = std::move(to);
        return *this;
    }

    Fsm& on_entry(StateT state, FsmAction action) {
        table_.entry_actions[std::move(state)] = std::move(action);
        return *this;
    }

    Fsm& on_exit(StateT state, FsmAction action) {
        table_.exit_actions[std::move(state)] = std::move(action);
        return *this;
    }

    void enqueue_event(EventT event) {
        state_->event_queue.push(std::move(event));
    }

    size_t pending_events() const {
        return state_->event_queue.size();
    }

    Controller build() {
        auto table = std::make_shared<TransitionTable>(std::move(table_));
        auto state = state_;

        StateT initial = initial_state_;
        if (!has_initial_ && !table->states.empty()) {
            initial = *table->states.begin();
        }
        table->states.insert(initial);
        state->current_state = initial;

        // Note: table_ was moved into shared_ptr; build() is idempotent

        return [table, state](StateView& view, CommandProducer& producer,
                               EphemeralProducer& ephem) -> core::Result<void> {

            while (!state->event_queue.empty()) {
                auto event = std::move(state->event_queue.front());
                state->event_queue.pop();

                auto trans_it = table->transitions.find(state->current_state);
                if (trans_it == table->transitions.end()) continue;

                auto event_it = trans_it->second.find(event);
                if (event_it == trans_it->second.end()) continue;

                StateT next_state = event_it->second;

                auto exit_it = table->exit_actions.find(state->current_state);
                if (exit_it != table->exit_actions.end()) {
                    exit_it->second(view, producer, ephem);
                }

                state->current_state = std::move(next_state);

                auto entry_it = table->entry_actions.find(state->current_state);
                if (entry_it != table->entry_actions.end()) {
                    entry_it->second(view, producer, ephem);
                }
            }

            return {};
        };
    }
};

} // namespace gameak::runtime
