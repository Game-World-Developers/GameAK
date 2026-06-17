# SPEC-019: Bit Representation

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-17

---

## Summary

This specification defines primitives for representing simulation state using bit-oriented structures.

Bit representations operate at the bit level and may be used for storage, query, filtering, synchronization, or execution.

Bit representations are independent from data layout strategies.

---

## Behavior

### Scenario: BitSet stores fixed-size bits

Given a BitSet of size 64

When bits 3, 7, and 31 are set

Then test(3), test(7), and test(31) return true, and all others return false

**Test:** `test_bit_representation.cpp` — `bitset_set_and_test`

### Scenario: BitSet supports bitwise operations

Given BitSet A = 0b1100 and BitSet B = 0b1010

When AND, OR, and XOR are computed

Then A & B = 0b1000, A | B = 0b1110, A ^ B = 0b0110

**Test:** `test_bit_representation.cpp` — `bitset_bitwise_ops`

### Scenario: BitVector stores dynamic-size bits

Given an empty BitVector

When 100 bits are appended

Then the BitVector holds exactly 100 bits, accessible by index

**Test:** `test_bit_representation.cpp` — `bitvector_dynamic_size`

### Scenario: BitFlags maps named flags to positions

Given BitFlags with definitions {Visible=0, Enabled=1, Locked=2}

When Enabled and Locked are set

Then is_set(Enabled) and is_set(Locked) return true, and is_set(Visible) returns false

**Test:** `test_bit_representation.cpp` — `bitflags_named_access`

### Scenario: Bit packing serializes multiple values into a word

Given a packed word with fields {x:3, y:5, z:8} (total 16 bits)

When x=5, y=12, z=200 are packed

Then unpacking yields x=5, y=12, z=200

**Test:** `test_bit_representation.cpp` — `bit_packing_roundtrip`

### Scenario: Bit representations are copyable

Given a BitSet with bits set

When it is copied

Then the copy is equal to the original and independent in memory

**Test:** `test_bit_representation.cpp` — `bit_representation_copy`

---

## Constraints

* BitSet size must be known at compile time.
  * **Test verification:** `test_bit_representation.cpp` — `bitset_compile_time_size`
* BitVector must grow dynamically without external allocation beyond its own internal storage.
  * **Test verification:** `test_bit_representation.cpp` — `bitvector_dynamic_growth`
* BitFlags must be backed by a fixed-size integer (default 64 bits).
  * **Test verification:** `test_bit_representation.cpp` — `bitflags_fixed_backing`
* All bit operations must be deterministic.
  * **Test verification:** `test_bit_representation.cpp` — `bit_ops_deterministic`
* Bit representations must not allocate on the heap for BitSet or BitFlags (BitVector may allocate).
  * **Test verification:** `test_bit_representation.cpp` — `allocation_free_bitset`

---

## Out of Scope

* Arbitrary-precision bit integers (bitsets are not big integers).
* Compression or encoding schemes.
* Hardware-specific bit manipulation intrinsics (generic implementations first).
* Bit-serialized network protocols.

---

## Open Questions

* [x] Should BitSet be backed by std::bitset or a custom implementation?

**Answer:** Custom implementation, based on the Platform layer for portability.

* [x] Should BitVector use flat_vector as internal storage?

**Answer:** Yes. BitVector uses `flat_vector` as internal storage.

* [x] Should bit packing support compile-time field definitions via templates?

**Answer:** Yes. Bit packing supports compile-time field definitions via templates.

* [x] Should there be a dynamic BitFlags (runtime-defined flag names)?

**Answer:** Yes. A dynamic BitFlags variant with runtime-defined flag names exists.

* [x] Should bit representations be usable as Data Block payloads directly?

**Answer:** May. Bit representations may be used as Data Block payloads, but this is optional.

---

## Definitions

### BitSet

A fixed-size sequence of bits indexed by position. Size is known at compile time.

### BitVector

A dynamically-sized sequence of bits. Grows on demand.

### BitFlags

A named interface over a fixed-size bit set. Each named flag maps to a bit position.

### Bit Packing

Encoding multiple small integer values into a single word at specific bit positions and widths.

### Bitwise Operation

An operation that acts on individual bits: AND, OR, XOR, NOT, shift, count.
