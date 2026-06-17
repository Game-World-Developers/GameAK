# SPEC-020: Finite State Machines

**Validation timestamp:** 2026-06-16

**Verdict:** READY

## Assessment

SPEC-020 defines Finite State Machines as a Controller-based state transformation mechanism. The spec is well-structured, with clear scenarios covering lifecycle (initial state, transitions, entry/exit actions, determinism) and edge cases (undefined transitions).

The FSM is correctly scoped as a Controller, ensuring it integrates with the existing Runtime execution model. Constraints properly enforce determinism, explicit transitions only, single active state, and no mutable state between ticks.

All 5 open questions have been resolved:
- **Dedicated event type** — FSM events are a dedicated type, not generic Data Blocks
- **Both declarative and imperative** — FSM supports builder/DSL and add_state/add_transition APIs
- **Command submission** — events are submitted via Command
- **Timed transitions** — supported
- **Multiple Commands from actions** — entry/exit actions may produce multiple Commands

## Open Questions

All resolved. No unanswered questions remain.

## Cross-Reference Verification

| Concept | Definition Location |
|---------|-------------------|
| **Controller** | SPEC-010 |
| **Command** | SPEC-007 |
| **CommandProducer** | SPEC-010 |
| **StateView** | SPEC-010 |
| **Runtime** | SPEC-009 |
| **Result** | SPEC-012 |

## Implementation Verification (2026-06-17)

All 5 test scenarios pass:
- `fsm_starts_in_initial_state` ✅
- `fsm_entry_actions` (covers entry/exit/transition) ✅
- `fsm_ignores_undefined_transitions` ✅
- `fsm_is_controller` ✅
- `fsm_explicit_transitions_only` ✅

## Final Determination

**SPEC-020 is IMPLEMENTED.** All behaviors are covered by passing tests (total: 5).
