# SPEC-035: Bulk Iteration API

Layer: Runtime

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-21

---

## Summary

This specification defines a bulk iteration API that returns dense contiguous spans of field data across all entities of a type.

The API skips per-block indirection and works with any layout strategy (AoS, SoA, AoSoA, Archetype).

Bulk iteration is read-only. Mutations must go through the command system to preserve determinism.

---

## Behavior

### Scenario: Bulk field read for Archetype storage

Given 128 entities of type `Player` with `Archetype{64}` layout and field `hp`

When a Controller calls `view.field_span<HP>(type_id)`

Then it receives `std::span<const HP>` of length 128 with contiguous HP values

**Test:** `test_bulk_iteration.h` — `bulk_archetype_field_span`

### Scenario: Bulk field read for SoA storage

Given N entities of type `Enemy` with SoA layout and field `position`

When a Controller calls `view.field_span<Position>(enemy_type)`

Then it receives `std::span<const Position>` of length N

**Test:** `test_bulk_iteration.h` — `bulk_soa_field_span`

### Scenario: Bulk field read for AoS storage

Given N entities of type `Item` with AoS layout and field `weight`

When a Controller calls `view.field_span<Weight>(item_type)`

Then it receives `std::span<const Weight>` of length N by scanning all blocks and assembling a contiguous view

**Test:** `test_bulk_iteration.h` — `bulk_aos_field_span`

### Scenario: Bulk read of empty type returns empty span

Given a type with zero entities

When a Controller calls `view.field_span<Field>(type_id)`

Then it receives an empty span

**Test:** `test_bulk_iteration.h` — `bulk_empty_type`

### Scenario: Bulk read for unregistered type returns empty span

Given no registration for type_id = 999

When a Controller calls `view.field_span<Field>(999)`

Then it receives an empty span

**Test:** `test_bulk_iteration.h` — `bulk_unregistered_type`

### Scenario: Bulk iteration is read-only

Given a Controller holding a `field_span`

When it attempts to write through the span

Then compilation fails (span is `const`)

**Test:** `test_bulk_iteration.h` — `bulk_read_only`

---

## Constraints

* Bulk iteration must work identically across all layout strategies.
  * **Test verification:** `test_bulk_iteration.h` — `bulk_layout_independent`
* Bulk iteration must be read-only.
  * **Test verification:** compilation check — `bulk_read_only`
* Bulk iteration must not allocate memory per call (returns existing internal storage or stack buffer for AoS assembly).
  * **Test verification:** `test_bulk_iteration.h` — `bulk_no_allocation`
* `field_span` must be available on `StateView`.
  * **Test verification:** `test_bulk_iteration.h` — `bulk_on_state_view`

---

## Out of Scope

* Bulk writes (mutations must use Commands).
* Filtered iteration (e.g., "hp < 50" — use existing `find_blocks` / fluent queries).
* Multi-field iteration (each field is accessed independently).

---

## Open Questions

* [x] How does AoS bulk iteration work without copying all data?

**Answer:** For AoS, the existing `find_blocks_by_type` returns identities; the bulk API can iterate blocks and return the field values. If callers need truly contiguous memory for AoS, they should convert to Archetype or SoA layout first.

* [x] Can bulk iteration be used inside a parallel dispatch group?

**Answer:** Yes. Bulk iteration is read-only and does not require synchronization. Multiple parallel groups may read from the same type simultaneously.

* [x] Does bulk iteration support field name lookup or only member pointer?

**Answer:** Both. The API mirrors `StateView::field()` — supports `field_span<T>(type_id, field_name)` and `field_span<U>(type_id, U T::* member)`.

---

## Definitions

### Field Span

A `std::span<const T>` representing all values of field `T` across all entities of a given type.

### Bulk Iteration

The process of accessing all values of a field for all entities of a type without per-entity indirection.
