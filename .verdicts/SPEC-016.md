# SPEC-016: Red-Black Tree

**Validation timestamp:** 2026-06-13

**Verdict:** NOT READY

---

## Assessment

### Spec clarity and completeness

The spec is structured and readable. The core requirements, API surface table, and red-black invariants are clearly stated. However, several details are underspecified:

1. **Iterator value_type is not defined.** The spec says "bidirectional iterator" but does not state that `*it` yields `std::pair<const Key, Value>&`. A developer implementing from scratch would have to guess.

2. **No const_iterator.** The spec describes `begin()/end()` without any `const` qualification. The implementation only has a single non-const iterator returned from const methods (meaning you can modify values through a const tree). The spec is silent on this design choice.

3. **No return-type specification for `find()`.** The spec says "Returns `end()` if not found" but doesn't say whether the return is `iterator` or `const_iterator` or something else.

4. **Fixup table is high-level but correct.** The table describing the three fixup cases (uncle red / uncle black inside / uncle black outside) is accurate and usable. It does not prescribe exact pointer reassignment, which is appropriate.

### Contradictions or impossible requirements

None found. No requirement contradicts another, and all requirements are feasible.

### Implementation vs Spec match

**The implementation does NOT correctly implement the spec's fixup algorithm.** Two demonstrable bugs exist:

**Bug 1 — Rotation direction in the outside-child case (line 87).**
The code does `rotate(g, -pd)` but should do `rotate(g, pd)`.

- For the LL case (pd = -1): `rotate(g, 1)` is a left rotation, but a right rotation is needed.
- For the RR case (pd = 1): `rotate(g, -1)` is a right rotation, but a left rotation is needed.
- This was confirmed with a standalone test: the wrong rotation produces an incorrect tree structure (80 becomes the root instead of 30 in the LL case).

**Bug 2 — Node reassignment in the inside-child case (line 84).**
The code does `n = child_ptr(n->parent, d)` but should do `n = child_ptr(n, -d)` (or equivalently, set `n` to the old parent after rotation).

- After `rotate(p, d)` in the LR case: `n->parent = g`, so `child_ptr(n->parent, d) = child_ptr(g, 1)` = the uncle (or nullptr if uncle is absent).
- `child_ptr(n, -d)` gives the old parent `p`, which is the correct node for subsequent processing.
- When the uncle is nullptr, the current code sets `n = nullptr` and the subsequent `dir_of(n->parent)` on line 85 performs a null-pointer dereference (crash).

**These bugs are not caught by the existing test** because the test only inserts 4 keys (3, 1, 4, 2) in an order that triggers only the "uncle is red" recoloring case. The "uncle is black" cases (both outside and inside) are never exercised.

### Missing acceptance criteria

The test at `tests/test_runtime.cpp` (lines 493–516) covers only:

- Insert of 4 elements
- `empty()`, `size()`, `contains()`, `find()`
- Forward in-order iteration (checking ascending keys)

**Missing test coverage:**

| Scenario | Status |
|----------|--------|
| Uncle is red (simple recolor) | Tested (passes) |
| Uncle is black, outside child (LL or RR) | NOT tested |
| Uncle is black, inside child (LR or RL) | NOT tested |
| Move construction | NOT tested |
| Move assignment | NOT tested |
| Key update (insert existing key) | NOT tested |
| Decrement iterator (`operator--`) | NOT tested |
| Empty tree iteration (begin == end) | NOT tested |
| Single element tree | NOT tested |
| Deep trees requiring complex rotations | NOT tested |

---

## Open Questions

1. **What should `*it` yield?** The spec should clarify that the iterator's `value_type` is `std::pair<const Key, Value>`.

2. **Should there be a `const_iterator`?** The spec should say whether const iteration over a const tree is supported, or whether the current design (non-const iterator from const methods) is intentional.

3. **What is the exact return type of `find()`?** The spec should state it returns `iterator` (or `const_iterator`).

---

## Required actions before READY

1. **[BUG]** Fix `rotate(g, -pd)` → `rotate(g, pd)` on line 87 of `rb_tree.h`.
2. **[BUG]** Fix `n = child_ptr(n->parent, d)` → `n = child_ptr(n, -d)` on line 84 of `rb_tree.h`.
3. **[TEST]** Add tests covering the "uncle is black, outside child" case (both LL and RR).
4. **[TEST]** Add tests covering the "uncle is black, inside child" case (both LR and RL).
5. **[TEST]** Add tests for move construction and move assignment.
6. **[TEST]** Add tests for key update (insert existing key modifies value).
7. **[TEST]** Add tests for bidirectional iteration (decrement).
8. **[TEST]** Add edge cases: empty tree, single element, two elements.

---

## Notes

- The spec's fixup logic is correctly described at the conceptual level. The bugs are in the implementation's translation of that logic to pointer operations.
- Without the rotation direction and inside-case reassignment fixes, the tree will produce incorrect structure (and potentially crash) for non-trivial insertion sequences.
- The spec status should remain DRAFT until the implementation is corrected and re-validated.
