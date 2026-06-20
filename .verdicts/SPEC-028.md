# Verdict: SPEC-028

**Spec:** GameAK Fluent Queries
**Validation timestamp:** 2026-06-20
**Verdict:** READY
**Layer:** Runtime

---

## Assessment

All 7 behaviors are clearly specified. Lazy `BlockQuery` pattern is well-defined with clear intermediate vs terminal method semantics.

## Open Questions

1. **Name-to-type resolution:** `.of_type("Player")` requires Runtime to resolve name → type_id. Decision: BlockQuery stores a `const Runtime*` (or a function to resolve names). Resolution happens at query execution, not at `.of_type()` call.

2. **Runtime::blocks() return type:** Currently returns raw map. Will return `BlockQuery` instead (backward compatible via implicit conversion or separate method). Decision: Add `Runtime::query()` as the new entry point, keep `Runtime::blocks()` for the old API.

## Notes

* `BlockQuery` stores a const reference/pointer to the block map, not a copy.
* Terminal methods (`count`, `map`, `each`, `any`, `all`, `first`) are `const`.
* `BlockQuery` is a regular value type (copyable).
* Depends on SPEC-027 for name-based type resolution.

## Next Steps

Implement `BlockQuery` class and `Runtime::query()` method.
