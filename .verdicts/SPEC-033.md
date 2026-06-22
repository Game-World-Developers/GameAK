# SPEC-033: Archetype Storage

**Validation timestamp:** 2026-06-21

**Verdict:** READY

## Assessment

SPEC-033 defines a dense Archetype Storage layout strategy that complements the existing AoS/SoA/AoSoA layouts. The spec is clear and internally consistent.

Key design decisions:
- Archetype is an additional LayoutStrategy, not a replacement — both per-block and archetype storage coexist
- Sparse set provides O(1) identity→(chunk,slot) mapping
- Compaction via swap-and-pop preserves density
- Conversion to/from AoS/SoA/AoSoA via the existing convert_layout command flow

All 7 behavior scenarios are testable. All 4 constraints are verifiable. All 3 open questions are answered.

## Open Questions

All resolved. No unanswered questions remain.

## Cross-Reference Verification

| Concept | Definition Location |
|---------|-------------------|
| **Data Block** | SPEC-006 |
| **LayoutStrategy** | SPEC-018 |
| **LayoutManager** | SPEC-018 |
| **Runtime Identity** | SPEC-008 |
| **convert_layout** | SPEC-007 |
| **Sparse Set** | This spec |

## Implementation Verification

Not yet implemented.

## Final Determination

**SPEC-033 is READY for implementation.**
