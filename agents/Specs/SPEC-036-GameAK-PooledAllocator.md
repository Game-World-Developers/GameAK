# SPEC-036: Pooled Allocator for Data Blocks

Layer: Runtime

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-21<｜end▁of▁thinking｜>Now let me implement in order. Starting with **SPEC-036: Pooled Allocator** — the simplest, most isolated change:

<｜｜DSML｜｜tool_calls>
<｜｜DSML｜｜invoke name="todowrite">
<｜｜DSML｜｜parameter name="todos" string="false">[{"priority":"high","content":"SPEC-036: Pooled Allocator for DataBlocks","status":"in_progress"},{"priority":"high","content":"SPEC-033: Archetype Storage layout strategy","status":"pending"},{"priority":"high","content":"SPEC-035: Bulk Iteration API on StateView","status":"pending"},{"priority":"high","content":"SPEC-034: Parallel Controller Dispatch","status":"pending"}]

---

## Summary

This specification defines a pooled memory allocator for Data Block storage.

Currently each `DataBlock` owns a `flat_vector<std::byte>` for its data, causing per-block heap fragmentation. The Pooled Allocator replaces per-block allocations with type-level memory pools, reducing allocation overhead and improving cache locality for blocks of the same type.

The Pooled Allocator is transparent to all consumers. No API changes are required for Controllers, Queries, Commands, or the Scheduler.

---

## Behavior

### Scenario: Blocks of the same type share a memory pool

Given 100 entities of type `Player` with fixed size 64 bytes

When the Runtime allocates blocks

Then all 100 blocks are allocated from a single contiguous pool instead of 100 individual heap allocations

**Test:** `test_pool.h` — `pool_same_type_shares_pool`

### Scenario: Pool grows on demand

Given a pool with initial capacity 64 and 65 entities

When the 65th entity is created

Then the pool doubles in capacity and all existing data remains valid

**Test:** `test_pool.h` — `pool_growth`

### Scenario: Destroyed blocks release pool slots for reuse

Given a pool with 10 entities

When entity 5 is destroyed

Then slot 5 becomes available and the next allocation reuses it without growing the pool

**Test:** `test_pool.h` — `pool_slot_reuse`

### Scenario: Pool is per-type

Given type A (size 32) and type B (size 128)

When entities of both types are created

Then each type uses its own independent pool

**Test:** `test_pool.h` — `pool_per_type`

### Scenario: Pool allocator is transparent to field access

Given an entity in a pool

When `block.field<T>(offset)` is called

Then it returns the correct value regardless of pool vs. individual allocation

**Test:** `test_pool.h` — `pool_transparent_field_access`

### Scenario: Variable-size blocks use fallback allocation

Given a block type with dynamic size (e.g., resizable)

When the block is resized beyond the pool slot size

Then it falls back to individual heap allocation

**Test:** `test_pool.h` — `pool_variable_size_fallback`

---

## Constraints

* Pool allocation must not change observable block behavior.
  * **Test verification:** `test_pool.h` — `pool_behavior_identical`
* Pool allocation must be transparent to all existing APIs.
  * **Test verification:** all existing tests pass with pooled storage
* Variable-size blocks must fall back to individual allocation.
  * **Test verification:** `test_pool.h` — `pool_variable_size_fallback`
* Pools must not cause memory leaks on Runtime destruction.
  * **Test verification:** `test_pool.h` — `pool_no_leak`

---

## Out of Scope

* Custom allocator injection (pools are managed by the Runtime).
* Pool defragmentation (slots are reused but not compacted).
* Cross-type shared pools.
* Thread-safe pool allocation (allocation is single-threaded in the command processing phase).

---

## Open Questions

* [x] Does the pool reserve memory upfront or grow lazily?

**Answer:** Lazily. The pool starts empty and grows in powers of 2 as blocks are created. This avoids wasting memory for types with few entities.

* [x] How does the pool interact with Archetype Storage?

**Answer:** Archetype Storage has its own internal dense storage and does not use per-block pools. Pooled allocation applies only to types using AoS layout. SoA/AoSoA layouts already use LayoutManager storage, not per-block pools.

* [x] Does save/load work correctly with pooled blocks?

**Answer:** Yes. `save()` copies block data out of pools into the snapshot. `load()` inserts data back through the normal allocation path, which uses pools.

---

## Definitions

### Pool

A contiguous memory region divided into fixed-size slots. Each slot holds one Data Block's data.

### Slot

A fixed-size region within a pool capable of holding one Data Block's data.

### Pool Growth

The process of allocating a new, larger pool and migrating existing data into it.
