# SPEC-005: Runtime Model

Status: READY

Last validated by Ralph: never

---

## Summary

This specification defines the runtime execution model of GameAK.

The runtime is responsible for managing state, coordinating controllers, executing transformations, and preserving deterministic behavior.

The runtime does not define simulation rules.

The runtime defines how simulation rules are executed.

---

## Core Concepts

### Data Block

The smallest independently addressable unit of simulation state.

### Controller

A runtime component capable of transforming state.

### Transformation

An operation that consumes state and produces a modified state.

### Runtime Identity

A stable identifier associated with a Data Block.

### Runtime Command

A Runtime Command is a request to modify simulation state.

Commands are produced by Controllers and executed by the Runtime.

Commands do not directly modify State.

The Runtime validates and applies Commands according to the execution model.

### Scheduler

A Scheduler is responsible for coordinating Runtime Commands.

The Scheduler determines command ordering, validation, and execution timing.

The Scheduler is the authoritative mechanism responsible for preserving deterministic execution.

---

## Runtime Responsibilities

The runtime is responsible for:

* Managing state.
* Managing runtime identities.
* Coordinating controller execution.
* Executing transformations.
* Preserving deterministic behavior.

---

## Constraints

* State remains the source of truth.
* Controllers operate on state.
* Runtime identities remain stable.
* Transformations must be deterministic.
* Layout and representation remain independent concerns.

---

## Out of Scope

* Specific layout implementations.
* Specific bit representations.
* FSM implementations.
* Scheduler implementations.
* Event loop implementations.

---

## Runtime Execution Model

The Runtime executes the following cycle:

1. Controllers read State.
2. Controllers produce Commands.
3. Commands are submitted to the Scheduler.
4. The Scheduler validates Commands.
5. The Scheduler applies accepted Commands.
6. State is updated.

---

## Open Questions

* [ ] Who owns state?

**Answer:**

The Runtime owns State.

Users interact with State through Runtime APIs.

The Runtime is responsible for allocation, storage, movement, identity management, and lifetime management.

* [ ] What is a Data Block?

**Answer:**

A Data Block is the smallest independently addressable unit of simulation state.

A Data Block contains data but no execution logic.

Data Blocks may be represented using different layouts and representations.

* [ ] Can Data Blocks move in memory?

**Answer:** Yes.

Data Blocks may move in memory as a consequence of layout optimization, compaction, migration, or representation changes.

Runtime Identities must remain stable regardless of physical location.

* [ ] What is a Controller?

**Answer:** A Controller is a runtime component that transforms simulation state.

Controllers contain behavior.

Controllers do not own state.

* [ ] Can Controllers create state?

**Answer:** Yes.

Controllers may request creation of new Data Blocks through Runtime APIs.

The Runtime remains responsible for allocation and identity generation.

* [ ] Can Controllers destroy state?

**Answer:** Yes.

Controllers may request destruction of Data Blocks through Runtime APIs.

The Runtime remains responsible for lifetime management.

* [ ] What is a Transformation?

**Answer:** A Transformation is a deterministic operation that consumes simulation state and produces a modified simulation state.

* [ ] Can Transformations fail?

**Answer:** Yes.

Transformations may fail.

Failures must be reported through explicit runtime mechanisms.

Exceptions are not permitted in the Runtime Core.

* [ ] Who executes Controllers?

**Answer:** The Runtime executes Controllers.

Controllers must not execute themselves.

Execution order is determined by the Runtime.

* [ ] Is execution sequential or parallel?

**Answer:** Both.

The Runtime may execute Controllers sequentially or in parallel.

Execution strategies must preserve deterministic behavior.

* [ ] How are Runtime Identities generated?

**Answer:** Runtime Identities are generated exclusively by the Runtime.

Identity generation must be independent from memory location, layout, and representation.

* [ ] Can Runtime Identities be reused?

**Answer:** No.

Runtime Identities must remain globally unique during the lifetime of a Runtime instance.

Destroyed identities must not be reused.

* [ ] How is state queried?

**Answer:** State is queried through Runtime Query APIs.

Queries operate on Runtime Identities, Data Block types, layouts, representations, and user-defined filters.

Query semantics must remain independent from storage layout.

* [ ] Can Controllers directly mutate State?

**Answer:** No.

Controllers must not directly mutate State.

Controllers interact with State through Runtime Commands.

The Runtime is responsible for validating, scheduling, and applying state modifications.

This ensures deterministic execution, layout independence, representation independence, and runtime ownership of State.

* [ ] When are Commands applied?

**Answer**: Commands are applied by the Runtime Scheduler.

Controllers produce Commands.

The Scheduler determines when Commands are validated and applied.

The Scheduler is responsible for preserving deterministic execution semantics.

* [ ] Are Commands immutable?

**Answer:** Yes. Commands are immutable after creation.

Neither Controllers nor the Scheduler may modify an existing Command.

Any modification requires creation of a new Command.Commands are immutable after creation.

Neither Controllers nor the Scheduler may modify an existing Command.

Any modification requires creation of a new Command.

* [ ] Can Commands be rejected?

**Answer:** Yes. The Runtime Scheduler may reject Commands that violate runtime constraints.

Rejected Commands must produce explicit failure information through Runtime error mechanisms.

Rejected Commands must never partially modify State.

* [ ] Can Controllers read State directly?*

**Answer:** Yes.

Controllers may read State through Runtime Query APIs.

Read access must not depend on layout or representation.

* [ ] Are Commands applied atomically?

**Answer:** Commands are applied atomically.

A Command either succeeds completely or fails completely.

Partial application is not permitted.

* [ ] Can Commands target multiple Data Blocks?

**Answer:**

Yes.

A Command may target one or more Data Blocks.

The Runtime Scheduler is responsible for validating all referenced Runtime Identities before execution.

Commands affecting multiple Data Blocks must preserve deterministic behavior and atomic execution semantics.

If validation fails for any targeted Data Block, the entire Command must fail.

* [ ] Can Commands depend on other Commands?

**Answer:**

No.

Commands must not depend on other Commands.

Controllers express intent through Commands.

Command ordering and coordination are responsibilities of the Runtime Scheduler.

Dependencies emerge from Runtime State rather than explicit Command-to-Command relationships.

---

## Definitions

### Runtime

The environment responsible for coordinating state transformations.

### State Transformation

A deterministic modification applied to simulation state.
