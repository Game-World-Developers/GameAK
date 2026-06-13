# SPEC-013: Flat Vector

Status: READY

Last validated by Ralph: 2026-06-13

---

## Summary

A small-buffer-optimized vector that stores up to `InlineN` elements inline without heap allocation, spilling to heap only when capacity is exceeded.

Used internally by the Runtime for storage that benefits from locality and avoids per-element allocation.

---

## Core Requirements

### Template Parameters

`flat_vector` must accept:
- `T` — element type
- `InlineN` — number of inline elements (default 8)

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
| `emplace_back(Args&&...)` | Construct in-place at end |
| `pop_back()` | Remove last element |
| `clear()` | Remove all elements |
| `size() const` | Number of elements |
| `capacity() const` | Current capacity |
| `empty() const` | `size() == 0` |
| `operator[](size_t)` | Bounded access (no bounds check) |
| `at(size_t)` | Bounded access (no bounds check) |
| `data()` | Raw pointer to storage |
| `begin()/end()` | Forward iteration |
| `reserve(size_t)` | Reserve capacity |
| `resize(size_t)` | Resize, default-constructing new elements |
| `back()` | Last element |
| `front()` | First element |
| `erase(const_iterator)` | Erase at position |
| `erase(const_iterator, const_iterator)` | Erase range |

### Copy Semantics

`flat_vector` may be copy-constructible and copy-assignable. After copy, the new vector has the same elements.

### Move Semantics

`flat_vector` must be move-constructible and move-assignable. Moved-from state must be empty (`size() == 0`).

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

---

## Open Questions

* [x] Should `reserve()` be a no-op when requested capacity <= current capacity?

**Answer:** Yes. `reserve()` only grows, never shrinks.

* [x] Should `resize()` value-initialize or default-construct new elements?

**Answer:** Default-construct. Elements are constructed with `T{}`.

* [x] What is the default for `InlineN`?

**Answer:** 8. Matches the runtime's typical use case.

---

## Definitions

### Small Buffer Optimization (SBO)

The technique of storing a small number of elements in a fixed-size buffer within the object itself, avoiding heap allocation for small vectors.
