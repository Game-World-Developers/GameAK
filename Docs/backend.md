# Backend Module

The Backend provides a runtime-dispatch VTable for low-level memory and bitset operations, enabling transparent SIMD acceleration without recompilation.

## Architecture

```
User Code          Inline Wrappers            VTable            Backend Implementations
   │                    │                       │                       │
   ├─ Backend::mem_copy ────► g_vtable──┐──► Detail::mem_copy (memcpy)
   ├─ Backend::mem_set  ────► g_vtable──┤──► Detail::mem_set  (memset)
   ├─ Backend::bitset_and ───► g_vtable──┤──► Detail::bitset_and (loop)
   └─ ...                ────► g_vtable──┘──► ...             (loop)
```

## VTable (`<AK/Backend/Backend.hpp>`)

```cpp
struct VTable {
  Type    type;         // Scalar or SIMD
  usize   simd_width;   // 0 for scalar, 16/32/64 for SIMD
  const char* name;     // "scalar", "sse2", "avx2", "neon"

  // Memory operations
  void* (*mem_copy)(void* dst, const void* src, usize size);
  void* (*mem_set)(void* dst, int value, usize size);
  int   (*mem_cmp)(const void* a, const void* b, usize size);

  // Bitset operations
  void  (*bitset_and)(u64* dst, const u64* a, const u64* b, usize word_count);
  void  (*bitset_or)(u64* dst, const u64* a, const u64* b, usize word_count);
  void  (*bitset_xor)(u64* dst, const u64* a, const u64* b, usize word_count);
  void  (*bitset_not)(u64* dst, const u64* a, usize word_count);
  usize (*bitset_popcount_range)(const u64* data, usize word_count);
};
```

## Initialization

```cpp
#include <AK/Backend/Backend.hpp>

int main() {
  GameAK::Backend::init();
  // g_vtable is now set to the scalar backend
}
```

Call `Backend::init()` once at program startup, before any memory or bitset operations. Currently it selects the scalar backend. In the future, it will perform runtime CPU feature detection to select the optimal SIMD backend.

## Inline Wrappers

All VTable function pointers have matching inline wrappers in the `GameAK::Backend` namespace. These are one-line forwarding calls through `g_vtable`:

```cpp
inline void* mem_copy(void* dst, const void* src, usize size) {
  return g_vtable->mem_copy(dst, src, size);
}
// ... same pattern for all functions
```

Using these wrappers ensures callers always use the active backend, even if the VTable is swapped at runtime.

## Scalar Backend (`<AK/Backend/ScalarBackend.hpp>`)

The default backend implementation, in the `GameAK::Backend::Detail` namespace:

| Function | Implementation |
|----------|---------------|
| `mem_copy` | `std::memcpy` |
| `mem_set` | `std::memset` |
| `mem_cmp` | `std::memcmp` |
| `bitset_and` | Loop: `dst[i] = a[i] & b[i]` |
| `bitset_or` | Loop: `dst[i] = a[i] \| b[i]` |
| `bitset_xor` | Loop: `dst[i] = a[i] ^ b[i]` |
| `bitset_not` | Loop: `dst[i] = ~a[i]` |
| `bitset_popcount_range` | Loop: `sum += Bits::popcount(data[i])` |

The `kScalarVTable` constant is defined as:

```cpp
inline constexpr VTable kScalarVTable = {
  .type        = Type::Scalar,
  .simd_width  = 0,
  .name        = "scalar",
  .mem_copy    = &Detail::mem_copy,
  // ... all function pointers
};
```

## SIMD Backend (Stub)

A stub file (`Src/AK/Backend/SIMDBackend.cpp`) exists for future SIMD implementation. When SSE/AVX/NEON intrinsics are added, a second VTable (e.g., `kSIMDVTable`) will be defined and selected by `Backend::init()` based on runtime CPU feature detection.

## Current Consumers

- `BitArray::reset()` uses `Backend::mem_set`
- `BitArray::and_with()`, `or_with()`, `xor_with()`, `negate()` use `Backend::bitset_*`
- `BitArray::popcount()` uses `Backend::bitset_popcount_range`
