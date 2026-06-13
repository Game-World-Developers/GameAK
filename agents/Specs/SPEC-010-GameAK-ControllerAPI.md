# SPEC-010: Controller API

Status: READY

Last validated by Ralph: never

---

## Summary

This specification defines the Controller interface.

A Controller is a runtime component that transforms simulation state by producing Runtime Commands.

Controllers contain behavior. Controllers do not own state.

---

## Core Requirements

### Callable Interface

A Controller must be a callable type.

The callable must accept a read-only state view and a command producer.

The callable must return a Result type.

### State Access

Controllers receive a read-only view of the current simulation state.

Controllers must not mutate state directly.

Controllers must access Data Blocks through Runtime Identities.

### Command Production

Controllers produce Runtime Commands through a command producer API.

Commands produced by a Controller are submitted to the Runtime for execution.

Controllers must not execute Commands directly.

### Lifecycle

Controllers are registered with the Runtime before execution.

The Runtime owns Controller execution during tick.

Controllers may be executed multiple times.

Controller execution order is determined by the Runtime.

### No Internal Mutable State

Controllers must not rely on internal mutable state between executions.

State required by a Controller must be stored as simulation state.

---

## Constraints

* Controllers are callables.
* Controllers receive read-only state views.
* Controllers produce Commands through a producer.
* Controllers do not mutate state directly.
* Controllers are registered before execution.
* Controllers do not hold mutable state across ticks.
* Controller execution order is determined by the Runtime.

---

## Out of Scope

* Controller state persistence.
* Controller networking.
* Controller hot-reloading.
* Controller scheduling policies.

---

## Open Questions

* [ ] Can a Controller fail?

**Answer:** Yes.

Controller failures are reported through the Result return type.

A failed Controller must not partially modify state.

* [ ] Can a Controller produce multiple Commands in a single execution?

**Answer:** Yes.

A Controller may produce zero or more Commands during a single execution.

* [ ] Can a Controller read state from other Controllers?

**Answer:** Controllers do not read state from other Controllers.

Controllers read state from the Runtime through the state view.

* [ ] Can a Controller access the time delta?

**Answer:** Yes.

The state view may include a time delta value.

Time delta is provided by the Runtime during tick execution.

---

## Definitions

### Controller

A callable component that transforms simulation state through Commands.

### State View

A read-only projection of simulation state provided to a Controller.

### Command Producer

An interface through which Controllers submit Commands to the Runtime.
