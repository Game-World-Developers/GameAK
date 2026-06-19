# SPEC-001: Runtime State

Status: READY

Last validated by Ralph: 2026-06-13

---

## Summary

State is the primary unit of information within GameAK.

All runtime operations consume, transform, query, or produce state.

State is the source of truth for simulation.

---

## Core Requirements

### State Must Exist Independently

State must not depend on:

* Rendering
* Audio
* Networking
* Tooling

---

### State Must Be Transformable

A runtime component must be able to read state and produce a modified state.

---

### State Must Be Queryable

Runtime systems must be able to inspect state.

---

### State Must Be Representable

State may be stored using different layouts and representations.

Examples:

* AoS
* SoA
* AoSoA
* Bitmaps
* Bit Vectors

---

### State Must Be Composable

Multiple pieces of state may be grouped together to describe a simulation.

---

## Constraints

* State is the source of truth.
* State contains no execution logic.
* State is independent from storage layout.
* State is independent from bit representation.
* State is independent from controllers.

---

## Out of Scope

* Data layouts.
* Controllers.
* Scheduling.
* FSMs.
* Event loops.
* Bit manipulation.

---

## Open Questions

* [ ] What is the smallest valid state unit?

**Answer:** The smallest valid state unit is a Data Block.

A Data Block represents an independently addressable portion of simulation state.

A Data Block may be stored using different layouts and representations depending on runtime requirements.

* [ ] Is state mutable or immutable?

**Answer:** State is mutable.

Controllers operate by mutating simulation state.

Immutable views, snapshots, and historical states may be provided as optional runtime features.

* [ ] Can state contain references?

**Answer:** State should not contain direct memory references.

Relationships between state units should be represented through runtime identities.

This allows layouts and representations to change without invalidating references.

* [ ] How are state identities defined?

**Answer:** Every State Unit must possess a runtime identity.

A runtime identity is stable during the lifetime of the state unit and remains independent from memory layout and representation.

Controllers, queries, and relationships operate through runtime identities rather than memory addresses.

---

## Definitions

### State

Information describing the simulation at a point in time.

### State Unit

The smallest independently addressable piece of state.
