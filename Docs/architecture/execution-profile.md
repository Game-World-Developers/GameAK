# Execution Profile & Archetype-Aware Dispatch

## Overview

`ExecutionProfile` extends the original `DispatchTable` concept with hardware
capability metadata and serves as the single dispatch object passed through
the engine. `ExecContext` wraps the profile together with frame-global state,
reducing system parameter count and enabling future ECS integration.

## ExecutionProfile

Carries function pointer tables (MemOps, BitsetOps) plus:

| Field | Type | Meaning |
|-------|------|---------|
| `simd_width` | `u8` | 0=scalar, 16=SSE, 32=AVX2, 64=AVX-512 |
| `cache_line_bytes` | `u8` | 64 on x86, 128 on some ARM |
| `has_huge_pages` | `bool` | transparent hugepage support |

### Factory Functions

| Function | Returns | Notes |
|----------|---------|-------|
| `make_scalar_profile()` | full profile with all scalar impls | always available |
| `init()` | runtime-detected best profile | uses CPUID on x86\_64 |
| `make_simd_mem()` | MemOps | currently delegates to scalar |
| `make_simd_bitset()` | BitsetOps | currently delegates to scalar |

## ExecContext

```cpp
struct ExecContext {
    ExecutionProfile profile;
    u64              frame_index;
    f64              delta_time;

    // Convenience forwarders — avoid ctx.profile.mem.copy(...)
    void *mem_copy(void *d, const void *s, usize n) const noexcept;
    void *mem_set(void *d, int v, usize n) const noexcept;
    int   mem_cmp(const void *a, const void *b, usize n) const noexcept;
    void  bit_and(u64 *d, const u64 *a, const u64 *b, usize n) const noexcept;
    void  bit_or(u64 *d, const u64 *a, const u64 *b, usize n) const noexcept;
    void  bit_xor(u64 *d, const u64 *a, const u64 *b, usize n) const noexcept;
    void  bit_not(u64 *d, const u64 *a, usize n) const noexcept;
    usize popcount_range(const u64 *d, usize n) const noexcept;
};
```

When ECS is added, `ExecContext` will also carry `World &world`, keeping
system signatures minimal:

```cpp
// Before (many params):
void execute(World &w, const DispatchTable &d) noexcept;

// After (single context):
void execute(ExecContext &ctx) noexcept;
```

## Archetype-Aware Dispatch (Design Sketch)

When ECS archetypes exist, different chunk densities can select different
execution profiles:

```cpp
void process_physics(ExecContext &ctx) noexcept {
    auto query = ctx.world.query<Position, Velocity>();

    if (ctx.profile.simd_width >= 32 && query.density() > 0.8f) {
        // Dense chunks → high-throughput profile
        ExecutionProfile dense = ctx.profile;
        // future: dense.mem.copy = avx512_copy;
        for (auto chunk : query.dense_chunks()) {
            process_chunk(chunk, dense);
        }
    } else {
        // Sparse chunks → standard scalar profile
        for (auto entity : query) {
            process_entity(entity, ctx.profile);
        }
    }
}
```

Future profiles could include:
- **SIMD-dense**: AVX-512 for wide contiguous arrays
- **Sparse-scalar**: scalar ops for thin archetypes
- **Huge-page**: transparent hugepage aware allocator companion
- **Plugin**: GPU/WASM dispatch via VTable bridge

## Why Archetype-Aware?

Archetype density varies widely during gameplay:
- **Dense chunks** (entities with all components): benefit from wide SIMD
  operations, prefetch streaming, write-combining
- **Sparse chunks** (tag-only entities, removed components): scalar dispatch
  avoids SIMD setup overhead

A single global profile forces worst-case assumptions. Per-chunk selection
lets the engine choose the optimal profile for each memory layout.

## Migration Path

1. ✅ ExecutionProfile replaces DispatchTable (this PR)
2. ✅ ExecContext created with forwarders
3. ⬜ ExecContext gains World& when ECS is built
4. ⬜ System::execute(ExecContext&) replaces dual-param signature
5. ⬜ Archetype query returns density hint for profile selection
6. ⬜ Plugin VTable bridge for external backends
