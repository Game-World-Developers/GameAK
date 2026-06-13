# SPEC-007: Runtime Commands

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-007 defines Runtime Commands — requests to modify simulation state. All 14 open questions have been answered with thorough detail:

| Question | Answer |
|---|---|
| Typed? | Yes |
| Arbitrary user data? | No, payload defined by type |
| Multi-block targets? | Yes, all validated atomically |
| Can create blocks? | Yes (Runtime allocates) |
| Can destroy blocks? | Yes (Runtime manages lifecycle) |
| Can be rejected? | Yes, with explicit failure info |
| Expire? | No |
| Can be cancelled? | Yes, before execution |
| Priorities? | No (scheduler determines ordering) |
| Timestamps? | No |
| Identifiers? | Transient, for diagnostics only |
| Replayable? | Yes |

Core requirements (immutable, state modification request, runtime-executed, identity-based, deterministic, atomic) are all clearly defined.

## Open Questions

None. All 14 open questions have been answered.

## Notes

- Command identifiers are described as "transient Runtime-generated identifiers" for diagnostics — a developer would need to decide the identifier format (e.g., incrementing counter, UUID). Since identifiers "must not participate in simulation behavior," the specific format is an implementation detail.
- The replay requirement ("Replay must preserve deterministic behavior") places constraints on the implementation but does not require additional spec detail — determinism is addressed in SPEC-005 and SPEC-011.
- The `Status: DRAFT` header should be updated to `READY` once this verdict is accepted.
