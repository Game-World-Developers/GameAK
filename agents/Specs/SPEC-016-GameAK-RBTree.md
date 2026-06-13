# SPEC-016: Red-Black Tree

Status: DRAFT

Last validated by Ralph: never

---

## Summary

A self-balancing binary search tree using the red-black coloring invariant. After every insertion, the tree fixes violations by recoloring and rotating.

Used internally for ordered lookups where amortized rebalancing cost is preferred over AVL's stricter balance.

---

## Core Requirements

### Template Parameters

`rb_tree` must accept:
- `Key` — key type (must be comparable)
- `Value` — mapped value type
- `Compare` — comparison functor (default `std::less<Key>`)

### Red-Black Invariants

1. Every node is either red or black.
2. The root is black.
3. Red nodes have only black children (no consecutive red nodes on any path).
4. Every path from root to a null leaf contains the same number of black nodes.

### Fixup on Insert

After inserting a red node, the tree must walk up fixing violations:

| Case | Action |
|------|--------|
| Uncle is red | Recolor parent, uncle, grandparent; move up |
| Uncle is black, node is inside child | Rotate parent outward, then treat as outside |
| Uncle is black, node is outside child | Rotate grandparent, recolor parent and grandparent |

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

If a key already exists, `insert` must update the value and return an iterator to the existing node.

### Move Semantics

`rb_tree` must be move-constructible and move-assignable. Moved-from state must be empty.

No copy semantics.

---

## Constraints

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

* [ ] Should the tree track black-height for validation?

**Answer:** No. Black-height validation is a debug-only concern. The tree does not expose it.

---

## Definitions

### Black-Height

The number of black nodes on any path from a node to a leaf. Invariant #4 guarantees this is the same for all paths.

### Red-Black Fixup

The post-insertion procedure that restores red-black invariants through recoloring and rotations.
