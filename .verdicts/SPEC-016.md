# SPEC-016: Red-Black Tree

**Validation timestamp:** 2026-06-13

**Verdict:** READY

---

## Assessment

Previous issues resolved:

### Bug fixes
- **Bug 1** (outside-child rotation direction): `rotate(g, -pd)` → `rotate(g, pd)` — fixed wrong rotation direction for LL/RR cases
- **Bug 2** (inside-child node reassignment): `n = child_ptr(n->parent, d)` → `n = child_ptr(n, -d)` — fixed nullptr dereference and wrong node tracking in LR/RL cases

### Test coverage added
- `check_invariants()` method verifies all 4 red-black invariants
- Invariant check is used in all rotation tests (LL, RR, LR, RL)
- Stress tests (10000 sequential + 10000 random) with invariant verification

### Spec clarifications
- Iterator `value_type` stated as `std::pair<const Key, Value>`
- `find()` return type clarified as `iterator`
- Erase marked as optional extension
- `check_invariants()` and `iterator::raw()` documented as additional public API

Implementation passes all 40222 tests including LL/RR/LR/RL rotation, stress, move semantics, key update, and erase.
