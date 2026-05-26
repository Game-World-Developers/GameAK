# Backend Module

## Summary

Runtime-dispatch VTable for low-level memory and bitset operations.
Enables transparent SIMD acceleration without recompilation or caller changes.
Currently uses a scalar backend backed by `std::memcpy`/`memset`/`memcmp` and simple loops.

## Architecture

```
User Code          Inline Wrappers         VTable            Backend Implementations
   │                    │                     │                       │
   ├─ Backend::mem_copy ────► g_vtable──┐──► Detail::mem_copy (memcpy)
   ├─ Backend::mem_set  ────► g_vtable──┤──► Detail::mem_set  (memset)
   ├─ Backend::bitset_and ───► g_vtable──┤──► Detail::bitset_and (loop)
   └─ ...                ────► g_vtable──┘──► ...             (loop)
```

## VTable (`<AK/Backend/Backend.hpp>`)

### Guarantees

- `init()` sets `g_vtable` to the scalar backend — always succeeds
- inline wrappers are one-line forwarding calls through `g_vtable` — no branching beyond the function pointer call
- function pointer signatures match `std::memcpy`, `std::memset`, `std::memcmp` for memory operations
- bitset operations operate on `u64` arrays — no byte-level ambiguity

### Non-Guarantees

- VTable is a global variable — only one active backend at a time
- `init()` does not perform CPU feature detection (future enhancement)
- no validation that `g_vtable` is initialized before use
- no thread-safe VTable swap mechanism exists

### Failure Semantics

- calling any wrapper before `init()` dereferences a null `g_vtable` — undefined behavior (crash)
- `init()` has no failure path
- passing null pointers to memory operations produces the same behavior as `std::memcpy(nullptr, ...)` — undefined behavior

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `init` | O(1) |
| inline wrapper dispatch | O(1) (indirect call through VTable) |
| `mem_copy` | O(n) (delegated) |
| `mem_set` | O(n) (delegated) |
| `mem_cmp` | O(n) (delegated) |
| bitset operations | O(n) (delegated) |

### Threading

- `init()` must be called once before any concurrent access
- concurrent reads from `g_vtable` after initialization are safe — VTable is immutable after init
- concurrent writes through the VTable (e.g., `mem_copy`) follow the thread-safety rules of the underlying function

### Valid Usage

```cpp
#include <AK/Backend/Backend.hpp>

int main() {
    GameAK::Backend::init();
    // g_vtable is now set to the scalar backend
}

void process() {
    Backend::mem_copy(dst, src, 1024);
    Backend::bitset_and(dst, a, b, 4);
}
```

### Invalid Usage

```cpp
// Calling before init:
Backend::mem_copy(dst, src, 64); // undefined behavior: g_vtable is null
```

### Invariants

- `g_vtable` is non-null after `init()` returns
- VTable function pointers are never null in `kScalarVTable`
- `g_vtable->type` is `Type::Scalar` after `init()`

### Integration Notes

- consumed by `BitArray` for bulk operations (`reset`, `and_with`, `or_with`, `xor_with`, `negate`, `popcount`)
- SIMD backend can be added by defining a second VTable and selecting it in `init()` — no caller changes required
- `kScalarVTable` is `constexpr` — stored in read-only data

---

## Scalar Backend (`<AK/Backend/ScalarBackend.hpp>`)

### Summary

Default backend implementation in `GameAK::Backend::Detail`.
Memory operations delegate to CRT functions; bitset operations use simple loops.

### Guarantees

- `mem_copy` delegates to `std::memcpy` — same behavior and aliasing rules
- `mem_set` delegates to `std::memset`
- `mem_cmp` delegates to `std::memcmp`
- bitset operations are deterministic — `bitset_popcount_range` uses `Bits::popcount` per word

### Non-Guarantees

- bitset loops are scalar — not vectorized by the backend itself (compiler may auto-vectorize)
- no alignment assumptions for input arrays beyond natural `u64` alignment

### Valid Usage

```cpp
u64 dst[4] = {};
u64 src[4] = {1, 2, 3, 4};
Detail::bitset_or(dst, src, src, 4);
```

### Invariants

- `kScalarVTable` is `inline constexpr` — one definition across translation units
- `kScalarVTable.simd_width == 0`
