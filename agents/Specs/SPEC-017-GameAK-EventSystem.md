# SPEC-017: Event System

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-13

---

## Summary

This specification defines the event system for the GameAK Runtime.

The event system allows external code and controllers to receive notifications when specific state transitions occur during a tick.

---

## Core Requirements

### Event Types

The Runtime must define the following event types:

```cpp
enum class EventType : uint32_t {
    TickBegin,       // Fired at the start of each tick (or sub-tick)
    TickEnd,         // Fired at the end of each tick (or sub-tick)
    BlockCreated,    // Fired after a block is created via command execution
    BlockDestroyed,  // Fired after a block is destroyed via command execution
};
```

### Event Data

Events carry a uniform data structure:

```cpp
struct Event {
    EventType type;
    core::Identity identity; // populated for BlockCreated / BlockDestroyed
    uint32_t block_type_id;  // populated for BlockCreated
};
```

### Subscribing and Unsubscribing

The Runtime must provide an API for registering and removing event handlers:

```cpp
using EventHandler = std::function<void(const Event&)>;
using EventId = uint64_t;

EventId listen(EventType type, EventHandler handler);
void unlisten(EventId id);
```

- `listen()` returns a unique `EventId` that can be used to unlisten.
- Multiple handlers may be registered for the same event type; all are called.
- Handlers are called synchronously during `tick()`.
- `unlisten()` removes a previously registered handler by its `EventId`. Calling `unlisten()` with an unknown `EventId` is a no-op.

**Tests:** `test_runtime.cpp` — `TickBegin and TickEnd fire during tick`, `BlockCreated fires when a block is created via command`, `BlockDestroyed fires when a block is destroyed via command`, `unlisten removes event handler`, `multiple handlers on same event type`

### Firing Semantics

- `TickBegin` is fired at the very start of `execute_single_tick()`, before Controller execution.
- `TickEnd` is fired at the very end of `execute_single_tick()`, after command processing and type count rebuild.
- `BlockCreated` is fired after a block is created via command execution during the Command Processing Phase. It carries the new block's identity and type_id.
- `BlockDestroyed` is fired after a block is destroyed via command execution during the Command Processing Phase. It carries the destroyed block's identity.

Block events are only fired if at least one handler is registered for that event type (to avoid the cost of diffing the block map unnecessarily).

**Tests:** `test_runtime.cpp` — `BlockCreated fires when a block is created via command`, `BlockDestroyed fires when a block is destroyed via command`

### Fixed Timestep Interaction

When using fixed timestep, events are fired for each sub-tick independently:
- Each sub-tick produces its own `TickBegin` and `TickEnd` events.
- Block events are fired within the sub-tick where the command that caused them was processed.

**Test:** `test_runtime.cpp` — `fixed timestep fires events for each sub-tick`

---

## Constraints

* Handlers are called synchronously during the tick in which the event occurs.
* Event handlers receive a const reference to the Event. They must not modify the Runtime.
* Event handlers are called in registration order for the same event type.
* Block events are detected by diffing the block map before and after command processing. This diff is only performed when at least one handler is registered for BlockCreated or BlockDestroyed.

---

## Out of Scope

* Asynchronous event delivery.
* Event filtering or priority.
* Event propagation (stopping an event from reaching other handlers).
* Events for state changes outside of tick execution (e.g., direct `create_block()` calls).

---

## Definitions

### Event Type

An enumeration value that identifies the kind of state transition that occurred.

### Event Handler

A callable that receives an `Event` and performs side effects (e.g., updating external state, logging, triggering audio).

### Event Id

A unique identifier for a registered event handler, used to unregister it later.
