# SPEC-010: Controller API

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-010 defines the Controller interface with sufficient clarity for implementation. All three open questions have been answered.

Key specifications:
- Controller is a callable type accepting a read-only state view and a command producer
- Returns a Result type
- Receives read-only state (no direct mutation)
- Produces commands through a command producer
- Registered with Runtime before execution
- Can produce zero or more commands per execution
- No mutable state between executions
- Can access time delta (provided by Runtime)
- Failures reported through Result type

## Minor Considerations

- **StateView and CommandProducer types are not defined.** The spec defines them conceptually ("a read-only projection of simulation state," "an interface through which Controllers submit Commands to the Runtime") but their exact API is not specified. A developer must define these interfaces. This is acceptable because:
  - StateView's purpose is clear (read-only access to Data Blocks via identities)
  - CommandProducer's purpose is clear (create/submit commands)
  - SPEC-005 and SPEC-006 define the underlying concepts
- **Controller's Result value type:** The Controller returns a Result, but the "success value" type is not specified (void? command count?). A developer must choose. This is a minor design decision.

## Open Questions

None. All three open questions have been answered.

## Notes

- This spec is tightly coupled with SPEC-009 (Runtime API, which registers controllers) and SPEC-011 (Scheduler, which determines execution order).
- The "no internal mutable state" constraint is important for determinism.
- The `Status: DRAFT` header should be updated to `READY` once this verdict is accepted.
