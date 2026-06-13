# SPEC-005: Runtime Model

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-005 is the most important conceptual spec — it defines how the runtime executes. It defines all core concepts (Data Block, Controller, Transformation, Runtime Identity, Runtime Command, Scheduler) and establishes the canonical execution cycle:

> 1. Controllers read State → 2. Controllers produce Commands → 3. Commands submitted to Scheduler → 4. Scheduler validates Commands → 5. Scheduler applies accepted Commands → 6. State is updated

All 18 open questions have been answered with thorough detail, covering:
- State ownership (Runtime owns state)
- Data Block semantics (movable, identity-based)
- Controller behavior (create/destroy state, read-only access)
- Command semantics (immutable, atomic, rejectable, no dependencies)
- Identity requirements (globally unique, never reused)
- Execution model (sequential or parallel, must be deterministic)

## Minor Issues (non-blocking)

- **Line 252:** Typo — trailing `*` in "Can Controllers read State directly?*"
- **Line 236-242:** Duplicated paragraph — "Commands are immutable after creation. Neither Controllers nor the Scheduler may modify an existing Command. Any modification requires creation of a new Command." appears twice verbatim.

These are formatting issues that do not affect clarity or implementability.

## Open Questions

None. All 18 open questions have been answered.

## Notes

- This spec provides the conceptual model that SPEC-009, SPEC-010, SPEC-011, and SPEC-012 build upon.
- The execution cycle clearly implies controllers produce commands *before* the scheduler validates/applies them within a tick. However, the order of controller execution relative to command processing from *previous* ticks is not addressed here (that is a SPEC-009 concern).
- The `Status: DRAFT` header should be updated to `READY` once this verdict is accepted.
