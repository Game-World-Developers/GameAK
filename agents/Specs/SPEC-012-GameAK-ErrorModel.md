# SPEC-012: Error Model

Status: IMPLEMENTED

Last validated by Ralph: never

---

## Summary

This specification defines the error model used throughout the GameAK Runtime Core.

Exceptions are not permitted in the Runtime Core. All failures must be communicated through explicit value-based mechanisms.

---

## Core Requirements

### No Exceptions

The Runtime Core must not throw or catch exceptions.

All error conditions must be represented as values.

### Result Type

All Runtime operations that can fail must return a Result type.

The Result type must contain either a value or an error.

The Result type must be [[nodiscard]].

### Error Type

All errors are represented by an Error type.

The Error type must contain an error code.

The Error type may contain an optional diagnostic message.

### Error Codes

Error codes are defined as an enumeration.

Each error code represents a distinct failure category.

New error codes may be added as the Runtime evolves.

### Error Handling

Consumers must check Result values before accessing contained data.

Unchecked Result values must produce a compiler warning.

---

## Error Codes

```
None                  No error
BlockNotFound         Data Block not found
BlockTypeMismatch     Operation type does not match block type
InvalidIdentity       Runtime Identity is invalid or zero
CommandRejected       Command was rejected by validation
CommandInvalid        Command is malformed or incomplete
TypeNotRegistered     Data Block type has not been registered
AllocationFailed      Memory allocation failed
InvalidOperation      Operation is not valid in current state
InternalError         Internal Runtime error
ControllerFailed      Controller execution failed
DuplicateRegistration Type or Controller already registered
CapacityExceeded      Runtime capacity limit reached
LayoutMismatch        Layout conversion failed
```

---

## Constraints

* No exceptions in Runtime Core.
* All fallible operations return Result.
* Result types are [[nodiscard]].
* Error codes are enumerations.
* Diagnostic messages are optional.
* Error codes must be human-readable.

---

## Out of Scope

* Stack traces.
* Exception handling adapters.
* Error recovery strategies.
* Transactional rollback beyond atomic Commands.

---

## Open Questions

* [ ] Should the Error type carry a diagnostic message string?

**Answer:** Yes, optionally.

The Error type may contain a string message for diagnostic purposes.

The message is not required for normal operation.

Messages must not be used for control flow.

* [ ] Should GameAK provide its own Result type or use std::expected?

**Answer:** GameAK provides its own Result type.

`std::expected` is C++23 and not guaranteed across supported compilers.

The custom Result type provides consistent API across all supported standards.

* [ ] Should Error support chaining or context propagation?

**Answer:** No.

Error chaining adds complexity without clear benefit for the initial implementation.

Context can be provided through diagnostic messages when needed.

---

## Definitions

### Result Type

A discriminated union containing either a value or an error.

### Error Code

An enumerated value identifying a failure category.

### Diagnostic Message

A human-readable string providing additional error context.
