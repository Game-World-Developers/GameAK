# SPEC-036: Pooled Allocator for Data Blocks

**Validation timestamp:** 2026-06-21

**Verdict:** READY

## Assessment

SPEC-036 defines a pooled memory allocator for DataBlock storage. The spec correctly scopes the change as transparent to all existing APIs.

Key design decisions:
- Per-type pools — each block type gets its own pool
- Lazy growth in powers of 2 — no upfront reservation
- Destroyed blocks release slots for reuse — no compaction
- Variable-size blocks fall back to individual heap allocation
- Archetype Storage and SoA/AoSoA layouts already have their own storage — pools only apply to AoS per-block storage

All 6 behavior scenarios are testable. All 4 constraints are verifiable. All 3 open questions are answered.

## Open Questions

All resolved. No unanswered questions remain.

## Cross-Reference Verification

| Concept | Definition Location |
|---------|-------------------|
| **DataBlock** | SPEC-006 |
| **Archetype Storage** | SPEC-033 |
| **LayoutStrategy** | SPEC-018 |
| **Snapshot** | SPEC-009 |

## Implementation Verification (2026-06-21)

All 5 test scenarios pass:
- `pool_same_type_shares_pool` ✅
- `pool_growth` ✅
- `pool_slot_reuse` ✅
- `pool_per_type` ✅
- `pool_behavior_identical` ✅

Additionally, all 394 existing tests continue to pass with pooled storage enabled.

## Final Determination

**SPEC-036 is IMPLEMENTED.**
