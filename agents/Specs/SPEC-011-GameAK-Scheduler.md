# SPEC-011: Scheduler

Status: READY

Last validated by Ralph: never

---

## Summary

This specification defines the Scheduler component of GameAK.

The Scheduler is responsible for validating, ordering, and executing Runtime Commands.

The Scheduler preserves deterministic execution semantics.

---

## Core Requirements

### Command Validation

The Scheduler must validate every Command before execution.

Validation must verify that referenced Runtime Identities exist.

Validation must verify that Command types match target Data Block types.

Invalid Commands must be rejected with explicit error information.

### FIFO Ordering

The default Scheduler executes Commands in First-In-First-Out order.

Command ordering must match submission order.

### Atomic Execution

Every Command must be applied atomically.

A Command either succeeds completely or fails completely.

Partial application is not permitted.

### Determinism

Given identical Commands in identical order, the resulting state must be identical.

Determinism must hold across platforms and compiler configurations.

### Rejection Handling

Rejected Commands must not modify simulation state.

Rejection must produce an explicit error containing the reason for rejection.

Rejected Commands are removed from the execution queue.

### Cancellation

Commands may be cancelled before execution.

Cancelled Commands must not modify simulation state.

Cancellation is managed through the Runtime API.

---

## Constraints

* Default scheduler is FIFO.
* Commands are validated before execution.
* Commands are applied atomically.
* Rejected Commands do not modify state.
* Cancelled Commands do not modify state.
* Scheduler behavior is deterministic.
* Commands do not expire.

---

## Out of Scope

* Priority-based scheduling.
* Preemptive scheduling.
* Distributed scheduling.
* Real-time scheduling guarantees.

---

## Open Questions

* [ ] When does validation occur?

**Answer:** Validation occurs during `tick()`, immediately before execution.

Commands are not validated at submission time.

This allows Commands to reference Data Blocks created later in the same tick.

* [ ] Can the Command queue be inspected?

**Answer:** Yes.

The pending Command queue may be inspected through a diagnostics API.

Inspection is intended for debugging and profiling purposes.

* [ ] Is the scheduler replaceable?

**Answer:** Yes.

The Runtime may accept a custom Scheduler implementation during creation.

Custom schedulers must preserve deterministic semantics.

The FIFO implementation is the default.

* [ ] How are rejected Commands reported?

**Answer:** Rejected Commands are reported in the TickResult returned by `tick()`.

Each rejection includes the Command identifier and the rejection reason.

---

## Definitions

### Scheduler

The Runtime component responsible for Command validation, ordering, and execution.

### FIFO

First-In-First-Out execution order.

### Tick

A single execution cycle that processes all pending Commands.

### Atomic Execution

Execution that either succeeds completely or fails completely.
