# SPEC-014: Intrusive List

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-13

---

## Summary

A doubly-linked intrusive list where elements embed the list node. No per-element allocation: nodes are fields in the user's type. Used for O(1) insertion/removal of tracked objects.

---

## Core Requirements

### Embedding

Types must inherit from `intrusive_node`. Nodes may be in at most one list at a time.

`intrusive_node` must provide `next` and `prev` pointers initialized to `nullptr`.

### Circular Sentinel

The list must use a circular sentinel node. An empty list has the sentinel pointing to itself. This guarantees:
- No null pointer checks in traversal
- `begin()` and `end()` are O(1)

### API Surface

| Method | Description |
|--------|-------------|
| `push_front(T*)` | Insert at front |
| `push_back(T*)` | Insert at back |
| `pop_front()` | Remove front (no-op if empty) |
| `pop_back()` | Remove back (no-op if empty) |
| `insert(const_iterator, T*)` | Insert before position. Returns `iterator` to inserted element. |
| `erase(const_iterator)` | Erase at position. Returns `iterator` to next element. |
| `clear()` | Remove all |
| `size() const` | Element count |
| `empty() const` | `size() == 0` |
| `front()` / `front() const` | First element reference |
| `back()` / `back() const` | Last element reference |
| `begin()/end()` | Iteration |

### Iterator

The list must provide:
- `iterator` — mutable, bidirectional
- `const_iterator` — immutable, bidirectional
- `iterator` must implicitly convert to `const_iterator`

### Copy and Move

- Copy is deleted
- Move is supported. After move, the source list is empty.

### Destructor

Destructor calls `clear()` to unlink all elements. The list does not own or delete nodes; `clear()` only updates pointers.

### No Ownership

The list does not own or delete nodes. The caller manages lifetime. Accessing `front()` or `back()` on an empty list is undefined behavior.

---

## Constraints

- Elements must not be in multiple lists simultaneously
- Removing an element not in the list is undefined behavior
- No bounds checking
- No exceptions

---

## Out of Scope

- Thread safety
- Allocator customization
- Element ownership
- `std::list` compatibility

---

## Open Questions

* [x] Should `size()` be cached O(1) or computed O(N)?

**Answer:** Cached O(1). Size is updated on every insert/erase.

* [x] What happens on `pop_front()`/`pop_back()` of empty list?

**Answer:** No-op.

---

## Definitions

### Intrusive Container

A container where the data structure linkage is embedded in the elements themselves, rather than in separate node objects.
