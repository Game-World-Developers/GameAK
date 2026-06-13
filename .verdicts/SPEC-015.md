# SPEC-015: AVL Tree

**Validation timestamp:** 2026-06-13

**Verdict:** READY

---

## Assessment

Previous issues resolved:

### Bug fixes
- `rebalance()` now correctly checks `n->balance == -2` and `n->balance == 2` (the current node, not its children)
- `rotate()` balance factor formulas corrected for all cases (LL, RR, LR, RL, including intermediate states)

### Spec clarifications
- Height defined: null = -1, leaf = 0
- Balance factor range clarified: {-1, 0, 1} is valid range; -2 and 2 appear transiently during insertion
- Rotation preconditions specified (balance factor conditions for each case)
- Erase marked as optional extension (implementation provides it but spec doesn't require it)
- `noexcept` on move operations explicitly allowed
- `check_balance_factors()` and `iterator::raw()` documented as additional public API
- Namespace: `gameak::core` (accepted)
- Compare must be default-constructible (stated)

Implementation passes all 40222 tests including LL/RR/LR/RL rotation, stress (10000 sequential + 10000 random), move semantics, key update, and erase.
