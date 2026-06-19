# SPEC-007: Runtime Commands

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-13

---

## Summary

This specification defines Runtime Commands.

Runtime Commands represent requests to modify simulation state.

Commands are produced by Controllers and consumed by the Runtime.

Commands do not directly modify State.

The Runtime validates and executes Commands according to the Runtime Model.

---

## Purpose

Runtime Commands provide a deterministic mechanism for expressing state modifications.

Commands separate decision-making from state mutation.

Controllers decide what should happen.

The Runtime decides how and when it happens.

---

## Core Requirements

### Immutable

Commands are immutable after creation.

A Command cannot be modified after submission to the Runtime.

---

### State Modification Request

A Command represents an intent to modify simulation state.

Commands do not perform modifications themselves.

---

### Runtime Executed

Commands are executed exclusively by the Runtime.

Controllers cannot execute Commands directly.

---

### Identity Based

Commands reference Data Blocks through Runtime Identities.

Commands must not depend on memory addresses.

---

### Deterministic

Commands must produce deterministic results when executed under identical Runtime conditions.

---

### Atomic

Commands are applied atomically.

A Command either succeeds completely or fails completely.

Partial application is not permitted.

---

## Constraints

* Commands are immutable.
* Commands do not directly modify State.
* Commands are executed by the Runtime.
* Commands operate through Runtime Identities.
* Commands preserve deterministic behavior.
* Commands are atomic.

---

## Out of Scope

* Scheduler implementation.
* Query implementation.
* Controller implementation.
* Networking.
* Serialization.
* Persistence.

---

## Open Questions

* [ ] Is every Command typed?

**Answer:** Yes.

Every Command has a type.

The Command type defines its intent, validation requirements, and execution semantics.

Command types remain independent from layout and representation.

* [ ] Can Commands contain arbitrary user data?

**Answer:** No.

Commands may contain only data required for execution.

Command payloads must be explicitly defined by the Command type.

Arbitrary user-defined payloads are not permitted.

* [ ] Can Commands target multiple Data Blocks?

**Answer:** Yes.

A Command may target one or more Data Blocks.

All referenced Runtime Identities must be validated before execution.

Commands affecting multiple Data Blocks must preserve deterministic and atomic execution semantics.

* [ ] Can Commands create Data Blocks?

**Answer:** Yes.

Commands may request creation of Data Blocks.

The Runtime remains responsible for allocation, initialization, and identity generation.

* [ ] Can Commands destroy Data Blocks?

**Answer:** Yes.

Commands may request destruction of Data Blocks.

The Runtime remains responsible for lifecycle management and cleanup.

* [ ] Can Commands be rejected?

**Answer:** Yes.

The Runtime may reject Commands that fail validation.

Rejected Commands must not modify State.

Rejection must produce explicit failure information.

* [ ] Can Commands expire?

**Answer:** No.

Commands do not expire.

Command lifetime is managed by the Runtime Scheduler.

A Command remains valid until it is executed, rejected, or cancelled.

* [ ] Can Commands be cancelled?

**Answer:** Yes.

Commands may be cancelled before execution.

Cancelled Commands must not modify State.

Cancellation is managed by the Runtime.

* [ ] Do Commands have priorities?

**Answer:** No.

Command priority is not part of the Runtime Command model.

Execution ordering is determined by the Scheduler.

Specialized Scheduler implementations may introduce prioritization mechanisms.

* [ ] Do Commands have timestamps?

**Answer:** No.

Commands do not possess timestamps.

Temporal behavior is the responsibility of the Runtime and Scheduler.

* [ ] How are Commands identified?

**Answer:** Commands are not Runtime Entities.

Commands may be assigned transient Runtime-generated identifiers for diagnostics, debugging, tracing, or profiling.

Command identifiers must not participate in simulation behavior.

* [ ] Can Commands be replayed?

**Answer:** Yes.

Commands may be replayed for debugging, testing, diagnostics, or simulation reproduction.

Replay must preserve deterministic behavior.

Replay mechanisms must not alter Command semantics.

---

## Definitions

### Runtime Command

A request to modify simulation state.

### Command Target

A Runtime Identity referenced by a Runtime Command.

### Atomic Execution

Execution that either succeeds completely or fails completely.
