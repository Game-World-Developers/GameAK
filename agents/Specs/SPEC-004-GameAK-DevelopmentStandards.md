# SPEC-004: Coding and Design Standards

Status: READY

Last validated by Ralph: never

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

---

## Definitions

### Simplicity

The minimum amount of code required to express a behavior correctly.

### Runtime Core

The portion of GameAK responsible for simulation state and execution.

### Data-Oriented Design

A design approach that prioritizes memory layout, cache locality, and efficient data access.
