# SPEC-018: Data Layout

**Validation timestamp:** 2026-06-16

**Verdict:** READY

## Assessment

SPEC-018 defines three memory layout strategies (AoS, SoA, AoSoA) for organizing Data Blocks. The spec is clear, internally consistent, and aligns with the vision established in SPEC-000.

Layout is correctly scoped as a type-level property of `BlockTypeDescriptor`, independent from representation and identity. The behavior scenarios cover all essential cases: default layout, type-level assignment, per-strategy field ordering, controller transparency, and layout conversion.

All 5 open questions have been resolved with the following key decisions:
- **Per-type registration** — layout is configured at type registration, not Runtime creation
- **Automatic conversion** — triggered based on access profile
- **Command-based** — layout conversion is requested via Command
- **Automatic alignment** — SoA padding based on `alignof`
- **Configurable chunk size** — AoSoA chunk size is configurable per type

## Open Questions

All resolved. No unanswered questions remain.

## Cross-Reference Verification

| Concept | Definition Location |
|---------|-------------------|
| **Data Block** | SPEC-006 |
| **BlockTypeDescriptor** | SPEC-009 |
| **Runtime API** | SPEC-009 |
| **Identity** | SPEC-008 |
| **Command** | SPEC-007 |
| **Controller** | SPEC-010 |

## Implementation Verification (2026-06-17)

All 6 test scenarios pass:
- `default_layout_is_aos` ✅
- `layout_is_type_property` ✅
- `aos_field_ordering` ✅
- `layout_transparent_to_controllers` ✅
- `layout_conversion_preserves_identity` ✅
- `multiple_layouts_coexist` ✅

## Final Determination

**SPEC-018 is IMPLEMENTED.** All behaviors are covered by passing tests (total: 6).
