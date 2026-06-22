# SPEC-033: Archetype Storage

Layer: Runtime

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-21

---

## Summary

This specification defines Archetype Storage, a dense memory layout strategy for Data Blocks of the same type.

Archetype Storage groups entities of the same type into fixed-size contiguous chunks where fields are stored as dense SoA arrays within each chunk.

Unlike the existing SoA layout, Archetype Storage adds a **sparse set** mapping Runtime Identity → (chunk_index, slot_index) for O(1) identity lookup and dense linear iteration without per-block `rb_tree` overhead.

Archetype Storage is a new `LayoutStrategy` that integrates with the existing `LayoutManager` and `convert_layout` command flow.

---

## Behavior

### Scenario: Archetype is opt-in per type

Given a BlockTypeDescriptor with `layout = Archetype{chunk_size = 64}`

When the Runtime creates Data Blocks of that type

Then blocks are stored in dense archetype chunks instead of individual `DataBlock` entries in the block tree

**Test:** `test_archetype.h` — `archetype_opt_in`

### Scenario: Dense field iteration

Given N entities of the same archetype type, each with field `hp`

When a Controller requests iteration over all `hp` values

Then it receives a contiguous `span<const float>` of N elements with no per-block indirection

**Test:** `test_archetype.h` — `archetype_dense_iteration`

### Scenario: O(1) identity lookup

Given an entity with Runtime Identity `id` stored in an archetype

When `get_block(id)` is called

Then the lookup returns the data without scanning the rb_tree

**Test:** `test_archetype.h` — `archetype_identity_lookup`

### Scenario: Identity remains stable through archetype conversion

Given entities in AoS layout

When a Controller issues `convert_layout(type_id, Archetype{64})`

Then all entities of that type are migrated to archetype storage and identities remain valid

**Test:** `test_archetype.h` — `archetype_conversion_preserves_identity`

### Scenario: Archetype supports add/remove at chunk boundaries

Given an archetype with chunk_size = 64 and chunk 0 full (64 entities)

When a new entity of that type is created

Then it is placed in chunk 1 (a new chunk)

**Test:** `test_archetype.h` — `archetype_chunk_overflow`

### Scenario: Destroyed entities leave gaps filled on compaction

Given an archetype with entities spanning 2 chunks

When entity in slot 5 of chunk 0 is destroyed

Then the entity from the last slot of chunk 1 is moved into slot 5, and the sparse set is updated

**Test:** `test_archetype.h` — `archetype_compaction`

### Scenario: Archetype coexists with AoS/SoA/AoSoA types

Given type A registered with Archetype layout and type B registered with AoS layout

When controllers operate on both types

Then both layouts function correctly and independently

**Test:** `test_archetype.h` — `archetype_coexists_with_other_layouts`

---

## Constraints

* Archetype Storage must preserve Runtime Identity stability.
  * **Test verification:** `test_archetype.h` — `archetype_identity_stable_after_migration`
* Archetype Storage must support the existing `field<T>(identity, offset)` access pattern.
  * **Test verification:** `test_archetype.h` — `archetype_field_access`
* Archetype chunk allocation must not cause observable side effects for non-archetype types.
  * **Test verification:** `test_archetype.h` — `archetype_no_side_effects`
* Archetype Storage must be convertible to/from AoS, SoA, and AoSoA.
  * **Test verification:** `test_archetype.h` — `archetype_round_trip_conversion`

---

## Out of Scope

* Automatic migration based on access profiling.
* Cross-type archetype groups (each archetype is single-type).
* GPU storage or compute.
* Persistent storage format for archetype chunks.

---

## Open Questions

* [x] Does Archetype Storage replace the existing `rb_tree`-based block storage?

**Answer:** No. Archetype Storage is an additional `LayoutStrategy`. Types that do not opt into archetype storage continue to use the existing per-block `rb_tree` storage. Both strategies coexist.

* [x] How is the sparse set maintained during compaction?

**Answer:** The sparse set is a `flat_vector<Identity>` (sparse) and `flat_vector<size_t>` (dense). On compaction, the dense array is updated with a swap-and-pop from the last chunk, and the sparse identity entry is remapped.

* [x] Can a type switch between archetype and non-archetype at runtime?

**Answer:** Yes, via `convert_layout`. The existing layout conversion mechanism handles migration between any two layout strategies.

---

## Definitions

### Archetype

A group of Data Blocks of the same type stored in dense contiguous memory chunks.

### Chunk

A fixed-size block of memory containing `chunk_size` entities in dense SoA field layout.

### Sparse Set

A data structure providing O(1) lookup from Runtime Identity to (chunk_index, slot_index) using a dense vector of identities and a sparse index vector.

### Compaction

The process of filling gaps left by destroyed entities by moving the last entity in the archetype into the vacated slot, preserving density.
