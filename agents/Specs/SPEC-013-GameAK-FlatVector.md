# SPEC-013: Flat Vector

Status: DRAFT

Last validated by Ralph: never

---

## Summary

A small-buffer-optimized vector that stores up to `InlineN` elements inline without heap allocation, spilling to heap only when capacity is exceeded.

Used internally by the Runtime for storage that benefits from locality and avoids per-element allocation.

---

## Core Requirements

### Template Parameters

`flat_vector` must accept:
- `T` — element type
- `InlineN` — number of inline elements (default 4)

### Inline Storage

When `size() <= InlineN`, no heap allocation occurs. Elements are stored in an internal array.

### Heap Fallback

When `size() > InlineN`, storage moves to heap-allocated memory. Existing elements must be moved (not copied) to the heap.

### API Surface

`flat_vector` must provide:

| Method | Description |
|--------|-------------|
| `push_back(const T&)` | Append by copy |
| `push_back(T&&)` | Append by move |
| `pop_back()` | Remove last element |
| `clear()` | Remove all elements |
| `size() const` | Number of elements |
| `capacity() const` | Current capacity |
| `empty() const` | `size() == 0` |
| `operator[](size_t)` | Bounded access (no bounds check) |
| `data()` | Raw pointer to storage |
| `begin()/end()` | Forward iteration |
| `reserve(size_t)` | Reserve capacity |
| `resize(size_t)` | Resize, default-constructing new elements |
| `back()` | Last element |
| `front()` | First element |
| `insert(const_iterator, const T&)` | Insert at position |
| `erase(const_iterator)` | Erase at position |

### Move Semantics

`flat_vector` must be move-constructible and move-assignable. Moved-from state must be empty (`size() == 0`).

### No Copy

`flat_vector` must not be copy-constructible or copy-assignable (intentional for internal use).

---

## Constraints

- No exception guarantees (core does not use exceptions)
- No bounds checking on `operator[]`
- Elements must be movable
- `InlineN` must be > 0

---

## Out of Scope

- Allocator customization
- `std::vector`-compatible allocator interface
- Thread safety
- Emplace construction

---

## Open Questions

* [ ] Should `reserve()` be a no-op when requested capacity <= current capacity?

**Answer:** Yes. `reserve()` only grows, never shrinks.

* [ ] Should `resize()` value-initialize or default-construct new elements?

**Answer:** Default-construct. Elements are constructed with `T{}`.

---

## Definitions

### Small Buffer Optimization (SBO)

The technique of storing a small number of elements in a fixed-size buffer within the object itself, avoiding heap allocation for small vectors.
