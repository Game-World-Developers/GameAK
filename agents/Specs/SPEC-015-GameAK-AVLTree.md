# SPEC-015: AVL Tree

Status: DRAFT

Last validated by Ralph: never

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
- `Compare` — comparison functor (default `std::less<Key>`)

### Balance Invariant

For every node, the height of the left and right subtrees must differ by at most 1. The balance factor is `height(right) - height(left)`, stored as `{-1, 0, 1}`.

### Rebalancing on Insert

After inserting a node, the tree must walk up toward the root, updating balance factors and performing rotations as needed:
- **Left-Left case**: single right rotation
- **Right-Right case**: single left rotation
- **Left-Right case**: left rotation then right rotation
- **Right-Left case**: right rotation then left rotation

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

In-order traversal via bidirectional iterator. `begin()` returns the leftmost (smallest) node.

### Key Update

If a key already exists, `insert` must update the value and return an iterator to the existing node (not rebalance).

### Move Semantics

`avl_tree` must be move-constructible and move-assignable. Moved-from state must be empty.

No copy semantics.

---

## Constraints

- Stable iteration order (in-order by key)
- No duplicate keys
- No exception guarantees
- Keys and values must be movable

---

## Out of Scope

- Erase (removal)
- Thread safety
- Allocator customization
- `std::map` compatibility

---

## Open Questions

* [ ] Should the balance factor be stored as `int` or `int8_t`?

**Answer:** `int`. Simplicity and alignment are preferred over micro-optimization for an internal structure.

---

## Definitions

### Balance Factor

`height(right) - height(left)`. A value of -1, 0, or 1 indicates a balanced node.

### Rotation

A local tree operation that preserves in-order traversal order while changing the tree structure to improve balance.
