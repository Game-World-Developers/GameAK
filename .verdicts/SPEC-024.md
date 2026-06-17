# SPEC-024: Ephemeral Data Blocks

**Validation timestamp:** 2026-06-17

**Verdict:** READY

## Assessment

SPEC-024 defines Ephemeral Data Blocks — a single-frame state lifecycle for intra-tick communication. The spec is clear, internally consistent, and implementable with minimal changes to the runtime.

### Strengths
- Solves the "intra-tick visibility" problem without new tick phases, channels, or concepts
- EphemeralProducer as a separate parameter keeps StateView read-only
- Automatic destruction at TickEnd is simple and predictable
- No event overhead for ephemeral blocks (high-frequency mechanism)
- All 4 open questions answered with clear rationale

### Design Decisions Confirmed
- Controller signature gains a third parameter (breaking change accepted)
- Ephemeral blocks live in the same `blocks_` map as persistent blocks (query APIs unchanged)
- Block event diff happens after controller phase (so ephemeral blocks don't trigger BlockCreated)
- Snapshots exclude ephemeral blocks
- SoA not supported initially

### Timing Clarification
The spec must clarify that the block event snapshot (`before_blocks`) is captured **after** Controller Execution Phase but **before** Command Processing Phase. This ensures ephemeral blocks created during controller execution are present in both `before_blocks` and `this->blocks_` at diff time, preventing spurious BlockCreated events.

## Open Questions

None. All questions answered.

## Notes

- This is a breaking change: `Controller` signature changes from 2 params to 3. All `Controller` callables (FSM, EventLoop, Pipeline, RuleSystem, tests) must be updated.
- Refactored FSM/EventLoop/Pipeline should use ephemeral blocks internally where appropriate.
- The block event diff timing adjustment is the only non-obvious implementation detail.

## Update Required

- [ ] SPEC-024: Add timing clarification about block event snapshot capture ordering
