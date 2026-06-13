# SPEC-015: AVL Tree

**Validation timestamp:** 2026-06-13

**Verdict:** NOT READY

---

## Assessment

The spec defines an AVL tree with template parameters, API surface, iteration, move semantics, and balance semantics. The implementation compiles and provides the correct API shape, but there is a **critical logic bug in the rebalancing algorithm** that violates the spec's own balance invariant, and the spec itself has **undefined terms** that make unambiguous implementation impossible.

---

## Spec Issues

### 1. "Height" is undefined (line 28)
The balance factor is defined as `height(right) - height(left)`, but the spec never defines what `height` means. Is leaf height 0 and null subtree height -1? Or are both 0? This changes the expected balance factor values.

### 2. Balance factor range is contradictory (line 28)
The spec says the balance factor is "stored as `{-1, 0, 1}`". But the implementation's own balance update loop (`p->balance += last_dir` at lines 193–196) produces values outside this range (e.g., +2, -2). If the spec means this is an *invariant* that must always hold, then no insertion could ever be detected as needing rebalancing. If it is a *description of a balanced node*, the spec needs to clarify.

### 3. Rotation preconditions are unspecified (lines 33–36)
The spec lists four rotation cases (LL, RR, LR, RL) with no balance-factor conditions that determine which case applies. For example:
- When is it Left-Left vs Left-Right?
- What balance values on the node and its child distinguish each case?

### 4. No acceptance criteria
The spec has no test cases or verification steps for the balance invariant, iteration correctness, or rotation coverage.

### 5. No namespace specified
The implementation uses `gameak::core`. The spec is silent on this.

### 6. Compare functor default-constructibility
`Compare` defaults to `std::less<Key>`, but the class has no constructor that accepts a `Compare` instance, implying `Compare` must be default-constructible. The spec does not state this requirement.

---

## Implementation Issues vs Spec

### CRITICAL: `rebalance()` checks the wrong node's balance (lines 58–81)

The `rebalance()` function reads:
```cpp
int left_h = n->left ? n->left->balance : 0;   // child's balance
int right_h = n->right ? n->right->balance : 0; // child's balance
if (left_h == 2) { ... }
else if (right_h == 2) { ... }
```

A correct AVL rebalancing routine must check the **current node's** balance factor for `-2` or `+2`. Instead, this code checks the **children's** balance field for the value `2`. This means:

- When a node's own balance becomes `+2` or `-2` (the actual trigger), the check evaluates the wrong values
- If balance factors are in `{-1, 0, 1}` per the spec, the check `== 2` would *never* trigger
- **Concrete trace**: Insert 5, 3, 1 sequentially. Node 5's balance becomes `-2`. `rebalance(3)` checks `3->left->balance` (node 1's balance = 0) and `3->right->balance` (null = 0). Neither equals 2. No rotation. Root (5) remains with balance `-2`. The invariant is violated.

### Balance update and rebalancing are misaligned

The spec says "walk up toward the root, updating balance factors and performing rotations as needed" (lines 32–36). The implementation splits this:
- Balance update: in `insert()`'s `for` loop (lines 193–196)
- Rotation: in `rebalance()` (lines 58–81)

But `rebalance()` never reads the balance values that were just updated — it reads children's balance instead.

### Extra API surface

`iterator::raw()` exists in the implementation but is not listed in the spec's API table.

### Move operations have `noexcept`

The spec says "No exception guarantees" (line 69), but the implementation marks move operations `noexcept`. This is a stronger contract than the spec commits to.

---

## Open Questions

| # | Question |
|---|----------|
| 1 | **What is the definition of `height`?** Leaf height 0 or 1? Null subtree height -1 or 0? |
| 2 | **What does the `balance` field actually store?** Is it a height difference (balance factor), or a different metric? The spec and implementation disagree. |
| 3 | **What balance-factor preconditions trigger each rotation case?** Specify: LL = bal == -2 && left->bal == -1; LR = bal == -2 && left->bal == +1; RR = bal == +2 && right->bal == +1; RL = bal == +2 && right->bal == -1. |
| 4 | **Should `rebalance()` check the current node's balance for `±2` (standard AVL) or the children's balance for `2` (current buggy code)?** |
| 5 | **What namespace should `avl_tree` be in?** Implementation uses `gameak::core`. |
| 6 | **Must `Compare` be default-constructible?** The class has no constructor accepting a `Compare` parameter. |
| 7 | **Should move operations be `noexcept`?** |
| 8 | **Is `iterator::raw()` intentionally part of the API?** |

---

## Required Fixes

### Spec fixes
1. Define `height` explicitly
2. Clarify the balance factor range: is `{-1, 0, 1}` an invariant or a description of balanced state?
3. Specify the balance-factor preconditions for each of the four rotation cases
4. Specify namespace (or accept `gameak::core`)
5. Add acceptance criteria / test cases

### Implementation fixes
1. Fix `rebalance()` to check `n->balance == 2` and `n->balance == -2` (the current node, not its children)
2. Fix the inner condition logic to correctly distinguish LL vs LR and RR vs RL based on the child's balance factor sign
3. Consider removing or documenting `iterator::raw()`

---

## Notes

- The RB tree implementation in `rb_tree.h` is clean and follows standard algorithms. The AVL tree should be held to the same standard of correctness.
- No existing tests were found for the AVL tree.
- The `rotate()` function and its balance-factor update formulas (lines 54–55) are standard and appear correct.
