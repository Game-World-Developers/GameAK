# SPEC-004: Coding and Design Standards

Status: READY

Last validated by Ralph: 2026-06-13

---

## Summary

This specification defines the coding and design standards used throughout GameAK.

The purpose of these standards is to maintain readability, consistency, portability, and long-term maintainability.

These standards are guidelines for engineering decisions and must be applied with consideration for performance, simplicity, and runtime requirements.

---

## Design Principles

### Simplicity First

Prefer the simplest solution that satisfies the requirements.

Avoid unnecessary abstractions.

Avoid speculative design.

---

### Single Responsibility

Functions, classes, and modules should have a single reason to change.

---

### Explicit Behavior

Runtime behavior should be observable and understandable.

Avoid hidden side effects.

---

### Composition Over Inheritance

Prefer composition whenever possible.

Inheritance should only be used when it clearly models the problem domain.

---

### Dependency Inversion

Depend on abstractions rather than implementations.

---

### Law of Demeter

Modules should communicate only with their direct collaborators.

---

## Coding Standards

### Naming

Requirements:

* Use descriptive names.
* Avoid abbreviations.
* Avoid encodings.
* Avoid magic numbers.
* Use searchable names.

---

### Functions

Requirements:

* Small and focused.
* One responsibility.
* Descriptive names.
* Minimal arguments.
* Avoid flag arguments.
* Minimize side effects.

---

### Comments

Requirements:

* Explain intent.
* Explain constraints.
* Explain non-obvious decisions.

Avoid:

* Redundant comments.
* Commented-out code.
* Comments that repeat implementation details.

---

### Source Organization

Requirements:

* Related code should remain close together.
* Concepts should be separated clearly.
* Variables should be declared near usage.
* Keep nesting levels shallow.

---

## Object Calisthenics

Object Calisthenics should be treated as design guidance.

Rules may be relaxed when:

* Data-oriented design requires simpler structures.
* Runtime performance would be negatively affected.
* Memory layout requirements justify exceptions.

The intent of the rule is more important than strict compliance.

---

## Data-Oriented Exceptions

GameAK is a simulation runtime.

The following structures are acceptable:

* Plain data structures.
* Value types.
* Bit-oriented structures.
* Layout-specific containers.

Performance-critical code may prioritize data locality over object-oriented purity.

---

## Constraints

* Readability is preferred over cleverness.
* Simplicity is preferred over abstraction.
* Generic implementations must remain understandable.
* Design standards must not conflict with runtime requirements.

---

## Testing Requirements

### Scope

This section applies to **behavioral specs** (those that define runtime behavior, such as SPEC-005 through SPEC-016).

This specification (SPEC-004) is a **meta-spec** governing development process. It defines principles and guidelines that are qualitative in nature. The testing requirements below apply to behavioral specs and their implementation, not to this document itself.

### Spec-Test Parity

Every behavior described in a behavioral spec must have an equivalent automated test.

This means:

* Each **Scenario** (Happy Path, Edge Case, etc.) in the spec must map to at least one test case.
* Each **Constraint** that expresses verifiable runtime behavior must be verifiable through tests.
* Each **Open Question** answer, once resolved, must be reflected in tests.

Subjective or qualitative constraints (e.g., naming conventions, readability guidelines, design principles) are exempt from automated test verification.

### Update Discipline

When a behavioral spec changes:

1. Update or add tests that cover the changed behavior **before** modifying implementation code.
2. Tests must fail **before** the implementation change and pass **after** it (red-green-refactor).
3. A behavioral spec must not be promoted to `IMPLEMENTED` status unless all its verifiable behaviors are covered by passing tests.

### Test Location

Tests must live under `Tests/` and follow the naming convention `test_<spec-area>.cpp`.

### Enforcement

The CI pipeline (or equivalent validation step) should reject a behavioral spec status change to `IMPLEMENTED` if corresponding tests are missing or failing.

---

## Out of Scope

* Formatting tools.
* Linter configuration.
* IDE settings.
* Code review procedures.

---

## Open Questions

* [ ] Should concepts be preferred over inheritance where possible?

**Answer:** Yes

Concepts should be preferred over inheritance whenever they can express the required behavior.

Inheritance should be reserved for true polymorphic relationships and stable abstractions.

Compile-time polymorphism is preferred over runtime polymorphism when it improves clarity, performance, or portability.

* [ ] What is the maximum acceptable function complexity?

**Answer:** Functions should remain small, focused, and understandable.

A function should perform a single responsibility.

When a function becomes difficult to understand without scrolling, it should be considered for decomposition.

Readability takes precedence over arbitrary line-count limits.

* [ ] Should exceptions be permitted in the runtime core?

**Answer:** No.

Exceptions are not permitted in the runtime core.

Runtime-core code must communicate failure through explicit mechanisms.

Examples include:

* Expected values
* Error objects
* Result types
* Status codes

Exceptions may be used by tooling, integrations, or external adapters when appropriate.

* [ ] Should Object Calisthenics rules be enumerated?

**Answer:** Yes.

The nine Object Calisthenics rules are:

1. Only One Level of Indentation Per Method.
2. Don't Use the ELSE Keyword.
3. Wrap All Primitives and Strings.
4. First-Class Collections.
5. One Dot Per Line.
6. Don't Abbreviate.
7. Keep All Entities Small.
8. No Classes With More Than Two Instance Variables.
9. No Getters/Setters/Properties.

These rules are design guidance, not strict enforcement. They may be relaxed when data-oriented design or runtime performance justifies exceptions.

* [ ] How is the Runtime Core boundary defined?

**Answer:** The Runtime Core is the portion of GameAK responsible for simulation state and execution. It corresponds to all code under `src/Runtime/`. Code outside this directory is not part of the runtime core and is not subject to its constraints (e.g., the no-exceptions rule).

* [ ] What constitutes a "Plain Data Structure" and "Value Type"?

**Answer:**

* **Plain Data Structure (PDS):** A class or struct with public data members and no invariants. It is directly constructable, copyable, and assignable. Examples: `Vec2`, `Color`, `BlockId`.
* **Value Type:** A type with value semantics: copyable, comparable, and independently usable. Value types may encapsulate behavior as long as they remain copyable and equality-comparable.
* **Bit-Oriented Structure:** A type whose representation maps directly to a specific bit layout for serialization or hardware interaction.
* **Layout-Specific Container:** A container whose memory layout is explicitly designed for a specific access pattern (e.g., SoA, AoSoA).

* [ ] What does Law of Demeter consider "direct collaborators"?

**Answer:** A module's direct collaborators are:

* Its own data members.
* Its function parameters.
* Objects it creates directly.
* Objects returned from its direct collaborators (one dot, no chaining).

Accessing transitive relationships (e.g., `a.b().c()`) violates the Law of Demeter unless the intermediate types are explicitly part of the public API contract.

* [ ] How should Simplicity First and Dependency Inversion be resolved when they conflict?

**Answer:** Simplicity First takes precedence when both approaches lead to the same observable behavior. Dependency Inversion should only be applied when there is a demonstrable need to swap implementations at compile time or runtime. Premature abstraction is discouraged.

As a tiebreaker:

1. If the dependency is stable and unlikely to change, prefer Simplicity First.
2. If the dependency has multiple valid implementations, prefer Dependency Inversion.
3. When in doubt, prefer Simplicity First.

* [ ] Many subjective constraints are unverifiable by tests — is this acceptable?

**Answer:** Yes. This is by design. Subjective constraints (naming conventions, readability, organization) are guidelines for human engineers, not automated checks. Only verifiable runtime behaviors require tests. This is explicitly stated in the Testing Requirements section under the exemption for subjective or qualitative constraints.

* [ ] How should the CI enforce spec status-to-test correspondence?

**Answer:** A validation script `.ci/check-spec-status.sh` must verify that for every behavioral spec promoted to `IMPLEMENTED`, corresponding test files exist under `Tests/` and compile successfully. The CI pipeline must run this script and reject the promotion if tests are missing or fail to compile. The mechanism for determining corresponding test files is: for each `agents/Specs/SPEC-NNN-*.md` with `Status: IMPLEMENTED`, there must be at least one file matching `Tests/test_<spec-area>.cpp` containing tests that cover the spec's behaviors.

---

## Definitions

### Simplicity

The minimum amount of code required to express a behavior correctly.

### Runtime Core

The portion of GameAK responsible for simulation state and execution.

### Data-Oriented Design

A design approach that prioritizes memory layout, cache locality, and efficient data access.
