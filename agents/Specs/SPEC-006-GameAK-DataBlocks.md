
# SPEC-006: Data Blocks

Status: IMPLEMENTED

Last validated by Ralph: never

---

## Summary

This specification defines Data Blocks, the fundamental unit of simulation state within GameAK.

Data Blocks are the primary building blocks used to represent simulation information.

Controllers, Queries, Commands, and Runtime Identities operate on Data Blocks.

Data Blocks contain data only.

Data Blocks do not contain behavior.

---

## Purpose

Data Blocks provide a stable abstraction for simulation state.

The Runtime may store Data Blocks using different layouts and representations without changing their observable behavior.

Examples include:

* AoS
* SoA
* AoSoA
* Bit-oriented representations

Storage strategy remains independent from Data Block semantics.

---

## Core Requirements

### Data Only

A Data Block contains data.

A Data Block must not contain execution logic.

---

### Runtime Managed

Data Blocks are created, managed, moved, and destroyed by the Runtime.

Controllers may request lifecycle operations but do not own Data Blocks.

---

### Identity Based

Every Data Block possesses a Runtime Identity.

Runtime Identities remain stable regardless of memory location, layout, or representation.

---

### Representation Independent

A Data Block must remain independent from:

* Layout
* Representation
* Storage strategy

---

### Queryable

Data Blocks must be accessible through Runtime Query APIs.

---

## Constraints

* Data Blocks contain no behavior.
* Data Blocks are owned by the Runtime.
* Data Blocks possess Runtime Identities.
* Data Blocks remain independent from storage layout.
* Data Blocks remain independent from representation strategy.
* Data Blocks are the fundamental unit of simulation state.

---

## Out of Scope

* Layout implementations.
* Runtime Queries.
* Runtime Commands.
* Scheduler behavior.
* Controller behavior.
* Serialization.
* Persistence.

---

## Open Questions

* [ ] Is a Data Block typed?

**Answer:** Yes.

Every Data Block has a type.

The type defines the structure and semantics of the contained data.

Runtime behavior must not depend on memory layout or representation.

* [ ] Can a Data Block contain multiple fields?

**Answer:** Yes.

A Data Block may contain one or more fields.

Field organization is defined by the Data Block type.

Storage layout remains a Runtime concern.

* [ ] Can a Data Block contain other Data Blocks?

**Answer:** No.

Data Blocks must not directly contain other Data Blocks.

Relationships between Data Blocks are represented through Runtime Identities.

This preserves layout independence and allows Data Blocks to move freely in memory.

* [ ] Can a Data Block exist without a Runtime Identity?

**Answer:** No.

Every Runtime-managed Data Block must possess a Runtime Identity.

Runtime Identities are required for queries, commands, relationships, and lifecycle management.

* [ ] Are Data Blocks fixed-size or variable-size?

**Answer:** Both.

GameAK supports fixed-size and variable-size Data Blocks.

The Runtime is responsible for managing storage requirements regardless of Data Block size.

* [ ] Can Data Blocks change representation during execution?

**Answer:** Yes.

The Runtime may change the representation of a Data Block during execution.

Representation changes must preserve observable behavior.

* [ ] Can Data Blocks change layout during execution?

**Answer:** Yes.

The Runtime may migrate Data Blocks between layouts during execution.

Layout changes must remain transparent to Controllers, Queries, and Commands.

* [ ] How are Data Blocks created?

**Answer:** Data Blocks are created by the Runtime.

Controllers and Runtime consumers may request Data Block creation through Runtime APIs.

The Runtime is responsible for allocation, identity generation, and initialization.

* [ ] How are Data Blocks destroyed?

**Answer:** Data Blocks are destroyed by the Runtime.

Controllers and Runtime consumers may request destruction through Runtime APIs.

The Runtime is responsible for lifecycle management and resource cleanup.

* [ ] Can a Data Block be empty?

**Answer:** Yes.

A Data Block may contain no fields.

Empty Data Blocks may be used to represent tags, markers, capabilities, or state presence.

Empty Data Blocks remain valid Data Blocks and possess Runtime Identities

---

## Definitions

### Data Block

The smallest independently addressable unit of simulation state.

### Runtime Identity

A stable identifier associated with a Data Block.

### Representation

The encoding strategy used to store state.

### Layout

The physical organization of state in memory.
