# SPEC-023: Event Loops

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-17

---

## Summary

This specification defines Event Loops as a runtime execution mechanism.

An Event Loop is a Controller that processes a queue of events. Each event is dispatched to registered handlers, which may produce Commands in response.

The Event Loop is distinct from the notification-based Event System (SPEC-017). SPEC-017 notifies listeners of runtime lifecycle events. SPEC-023 defines a general-purpose event-driven execution model where user-defined events drive state transformation.

---

## Behavior

### Scenario: Event is dispatched to registered handlers

Given an Event Loop with a handler registered for "PlayerJump"

When a PlayerJump event is enqueued and the Event Loop executes

Then the handler is called and may produce Commands

**Test:** `test_event_loop.cpp` — `event_dispatches_to_handler`

### Scenario: Event with no handlers is silently consumed

Given an Event Loop with no handlers for "UnknownEvent"

When an UnknownEvent is enqueued

Then the Event Loop executes without error and produces no Commands

**Test:** `test_event_loop.cpp` — `event_with_no_handlers`

### Scenario: Multiple handlers for the same event all execute

Given an Event Loop with handlers H1 and H2 for "ScoreChanged"

When a ScoreChanged event is enqueued

Then both H1 and H2 are called in registration order

**Test:** `test_event_loop.cpp` — `multiple_handlers_for_event`

### Scenario: Events are processed in FIFO order

Given events E1, E2, E3 enqueued in that order

When the Event Loop executes

Then E1 is processed first, then E2, then E3

**Test:** `test_event_loop.cpp` — `fifo_event_processing`

### Scenario: Handlers receive event data

Given a handler registered for "TakeDamage" expecting {amount: int, source: Identity}

When a TakeDamage event with {amount: 10, source: id_42} is enqueued

Then the handler receives amount=10 and source=id_42

**Test:** `test_event_loop.cpp` — `handler_receives_event_data`

### Scenario: Event Loop is a Controller

Given an Event Loop

When registered with the Runtime as a Controller

Then it is called during the Controller Execution Phase

**Test:** `test_event_loop.cpp` — `event_loop_is_controller`

### Scenario: Events can be enqueued from Controllers

Given a Controller that enqueues an event during its execution

When the Runtime ticks and the Controller executes

Then the event is processed by the Event Loop in the same tick (if before the Event Loop) or the next tick

**Test:** `test_event_loop.cpp` — `controller_enqueues_event`

---

## Constraints

* Event Loop must be a Controller (callable type receiving StateView + CommandProducer).
  * **Test verification:** `test_event_loop.cpp` — `event_loop_is_controller`
* Events must be processed in FIFO order within a single tick.
  * **Test verification:** `test_event_loop.cpp` — `fifo_event_processing`
* Handlers must not directly mutate state — they produce Commands.
  * **Test verification:** `test_event_loop.cpp` — `handlers_produce_commands`
* Unhandled events must be silently consumed (not an error).
  * **Test verification:** `test_event_loop.cpp` — `event_with_no_handlers`
* Event Loop must not hold state between ticks (events are transient).
  * **Test verification:** `test_event_loop.cpp` — `no_state_between_ticks`

---

## Out of Scope

* Event prioritization (all events are equal, processed FIFO).
* Event filtering or routing beyond type-based dispatch.
* Event bubbling or propagation.
* Asynchronous or deferred event processing.
* Event cancellation after enqueue.

---

## Open Questions

* [x] Should event types be string-based or integer/enum based?

**Answer:** Integer/enum based. Event types are identified by integer or enum values.

* [x] Should event data be a generic type-erased payload or typed template parameter?

**Answer:** Template parameter. Event data is a typed template parameter.

* [x] Should handler registration be part of the Runtime API or scoped to the Event Loop?

**Answer:** Both. Handler registration is available both through the Runtime API and scoped to the Event Loop.

* [x] Should events be enqueulable during Command Processing Phase or only during Controller Execution Phase?

**Answer:** Yes. Events may be enqueued both during Command Processing Phase and Controller Execution Phase.

* [x] Should unhandled events be reported through the Runtime diagnostics API?

**Answer:** Yes. Unhandled events are reported through the Runtime diagnostics API.

---

## Definitions

### Event Loop

A Controller that maintains an event queue and dispatches events to registered handlers during execution.

### Event

A unit of information representing something that occurred. Consists of a type identifier and optional data payload.

### Handler

A callable registered for a specific event type. Receives event data and a CommandProducer, and may produce Commands.

### Event Queue

A FIFO queue of pending events to be processed by the Event Loop.

### Event Type

An identifier that determines which handlers receive the event.
