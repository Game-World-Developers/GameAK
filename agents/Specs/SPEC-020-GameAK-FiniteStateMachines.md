# SPEC-020: Finite State Machines

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-17

---

## Summary

This specification defines Finite State Machines (FSM) as a runtime state transformation mechanism.

An FSM is a Controller that transitions between states based on events, producing Commands as the result of each transition.

FSMs provide deterministic, structured state transformation logic.

---

## Behavior

### Scenario: FSM starts in initial state

Given an FSM with states {Idle, Active, Paused} and initial state = Idle

When the FSM is executed for the first time

Then it produces no Commands and remains in Idle state

**Test:** `test_fsm.cpp` — `fsm_starts_in_initial_state`

### Scenario: FSM transitions on event

Given an FSM in Idle state with transition Idle -> Active on event Start

When the FSM receives event Start

Then it transitions to Active and exit(Idle) and enter(Active) actions are called

**Test:** `test_fsm.cpp` — `fsm_transitions_on_event`

### Scenario: FSM produces Commands during transition

Given an FSM in Idle state with transition Idle -> Active on event Start, producing SetField command

When the FSM processes event Start

Then the CommandProducer receives the SetField command

**Test:** `test_fsm.cpp` — `fsm_produces_commands_on_transition`

### Scenario: FSM ignores undefined transitions

Given an FSM in Idle state with no transition defined for event Pause

When the FSM receives event Pause

Then it remains in Idle state and produces no Commands

**Test:** `test_fsm.cpp` — `fsm_ignores_undefined_transitions`

### Scenario: FSM supports entry actions

Given an FSM transitioning to Active state with entry action

When the transition completes

Then the entry action is executed and its resulting Commands are produced

**Test:** `test_fsm.cpp` — `fsm_entry_actions`

### Scenario: FSM supports exit actions

Given an FSM transitioning out of Idle state with exit action

When the transition executes

Then the exit action is executed before the transition

**Test:** `test_fsm.cpp` — `fsm_exit_actions`

### Scenario: FSM is deterministic

Given an FSM in a known state

When the same event is processed twice

Then both executions produce identical Commands and end in the same state

**Test:** `test_fsm.cpp` — `fsm_deterministic_execution`

---

## Constraints

* FSM must be a Controller (callable type receiving StateView + CommandProducer).
  * **Test verification:** `test_fsm.cpp` — `fsm_is_controller`
* FSM must not hold mutable state between ticks.
  * **Test verification:** `test_fsm.cpp` — `fsm_no_mutable_state`
* All transitions must be explicitly defined (no implicit transitions).
  * **Test verification:** `test_fsm.cpp` — `fsm_explicit_transitions_only`
* FSM may only have one active state at a time.
  * **Test verification:** `test_fsm.cpp` — `fsm_single_active_state`
* Events are passed through a dedicated FSM event channel.
  * **Test verification:** `test_fsm.cpp` — `fsm_event_channel`

---

## Out of Scope

* Hierarchical / nested state machines.
* Orthogonal regions / concurrent states.
* Guards (conditional transitions) — deferred to a future spec.
* FSM composition (multiple FSMs running as one).

---

## Open Questions

* [x] Should events be a dedicated type or generic Data Blocks?

**Answer:** Dedicated type. FSM events are a dedicated type, not generic Data Blocks.

* [x] Should the FSM be defined declaratively (e.g., DSL or builder) or imperatively?

**Answer:** Both. FSM supports both declarative (builder/DSL) and imperative (add_state, add_transition) definition.

* [x] How are events submitted to the FSM — via Command or direct API?

**Answer:** Command. Events are submitted to the FSM via Command.

* [x] Should FSM support timed/automatic transitions?

**Answer:** Yes. FSM supports timed/automatic transitions.

* [x] Should entry/exit actions be allowed to produce multiple Commands?

**Answer:** Yes. Entry/exit actions may produce multiple Commands.

---

## Definitions

### FSM (Finite State Machine)

A computation model consisting of a finite set of states, a set of transitions between states, and actions executed on entry/exit/transition.

### State

A distinct mode of operation within an FSM. Only one state is active at a time.

### Transition

A directed edge between two states, triggered by an event.

### Event

An input that triggers a transition in the FSM.

### Entry Action

A set of operations executed when entering a state.

### Exit Action

A set of operations executed when leaving a state.
