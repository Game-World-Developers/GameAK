# SPEC-018: Data Layout

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-17

---

## Summary

This specification defines how Data Blocks are organized in memory through layout strategies.

Layout determines the physical arrangement of data fields within and across Data Blocks of the same type.

Layout is independent from representation, behavior, and identity.

Different layout strategies exist to optimize for different access patterns.

---

## Behavior

### Scenario: Blocks have a default layout strategy

Given a BlockTypeDescriptor without explicit layout

When the Runtime allocates a Data Block of that type

Then the block uses AoS layout

**Test:** `test_data_layout.cpp` — `default_layout_is_aos`

### Scenario: Layout is a type-level property

Given a BlockTypeDescriptor with layout = SoA

When multiple Data Blocks of that type are created

Then all blocks of that type share the SoA layout strategy

**Test:** `test_data_layout.cpp` — `layout_is_type_property`

### Scenario: AoS layout stores fields per entity

Given a block type with fields {position, velocity, mass} and N blocks

When blocks are allocated as AoS

Then each block contains all fields for one entity, laid out consecutively

**Test:** `test_data_layout.cpp` — `aos_field_ordering`

### Scenario: SoA layout stores fields per array

Given a block type with fields {position, velocity, mass} and N blocks

When blocks are allocated as SoA

Then each field is stored as a separate contiguous array of N elements

**Test:** `test_data_layout.cpp` — `soa_field_separation`

### Scenario: AoSoA layout stores fields in chunks

Given a block type with fields {position, velocity, mass}, chunk size = 8

When blocks are allocated as AoSoA

Then fields are grouped in chunks of 8, each chunk storing all fields as SoA

**Test:** `test_data_layout.cpp` — `aosoa_chunk_layout`

### Scenario: Controllers read state independently of layout

Given a block type with layout = SoA

When a Controller reads a field from a Data Block

Then the Controller receives the correct value regardless of physical layout

**Test:** `test_data_layout.cpp` — `layout_transparent_to_controllers`

### Scenario: Runtime may convert layout strategy

Given a block type currently stored as AoS

When the Runtime converts the type to SoA

Then all existing Data Blocks of that type are migrated, and identities remain stable

**Test:** `test_data_layout.cpp` — `layout_conversion_preserves_identity`

---

## Constraints

* Layout is a property of BlockTypeDescriptor, not individual Data Blocks.
  * **Test verification:** `test_data_layout.cpp` — `layout_is_type_property`
* Layout conversion must preserve Data Block identity.
  * **Test verification:** `test_data_layout.cpp` — `layout_conversion_preserves_identity`
* Layout conversion must not change observable state.
  * **Test verification:** `test_data_layout.cpp` — `layout_conversion_preserves_values`
* Layout must be transparent to Controllers.
  * **Test verification:** `test_data_layout.cpp` — `layout_transparent_to_controllers`
* Multiple layout strategies may coexist across different block types.
  * **Test verification:** `test_data_layout.cpp` — `multiple_layouts_coexist`

---

## Out of Scope

* Individual block-level layout overrides (layout is type-level).
* Runtime auto-tuning or profiling-guided layout selection.
* Custom/user-defined layout strategies.
* Nested or hierarchical layouts.

---

## Open Questions

* [x] Should layout be configurable at Runtime creation or per type registration?

**Answer:** Per type registration. Layout is configured via `register_type<MyBlock>(layout=SoA)`.

* [x] What is the mechanism for triggering layout conversion?

**Answer:** Automatic, based on access profile.

* [x] Should layout conversion be a Command or a Runtime API call?

**Answer:** Command. Layout conversion is requested via a Command.

* [x] How should SoA field padding/alignment be handled?

**Answer:** Automatic, based on `alignof` of each field.

* [x] Should AoSoA chunk size be configurable per type?

**Answer:** Yes. AoSoA chunk size is configurable per type via `register_type<MyBlock>(AoSoA{chunk_size=32})`.

---

## Definitions

### AoS (Array of Structs)

Fields for a single entity are stored contiguously. Entity N+1 follows entity N. Best for access patterns that touch multiple fields of the same entity.

### SoA (Struct of Arrays)

Each field is stored in its own contiguous array across all entities of the same type. Best for access patterns that touch the same field across many entities.

### AoSoA (Array of Struct of Arrays)

Entities are divided into fixed-size chunks. Within each chunk, fields are stored as SoA. Compromise between AoS and SoA for cache utilization.

### Layout Conversion

The process of changing the physical storage strategy of a block type while preserving identity and value semantics.
