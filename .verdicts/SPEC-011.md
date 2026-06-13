# SPEC-011: Scheduler

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-011 defines the Scheduler component with clear requirements:

| Requirement | Specification |
|---|---|
| Validation | Before execution, verify identities exist and types match |
| Ordering | FIFO by submission order |
| Atomicity | Commands succeed or fail completely |
| Determinism | Same commands + same order = same state (cross-platform) |
| Rejection | Explicit error with reason; removed from queue |
| Cancellation | Managed through Runtime API; before execution only |

All four open questions have been answered with concrete detail:
- Validation occurs during `tick()`, *not* at submission time
- Command queue is diagnosable (inspection API)
- Scheduler is replaceable (custom implementation allowed)
- Rejections reported in TickResult with identifier + reason

## Minor Considerations

- **Diagnostics API reference:** The spec says "The pending Command queue may be inspected through a diagnostics API" but this API is not defined in any spec. However, this is optional ("may"), not required. A developer can implement the core scheduler without it and add inspection later. This does NOT block implementation.
- **Cancellation reporting:** The spec says cancelled commands are managed "through the Runtime API" but doesn't define the API for cancellation. SPEC-009 doesn't define it either. However, cancellation semantics are clear (no state modification, before execution only).

## Open Questions

None. All four open questions have been answered.

## Notes

- The scheduler is replaceable — the FIFO implementation is the default. A custom scheduler API is implied but not specified; a developer would need to define the scheduler interface. The core behavior contract (determinism, validation, atomicity) is clear.
- SPEC-005's execution cycle and SPEC-011's validation timing are consistent: validation happens during tick, immediately before execution.
- The `Status: DRAFT` header should be updated to `READY` once this verdict is accepted.
