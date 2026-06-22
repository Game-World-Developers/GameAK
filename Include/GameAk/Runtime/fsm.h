#pragma once

#include "controller.h"
#include "command.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/rb_tree.h"
#include "GameAk/Core/result.h"

#include <functional>
#include <memory>
#include <string>

namespace gameak::runtime {

using FsmAction = std::function<void(StateView&, CommandProducer&, EphemeralProducer&)>;

template <typename StateT = std::string, typename EventT = std::string>
class Fsm {
    struct InternalState {
        StateT current_state;
        core::flat_vector<EventT, 4> event_queue;
    };

    struct TransitionTable {
        core::rb_tree<StateT, bool> states;
        core::rb_tree<StateT, core::rb_tree<EventT, StateT>> transitions;
        core::rb_tree<StateT, FsmAction> entry_actions;
        core::rb_tree<StateT, FsmAction> exit_actions;
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
        table_.states.insert(std::move(state), true);
        return *this;
    }

    Fsm& add_transition(StateT from, EventT event, StateT to) {
        auto it = table_.transitions.find(from);
        if (it != table_.transitions.end()) {
            it->second.insert(event, std::move(to));
        } else {
            core::rb_tree<EventT, StateT> inner;
            inner.insert(event, std::move(to));
            table_.transitions.insert(std::move(from), std::move(inner));
        }
        return *this;
    }

    Fsm& on_entry(StateT state, FsmAction action) {
        table_.entry_actions.insert(std::move(state), std::move(action));
        return *this;
    }

    Fsm& on_exit(StateT state, FsmAction action) {
        table_.exit_actions.insert(std::move(state), std::move(action));
        return *this;
    }

    void enqueue_event(EventT event) {
        state_->event_queue.push_back(std::move(event));
    }

    size_t pending_events() const {
        return state_->event_queue.size();
    }

    Controller build() {
        auto table = std::make_shared<TransitionTable>(std::move(table_));

        // The controller gets its own state, decoupled from the builder.
        // Events must be enqueued before build().
        auto ctrl_state = std::make_shared<InternalState>();
        ctrl_state->current_state = std::move(state_->current_state);
        ctrl_state->event_queue = std::move(state_->event_queue);

        // Reset builder state so build() is idempotent and builder no longer
        // shares mutable state with the controller.
        state_ = std::make_shared<InternalState>();

        StateT initial = initial_state_;
        if (!has_initial_ && !table->states.empty()) {
            initial = table->states.begin()->first;
        }
        table->states.insert(initial, true);
        ctrl_state->current_state = initial;

        return [table, ctrl_state](StateView& view, CommandProducer& producer,
                                    EphemeralProducer& ephem) -> core::Result<void> {

            for (auto& event : ctrl_state->event_queue) {

                auto trans_it = table->transitions.find(ctrl_state->current_state);
                if (trans_it == table->transitions.end()) continue;

                auto event_it = trans_it->second.find(event);
                if (event_it == trans_it->second.end()) continue;

                StateT next_state = event_it->second;

                auto exit_it = table->exit_actions.find(ctrl_state->current_state);
                if (exit_it != table->exit_actions.end()) {
                    exit_it->second(view, producer, ephem);
                }

                ctrl_state->current_state = std::move(next_state);

                auto entry_it = table->entry_actions.find(ctrl_state->current_state);
                if (entry_it != table->entry_actions.end()) {
                    entry_it->second(view, producer, ephem);
                }
            }
            ctrl_state->event_queue.clear();

            return {};
        };
    }
};

} // namespace gameak::runtime
