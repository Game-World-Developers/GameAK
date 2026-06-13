# SPEC-000: GameAK Vision

Status: READY

Last validated by Ralph: never

---

## Summary

GameAK is a simulation-first runtime focused on transforming structured data into efficient execution models.

The runtime is built around three architectural pillars:

* Data Layout
* Bit Representation
* State Transformation

The purpose of GameAK is not to provide gameplay features, rendering, or editor tooling.

Its purpose is to provide a foundation for representing, organizing, transforming, and executing simulation state.

---

## Architectural Pillars

### Data Layout

GameAK must support multiple memory layouts.

Examples:

* AoS
* SoA
* AoSoA

Data layout is considered a runtime concern.

---

### Bit Representation

GameAK must provide primitives for representing state using bit-oriented structures.

Examples:

* Bitwise operations
* Bitmaps
* Bit vectors
* Bit flags
* Bit packing

Bit representations may be used as storage, query, filtering, synchronization, or execution mechanisms.

---

### State Transformation

GameAK operates by transforming state.

State transformations may be performed by:

* Finite State Machines
* Event Loops
* Schedulers
* Rule Systems
* Runtime Pipelines

---

## Core Concepts

### State

The source of truth.

### Layout

The physical organization of state.

### Representation

The encoding of state.

### Controller

A mechanism that transforms state.

### Runtime

The environment responsible for executing transformations.

---

## Constraints

* State is the primary concern.
* Layout is independent from behavior.
* Representation is independent from layout.
* Controllers operate on state.
* Rendering is external to the runtime.
* Networking is external to the runtime.
* Tooling is external to the runtime.

---

## Out of Scope

* Rendering.
* Audio.
* Editor tooling.
* Asset management.
* Networking protocols.
* Game-specific features.

---

## Open Questions

* [ ] What is the smallest runtime unit?

**Answer:** The Smallest runtime unite is a Data Block. A Data Block represents an independently addressable portion of simulation state.

Layouts, representations and controllers operate on Data Blocks.

* [ ] Does GameAK require ECS?

**Answer:** No.
ECS is considered an optional abstraction that may be implemented on top of the runtime.

* [ ] Are layouts runtime-selectable?

**Answer:** Yes. The runtime may support multiple layout strategies including AoS, SoA, AoSoA, and bit-oriented layouts.

* [ ] Are bit representations user-visible?

**Answer:** Partially. Bit represetantions are exposed through dedicated descriptive APIs.

* [ ] What execution model drives the runtime?

**Answer:** The runtime is driven by state transformations. Controllers transform simulation state over time through deterministic execution models.
