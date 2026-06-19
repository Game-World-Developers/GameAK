# SPEC-025: Semantic Storage Inference

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-18

---

## Summary

The physical representation of data should not be the game author's responsibility.

The author describes meaning, constraints, and domain. The Runtime and its Specializers decide bit width, physical layout, compaction, alignment, and vectorization.

The semantics of data are independent of its physical representation.

---

## SemanticConstraint

A `SemanticConstraint` is a value type that captures the semantic range of a data field.

The Runtime consumes `SemanticConstraint` at block type registration time to infer the minimum storage size required.

### Fields

* `min` — Minimum inclusive value (signed 64-bit).
* `max` — Maximum inclusive value (signed 64-bit).
* `is_bool` — If true, the field is a boolean (1 bit).
* `enum_count` — Number of distinct enum values.

### Validation

A `SemanticConstraint` is valid when at least one of the following is true:

* `is_bool` is true.
* `enum_count > 0`.
* `max > min` (a range with at least two values).
* `max == min && max != 0` (a single non-zero value).

---

## bits_for

The `bits_for` function computes the minimum number of bits required to represent all values within a `SemanticConstraint`.

### Rules

* **Bool:** 1 bit.
* **Enum:** `ceil(log2(enum_count))` bits. Minimum 1.
* **Range (max > min):** `ceil(log2(max - min + 1))` bits. Minimum 1.
* **Single value (max == min != 0):** 1 bit.
* **No constraint (default-constructed):** Returns 0 (no inference possible).

### Scenario: Bool inference

Given a `SemanticConstraint` with `is_bool = true`

When `bits_for` is called

Then it returns 1.

**Test:** `test_semantic.h` — `semantic_bits_for_bool`

### Scenario: Enum inference

Given a `SemanticConstraint` with `enum_count = 4`

When `bits_for` is called

Then it returns 2.

**Test:** `test_semantic.h` — `semantic_bits_for_enum`

### Scenario: Range inference

Given a `SemanticConstraint` with `min = 0, max = 100`

When `bits_for` is called

Then it returns 7 bits.

**Test:** `test_semantic.h` — `semantic_bits_for_range_100`

### Scenario: Single value

Given a `SemanticConstraint` with `min = 42, max = 42`

When `bits_for` is called

Then it returns 1.

**Test:** `test_semantic.h` — `semantic_bits_for_single_value`

### Scenario: Zero range

Given a `SemanticConstraint` with `min = 0, max = 0`

When `bits_for` is called

Then it returns 0 (no valid constraint).

**Test:** `test_semantic.h` — `semantic_bits_for_zero_range`

### Scenario: Negative range

Given a `SemanticConstraint` with `min = -100, max = 100`

When `bits_for` is called

Then it returns 8 bits (201 values).

**Test:** `test_semantic.h` — `semantic_bits_for_negative_range`

### Scenario: Large range

Given a `SemanticConstraint` with `min = 0, max = 1000000`

When `bits_for` is called

Then it returns 20 bits.

**Test:** `test_semantic.h` — `semantic_bits_for_large_range`

---

## BlockTypeDescriptor Integration

`BlockTypeDescriptor` gains an optional `SemanticConstraint` pointer.

### Scenario: Inference at registration

Given a `BlockTypeDescriptor` with `size = 0, alignment = 0` and a valid `SemanticConstraint` pointer

When `register_block_type` is called

Then the Runtime infers `size` from `bits_for` rounded up to bytes, and sets `alignment` to the inferred size (minimum 1).

**Test:** `test_semantic.h` — `semantic_registration_infers_size`

### Scenario: Explicit size with semantic

Given a `BlockTypeDescriptor` with `size = 4, alignment = 4` and a valid `SemanticConstraint` pointer

When `register_block_type` is called

Then the Runtime respects the explicit size and alignment, ignoring the inference.

**Test:** `test_semantic.h` — `semantic_registration_explicit_size`

### Scenario: No semantic (backward compatible)

Given a `BlockTypeDescriptor` with `size = 4, alignment = 4` and `semantic = nullptr`

When `register_block_type` is called

Then the Runtime uses the explicit size and alignment exactly as today.

**Test:** `test_semantic.h` — `semantic_registration_no_semantic`

### Scenario: Invalid semantic constraint

Given a `BlockTypeDescriptor` with `size = 0` and an invalid (default-constructed) `SemanticConstraint`

When `register_block_type` is called

Then it returns `ErrorCode::InvalidOperation`.

**Test:** `test_semantic.h` — `semantic_registration_invalid_constraint`

---

## Constraints

* `SemanticConstraint` must be a value type (copyable, movable).
* `bits_for` must be `constexpr`.
* Inference is only applied at `register_block_type` time.
* Once registered, a block type's size is fixed for its lifetime.
* A `SemanticConstraint` with `is_bool` takes precedence over range fields.
* Floats are not supported by inference — their size must be explicit.
* The Specializer (SPEC-003 CRTP) may override the inferred size with a larger value, but never smaller than the minimum inferred size.
  * **Test verification:** `test_semantic.h` — enforce via design (no CRTP override test in this spec)

---

## Out of Scope

* Bit-level access or storage (delegated to SPEC-019).
* Layout decisions (AoS, SoA, AoSoA — delegated to SPEC-018).
* YAML, JSON, or any external format parsing.
* Runtime changes to semantic constraints after registration.
* Float inference.
* The Seed code generator (separate tool).

---

## Open Questions

None.

---

## Definitions

### SemanticConstraint

A value type describing the semantic range of a data field, consumed by the Runtime at registration time.

### bits_for

A constexpr function that computes the minimum bit width required to represent a `SemanticConstraint`.

### Inference

The process of determining storage size from semantic constraints at block type registration.
