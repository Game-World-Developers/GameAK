# Backend Module

## Summary

Runtime-dispatch layer for low-level memory and bitset operations using a
per-system `ExecutionProfile` model. No global state, no VTable indirection in
the hot path, thread-safe by design. Enables transparent SIMD acceleration
without recompilation or caller changes.

## Architecture

```
System (owns ExecutionProfile)
  │
  ├─ p.mem.copy(...)      →  MemOps::copy     (function pointer)
  ├─ p.mem.set(...)       →  MemOps::set
  ├─ p.mem.cmp(...)       →  MemOps::cmp
  │
  ├─ p.bitset.bit_and(...) →  BitsetOps::bit_and
  ├─ p.bitset.bit_or(...)  →  BitsetOps::bit_or
  ├─ p.bitset.bit_xor(...) →  BitsetOps::bit_xor
  ├─ p.bitset.bit_not(...) →  BitsetOps::bit_not
  └─ p.bitset.popcount_range(...) → BitsetOps::popcount_range
```

Each system creates or receives an `ExecutionProfile` at the start of execution
and passes it explicitly to bulk operations. The profile lives on the stack —
L1-hot, no pointer chasing beyond the indirect call itself.

## ExecutionProfile (`<AK/Backend/ExecutionProfile.hpp>`)

### Guarantees

- `MemOps` contains three function pointers: `copy`, `set`, `cmp`
- `BitsetOps` contains five function pointers: `bit_and`, `bit_or`, `bit_xor`,
  `bit_not`, `popcount_range`
- `ExecutionProfile` aggregates `MemOps`, `BitsetOps`, and hardware capability
  metadata (simd_width, cache_line_bytes, has_huge_pages) — fits in one cache line
- `make_scalar_profile()` always succeeds and fills all pointers with scalar implementations
- `make_simd_mem()` / `make_simd_bitset()` always succeed (currently delegate to scalar)
- `init()` detects CPU features at runtime and returns the best available profile
- Function pointer signatures match `std::memcpy`, `std::memset`, `std::memcmp`
  for memory operations
- Bitset operations operate on `u64` arrays — no byte-level ambiguity
- `simd_width` is 0 for scalar, 32 for AVX2, 64 for AVX-512
- `cache_line_bytes` is 64 on x86\_64
- `align_to_cache(n)` rounds up to `cache_line_bytes`

### Non-Guarantees

- `ExecutionProfile` is not a global — each caller manages its own profile
- `init()` performs CPU feature detection on x86\_64 via `__builtin_cpu_supports`
  (GCC/Clang); ARM64 detection is future work
- No validation that function pointers are non-null before use (by design —
  the factory functions always fill them)
- No thread-safety mechanism needed — profiles are value types, each thread owns
  its own copy
- `has_huge_pages` is always `false` currently (future: /proc/meminfo check)

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `init` | O(1) + CPUID |
| `make_scalar_profile` | O(1) |
| `mem_copy` | O(n) (delegated) |
| `mem_set` | O(n) (delegated) |
| `mem_cmp` | O(n) (delegated) |
| bitset operations | O(n) (delegated) |

### Threading

- Fully thread-safe: each thread creates or receives its own `ExecutionProfile`
- Concurrent reads from a shared `ExecutionProfile` are safe — it is read-only
  after construction
- Concurrent writes through the profile (e.g., `p.mem.copy(...)`) follow the
  thread-safety rules of the underlying function

### Valid Usage (Standalone)

```cpp
#include <AK/Backend/Backend.hpp>
#include <AK/Backend/ScalarBackend.hpp>

void process() {
    auto p = GameAK::Backend::make_scalar_profile();
    p.mem.copy(dst, src, 1024);
    p.bitset.bit_and(out, a, b, 4);
}
```

### Valid Usage (Runtime Detection)

```cpp
#include <AK/Backend/Backend.hpp>

int main() {
    auto p = GameAK::Backend::init();
    // p uses SIMD ops if AVX2 is available, scalar otherwise
    p.mem.set(buf, 0, 256);
}
```

### Valid Usage (ECS System — Future)

```cpp
struct MySystem : System {
    void execute(ExecContext &ctx) noexcept override {
        for (auto [e, layer] : ctx.world.query<BitLayer>()) {
            layer.clear_all(ctx.profile);
        }
    }
};
```

### Invariants

- All function pointers in a profile returned by `make_scalar_profile()` are non-null
- All function pointers in a profile returned by `init()` are non-null
- `bit_not(bit_not(x)) == x`
- `popcount_range` agrees with `Bits::popcount` per word

## Scalar Backend (`<AK/Backend/ScalarBackend.hpp>`)

Default backend implementation in `GameAK::Backend::Detail::Scalar`.
Memory operations delegate to CRT functions; bitset operations use simple loops.

### Guarantees

- `mem_copy` delegates to `std::memcpy` — same behavior and aliasing rules
- `mem_set` delegates to `std::memset`
- `mem_cmp` delegates to `std::memcmp`
- Bitset operations are deterministic — `bitset_popcount_range` uses
  `Bits::popcount` per word

### Non-Guarantees

- Bitset loops are scalar — not vectorized by the backend itself
  (compiler may auto-vectorize)
- No alignment assumptions for input arrays beyond natural `u64` alignment

## Migration from VTable (Legacy)

The previous design used a global `g_vtable` with indirect calls through a
`VTable` struct. The new design replaces it with:

1. **No global state** — each caller owns an `ExecutionProfile` on the stack
2. **Sub-tables by category** — `MemOps` and `BitsetOps` prevent cache line
   pollution when only one category is used
3. **Runtime detection** — `init()` uses `__builtin_cpu_supports` on x86\_64
   to select SIMD backend when available
4. **Thread safety** — no shared mutable state
5. **Explicit parameter** — `const ExecutionProfile &p` is passed to bulk
   operations on `BitArray` and `BitLayer`
