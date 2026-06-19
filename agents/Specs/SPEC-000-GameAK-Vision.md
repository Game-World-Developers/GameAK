# SPEC-000: GameAK Vision

Layer: Foundation

Status: READY

Last validated by Ralph: 2026-06-18

---

## Summary

GameAK is an **Abstraction Kit (AK) and simulation runtime** focused on transforming structured data into efficient execution models.

GameAK is composed of two layers:

* **AK (Abstraction Kit)** — a library of generic, reusable abstractions inspired by the AK of SerenityOS. Provides containers, value types, bit-level primitives, and platform abstractions. The AK has no dependency on the Runtime and can be used independently.
* **Runtime** — a deterministic simulation engine built on top of the AK. Manages simulation state (Data Blocks), coordinates transformations (Controllers), and preserves deterministic execution.

The project is built around four architectural pillars:

* Generic Abstractions
* Data Layout
* Bit Representation
* State Transformation

The purpose of GameAK is not to provide gameplay features, rendering, or editor tooling.

Its purpose is to provide a foundation for representing, organizing, transforming, and executing simulation state.

---

## Layer Architecture

```
GameAK
├── AK (Abstraction Kit)         ← usable standalone
│   ├── Generic Containers       │   flat_vector, IntrusiveList, AVLTree, RBTree
│   ├── Bit Primitives           │   BitSet, BitVector, BitFlags, BitPacking
│   ├── Value Types              │   Result<T>, Identity, Error, SemanticConstraint
│   └── Platform Layer           │   SIMD, compiler abstractions, capability detection
│
└── Runtime                      ← depends on AK
    ├── State Model              │   Data Blocks, Commands, Scheduler, Identities
    ├── Controllers              │   FSM, Rule Systems, Pipelines, Event Loops
    ├── Data Layout              │   AoS, SoA, AoSoA, layout conversion
    ├── Event System             │   lifecycle notifications (TickBegin, BlockCreated, etc.)
    └── Ephemeral State          │   single-tick Data Blocks for intra-tick communication
```

### AK Independence

The AK must be usable without the Runtime.

A consumer may use `flat_vector`, `Result<T>`, `Identity`, `BitSet`, or `AVLTree` without ever creating a Runtime instance.

### Runtime Depends on AK

The Runtime builds on AK abstractions.

`Identity`, `Result<T>`, `Error`, containers, and platform primitives are shared between both layers.

---

## Architectural Pillars

### Generic Abstractions

GameAK must provide reusable, generic building blocks applicable beyond simulation.

Examples:

* Containers (flat_vector, IntrusiveList, AVLTree, RBTree)
* Value types (Identity, Result, Error)
* Bit-level primitives (BitSet, BitVector, BitFlags, BitPacking)
* Platform abstractions (SIMD, compiler detection, capability detection)

These abstractions form the AK layer and must remain independent from the Runtime.

---

### Data Layout

The Runtime must support multiple memory layouts for simulation state.

Examples:

* AoS
* SoA
* AoSoA

Data layout is a Runtime concern.

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

Bit primitives belong to the AK layer. The Runtime may use them for simulation state.

---

### State Transformation

The Runtime operates by transforming state.

State transformations may be performed by:

* Finite State Machines
* Event Loops
* Schedulers
* Rule Systems
* Runtime Pipelines

---

## Core Concepts

### AK (Abstraction Kit)

A library of generic, reusable abstractions. No dependency on the Runtime.

### State

The source of truth within the Runtime.

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

* The AK must be usable without the Runtime.
* The Runtime depends on the AK, never the reverse.
* State is the primary concern of the Runtime.
* Layout is independent from behavior.
* Representation is independent from layout.
* Controllers operate on state.
* Rendering is external to the Runtime.
* Networking is external to the Runtime.
* Tooling is external to the Runtime.

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

**Answer:** The smallest runtime unit is a Data Block. A Data Block represents an independently addressable portion of simulation state.

Layouts, representations and controllers operate on Data Blocks.

* [ ] Does GameAK require ECS?

**Answer:** No.
ECS is considered an optional abstraction that may be implemented on top of the runtime.

* [ ] Are layouts runtime-selectable?

**Answer:** Yes. The runtime may support multiple layout strategies including AoS, SoA, AoSoA, and bit-oriented layouts.

* [ ] Are bit representations user-visible?

**Answer:** Partially. Bit representations are exposed through dedicated description APIs.

* [ ] What execution model drives the runtime?

**Answer:** The runtime is driven by state transformations. Controllers transform simulation state over time through deterministic execution models.

* [ ] What is the difference between AK and Runtime?

**Answer:** The AK is a general-purpose library of reusable abstractions (containers, value types, bit primitives) with no simulation concepts. The Runtime is a deterministic simulation engine that depends on the AK. The AK can be used standalone; the Runtime cannot.

* [ ] Can a consumer use only the AK?

**Answer:** Yes. The AK is designed to be used independently. A consumer may pull in `flat_vector`, `Result<T>`, `Identity`, `BitSet`, or any other AK component without linking against the Runtime.
