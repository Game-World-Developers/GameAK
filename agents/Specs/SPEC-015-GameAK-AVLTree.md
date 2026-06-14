# SPEC-015: AVL Tree

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-13

---

## Summary

A self-balancing binary search tree where each node tracks a balance factor. After every insertion, the tree rebalances by rotating around nodes whose subtrees differ in height by more than 1.

Used internally for ordered lookups and range queries.

---

## Core Requirements

### Template Parameters

`avl_tree` must accept:
- `Key` — key type (must be comparable)
- `Value` — mapped value type
- `Compare` — comparison functor (default `std::less<Key>`, must be default-constructible)

### Balance Invariant

For every node, the height of the left and right subtrees must differ by at most 1. The balance factor is `height(right) - height(left)`. A balanced node has factor in `{-1, 0, 1}`; nodes may transiently reach `{-2, 2}` during insertion before rotation restores the invariant.

Height of a null subtree is -1. Height of a leaf node is 0.

### Rebalancing on Insert

After inserting a node, the tree must walk up toward the root, updating balance factors and performing rotations as needed:
- **Left-Left case**: node balance == -2 && left child balance == -1 → single right rotation
- **Right-Right case**: node balance == 2 && right child balance == 1 → single left rotation
- **Left-Right case**: node balance == -2 && left child balance == 1 → left rotation on child, then right rotation on node
- **Right-Left case**: node balance == 2 && right child balance == -1 → right rotation on child, then left rotation on node

### API Surface

| Method | Description |
|--------|-------------|
| `insert(Key, Value)` | Insert or update key-value pair. Returns iterator to node. |
| `find(const Key&)` | Look up key. Returns `end()` if not found. |
| `contains(const Key&)` | Boolean existence check |
| `size() const` | Element count |
| `empty() const` | `size() == 0` |
| `begin()/end()` | In-order iteration |

### Iteration

In-order traversal via bidirectional iterator. `begin()` returns the leftmost (smallest) node. Decrementing `begin()` and incrementing `end()` are undefined behavior.

### Key Update

If a key already exists, `insert` must update the value and return an iterator to the existing node (not rebalance).

### Move Semantics

`avl_tree` must be move-constructible and move-assignable. Moved-from state must be empty.

No copy semantics. Move operations are `noexcept`.

### Additional Public API

For testing and diagnostics, `check_balance_factors() const` returns `true` if every node's balance factor is in `{-1, 0, 1}`.

`iterator::raw()` returns the underlying `node*` (for debugging use).

### Erase

The implementation provides `erase(const Key&)` as an extension. It is not required by this specification and may be omitted by alternative implementations.

---

## Constraints

- Stable iteration order (in-order by key)
- No duplicate keys
- No exception guarantees
- Keys and values must be movable

---

## Out of Scope

- Thread safety
- Allocator customization
- `std::map` compatibility

---

## Open Questions

* [x] Should the balance factor be stored as `int` or `int8_t`?

**Answer:** `int`. Simplicity and alignment are preferred over micro-optimization for an internal structure.

* [x] What is the definition of height?

**Answer:** Height of a null subtree is -1. Height of a leaf is 0. Node height = max(left->height, right->height) + 1.

* [x] Is `{-1, 0, 1}` an invariant?

**Answer:** It is the valid range for a balanced node. Values -2 and 2 appear transiently during insertion and trigger rotation.

---

## Definitions

### Balance Factor

`height(right) - height(left)`. A value of -1, 0, or 1 indicates a balanced node.

### Rotation

A local tree operation that preserves in-order traversal order while changing the tree structure to improve balance.
