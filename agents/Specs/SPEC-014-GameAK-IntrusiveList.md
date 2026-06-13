# SPEC-014: Intrusive List

Status: DRAFT

Last validated by Ralph: never

---

## Summary

A doubly-linked intrusive list where elements embed the list node. No per-element allocation: nodes are fields in the user's type. Used for O(1) insertion/removal of tracked objects.

---

## Core Requirements

### Embedding

Types must inherit from `intrusive_node` or contain an `intrusive_node` member.

`intrusive_node` must provide `next` and `prev` pointers manipulated by the list.

### Circular Sentinel

The list must use a circular sentinel node. An empty list has the sentinel pointing to itself. This guarantees:
- No null pointer checks in traversal
- `begin()` and `end()` are O(1)

### API Surface

| Method | Description |
|--------|-------------|
| `push_front(T*)` | Insert at front |
| `push_back(T*)` | Insert at back |
| `pop_front()` | Remove front |
| `pop_back()` | Remove back |
| `insert(const_iterator, T*)` | Insert before position |
| `erase(const_iterator)` | Erase at position |
| `clear()` | Remove all |
| `size() const` | Element count |
| `empty() const` | `size() == 0` |
| `front()` | First element reference |
| `back()` | Last element reference |
| `begin()/end()` | Iteration |

### Iterator

The list must provide:
- `iterator` — mutable, bidirectional
- `const_iterator` — immutable, bidirectional
- `iterator` must implicitly convert to `const_iterator`

### No Ownership

The list does not own or delete nodes. The caller manages lifetime.

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

* [ ] Should `size()` be cached O(1) or computed O(N)?

**Answer:** Cached O(1). Size is updated on every insert/erase.

---

## Definitions

### Intrusive Container

A container where the data structure linkage is embedded in the elements themselves, rather than in separate node objects.
