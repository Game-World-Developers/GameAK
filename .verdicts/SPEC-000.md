# SPEC-000: GameAK Vision

**Validation timestamp:** 2026-06-18

**Verdict:** READY

## Assessment

SPEC-000 is a vision document, not an implementation specification. It has been revised to reflect the two-layer nature of GameAK:

| Layer | Description |
|-------|------------|
| **AK (Abstraction Kit)** | General-purpose library of reusable abstractions (containers, value types, bit primitives, platform layer). No dependency on the Runtime. |
| **Runtime** | Deterministic simulation engine built on the AK. Manages Data Blocks, Controllers, Commands, and state transformations. |

The architectural pillars have been expanded from three to four (adding **Generic Abstractions** as the AK pillar alongside Data Layout, Bit Representation, and State Transformation). The spec clearly states that the AK must be usable independently and the Runtime depends on the AK but never the reverse.

## What Changed

- Summary redefined GameAK as "Abstraction Kit (AK) and simulation runtime" instead of "simulation-first runtime"
- Added **Layer Architecture** section with ASCII diagram showing AK → Runtime dependency
- Added **Generic Abstractions** as the fourth architectural pillar
- Added **AK (Abstraction Kit)** to Core Concepts
- Updated Constraints: AK independence guarantee, Runtime depends on AK
- Added two new Open Questions (AK vs Runtime distinction, standalone AK usage)
- Added `Layer:` metadata to header
- Fixed typo: "represetantions" → "representations", "unite" → "unit"

## Open Questions

All seven open questions in the spec have been answered. No unresolved questions remain.

## Notes

- This spec is a conceptual foundation. It does not define any APIs, data structures, or concrete behaviors — it exists to guide architectural decisions in downstream specs.
- No code should be written directly from this spec. It exists to validate that all other specs are consistent with the vision.
- All existing specs already fall into the correct layer (AK, Runtime, or Foundation) in practice. No spec renumbering or renaming is required at this time.
- The `Layer:` field in the header should be used in all specs going forward to clarify which layer each spec addresses.
