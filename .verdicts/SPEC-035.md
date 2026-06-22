# SPEC-035: Bulk Iteration API

**Validation timestamp:** 2026-06-21

**Verdict:** READY

## Assessment

SPEC-035 defines a bulk iteration API that returns dense contiguous spans of field data. The spec correctly constrains the API to read-only access and keeps it layout-agnostic.

Key design decisions:
- `field_span<T>(type_id)` is available on StateView
- Works across all layout strategies (AoS, SoA, AoSoA, Archetype)
- AoS returns data by iterating blocks (non-contiguous but works)
- Read-only by design — spans are `const`
- No per-call allocation
- Supports both field name and member pointer lookup (mirrors existing field<T> API)

All 6 behavior scenarios are testable. All 4 constraints are verifiable. All 3 open questions are answered.

## Open Questions

All resolved. No unanswered questions remain.

## Cross-Reference Verification

| Concept | Definition Location |
|---------|-------------------|
| **StateView** | SPEC-009 |
| **LayoutStrategy** | SPEC-018 |
| **Archetype Storage** | SPEC-033 |
| **field<T>** | SPEC-009 |

## Implementation Verification

Not yet implemented.

## Final Determination

**SPEC-035 is READY for implementation.**
