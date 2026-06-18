# SPEC-012: Error Model

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-012 defines the error model for the GameAK Runtime Core with sufficient detail for implementation.

Key specifications:
- **No exceptions** in Runtime Core (value-based error communication)
- **Result type:** Custom (not std::expected), [[nodiscard]], discriminated union (value | error)
- **Error type:** Contains error code + optional diagnostic message
- **Error codes:** Enumeration with 12 defined values (None, BlockNotFound, BlockTypeMismatch, InvalidIdentity, CommandRejected, CommandInvalid, TypeNotRegistered, AllocationFailed, InvalidOperation, InternalError, ControllerFailed, DuplicateRegistration, CapacityExceeded)
- Messages must not be used for control flow
- No error chaining

All three open questions have been answered.

## Why NOT BLOCKED by cancellation reporting

The spec does not include a `Cancelled` error code. This is correct — cancellation is not an error, it is an intentional operation. The handling of cancelled command reporting in TickResult is a concern of SPEC-009 (which has its own issues), not SPEC-012. SPEC-012 defines the error model; cancellation is a separate concern.

## Minor Considerations

- **Error code `None`:** Used as a "no error" sentinel. Its usage pattern is not defined (zero-initialization of Error type? Success state for Result?).
- **`InternalError`:** Very generic. By design — the spec says it represents unspecified internal failures.
- **Compile-time guarantees:** The spec requires 13 error codes but doesn't specify that this is an exhaustive enumeration (new codes may be added).
- **`LayoutMismatch`:** Added as a 14th error code during implementation. The spec has been updated accordingly.

## Open Questions

None. All three open questions have been answered.

## Notes

- This spec is foundational — all other specs that return results depend on it.
- The custom Result type (not std::expected) is a deliberate choice due to C++23 requirement.
- The `Status: DRAFT` header should be updated to `READY` once this verdict is accepted.
