# Verdict: SPEC-027

**Spec:** GameAK Block Type DSL
**Validation timestamp:** 2026-06-20
**Verdict:** READY
**Layer:** Runtime

---

## Assessment

All 6 behaviors are clearly specified with testable acceptance criteria. The builder pattern is well-defined.

## Open Questions

1. **type_id auto-assignment:** The spec mentions an "incrementing counter" but doesn't specify where it lives (Runtime vs builder). Decision: Runtime will hold a `next_block_type_id_` counter, auto-assigned in `done()`. The builder also needs `.id(n)` for explicit assignment.

2. **Name-based lookup:** `of_type("Player")` from SPEC-028 requires `Runtime` to have a `name → type_id` map. Decision: Add `std::unordered_map<std::string, uint32_t> type_name_to_id_` to Runtime, populated during `done()`.

3. **Member pointer template:** The `has(name, &Struct::member)` pattern works via pointer-to-member but `offsetof` on non-standard-layout types is UB. Decision: Document that Struct must be standard-layout, or use compiler builtins.

## Notes

* The builder stores a `std::function<Result<void>(BlockTypeDescriptor)>` callback (set by `Runtime::define()`) to avoid storing a raw `Runtime*` pointer.
* `BlockTypeBuilder` is move-only (moved into `done()`).
* Auto-assigned `type_id` starts at 1 (0 = unassigned sentinel).
* Name map is populated when `done()` successfully registers the type.

## Next Steps

Implement `BlockTypeBuilder` class and `Runtime::define()` method.
