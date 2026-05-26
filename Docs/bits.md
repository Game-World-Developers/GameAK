# Bits Module

## BitOps (`<AK/Core/Bits/BitOps.hpp>`)

### Summary

Five `constexpr` bit-level utility functions: single-bit mask generation, low-bit mask, power-of-two test, aligned rounding, and population count.
All functions are `[[nodiscard]]` and usable in `static_assert`.

### Guarantees

- all functions are `constexpr` — evaluable at compile time
- `is_power_of_two` returns `false` for zero
- `align_up` produces correct results only when alignment is a power of two
- `popcount` uses the Brian Kernighan algorithm — O(number of set bits)

### Non-Guarantees

- `align_up` behavior is undefined if alignment is not a power of two (no validation in release)
- `popcount` is not vectorized — it is a scalar loop over set bits

### Failure Semantics

- no runtime failure paths exist — all functions operate on values with no allocation or side effects
- invalid alignment passed to `align_up` produces a garbage result (not trapped)

### Complexity

| Function | Complexity |
|----------|-------------|
| `bit` | O(1) |
| `mask` | O(1) |
| `is_power_of_two` | O(1) |
| `align_up` | O(1) |
| `popcount` | O(number of set bits) |

### Threading

All functions are stateless and reentrant — safe for concurrent call from any number of threads.

### Valid Usage

```cpp
static_assert(Bits::is_power_of_two(64u));
static_assert(Bits::align_up(100u, 64u) == 128);
static_assert(Bits::popcount(0xFFu) == 8);

u32 flag = Bits::bit(5u);   // 0b100000
u32 low  = Bits::mask(4u);  // 0b1111
```

### Invalid Usage

```cpp
Bits::align_up(15u, 5u);  // alignment 5 is not a power of two — result is garbage
```

### Invariants

- `bit(n)` is equivalent to `T(1) << n`
- `mask(n)` is equivalent to `(T(1) << n) - 1`
- `align_up(value, alignment)` returns the smallest multiple of `alignment` that is >= `value`

### Integration Notes

- `align_up` is used by `ArenaAllocator` for aligned bump allocation
- `popcount` is used by `BitArray::popcount()` through the Backend layer
- all functions are header-only and inline — zero call overhead when used with constant propagation

---

## BitMask (`<AK/Core/Bits/BitMask.hpp>`)

### Summary

Type-safe bitmask wrapper over a `u64`.
Enum values name individual bit positions.
All operations are `constexpr`.

### Guarantees

- `set`, `clear`, `has` operate on a single bit — O(1)
- internal storage is a single `u64` — no heap allocation
- all operations are `constexpr`

### Non-Guarantees

- enum values outside `[0, 63]` produce undefined behavior (no bounds checking)
- not thread-safe for concurrent modification

### Failure Semantics

- `set`/`clear`/`has` with an enum value >= 64 shifts by 64 or more — undefined behavior in C++
- no debug validation for out-of-range enum values

### Memory Behavior

- stores a single `u64` value — no dynamic memory
- trivially copyable

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `set` | O(1) |
| `clear` | O(1) |
| `has` | O(1) |
| `raw` | O(1) |

### Threading

- concurrent reads (`has`, `raw`) are safe
- concurrent writes (`set`, `clear`) from multiple threads are unsafe

### Valid Usage

```cpp
enum class EntityFlag : u64 {
    Active     = 0,
    Visible    = 1,
    Collidable = 2,
};

BitMask<EntityFlag> flags;
flags.set(EntityFlag::Active);
flags.set(EntityFlag::Collidable);

if (flags.has(EntityFlag::Active)) { /* ... */ }
flags.clear(EntityFlag::Collidable);

u64 raw = flags.raw();
```

### Invalid Usage

```cpp
enum class BadEnum : u64 {
    Flag = 100, // bit position 100 — exceeds u64 width
};

BitMask<BadEnum> mask;
mask.set(BadEnum::Flag); // undefined behavior: shift by 100
```

### Invariants

- internal `m_value` always reflects the union of all `set` calls minus all `clear` calls
- default-constructed mask has `m_value == 0`

### Integration Notes

- suitable for flag fields in ECS components, render state, entity metadata
- trivial to serialize: read/write `raw()`

---

## BitArray (`<AK/Core/Bits/BitArray.hpp>`)

### Summary

Non-owning view over a caller-provided `u64` array treated as a bitset.
Supports single-bit access and bulk operations (AND, OR, XOR, NOT, popcount) delegated to the Backend VTable.
Compile-time bounds checking via `BitCheck` template parameter.

### Guarantees

- `set`, `clear`, `test` are O(1)
- bulk operations (`and_with`, `or_with`, `xor_with`, `negate`, `popcount`) dispatch through `Backend::g_vtable`
- `BitCheck::Bounded` validates bit indices against `words() * 64` in debug builds
- `BitCheck::None` produces zero overhead for bounds checking

### Non-Guarantees

- `BitCheck::Bounded` validation is compiled out in release builds (`GAMEAK_DEBUG_VALIDATE` not defined)
- `popcount` complexity depends on the active backend (scalar: O(n), future SIMD: vectorized)
- does not own the underlying storage — caller must ensure storage outlives the view

### Failure Semantics

- out-of-range bit index with `BitCheck::Bounded` triggers `GAMEAK_DEBUG_BREAK()` in debug builds
- out-of-range bit index with `BitCheck::None` produces undefined behavior (wild memory access)
- `and_with`/`or_with`/`xor_with` with `other` of different size produces undefined behavior

### Memory Behavior

- does not allocate or free memory
- operates in-place on caller-provided `u64` array
- bulk operations read from `other` and write to `this` — source and destination must not alias for correct results

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `set` | O(1) |
| `clear` | O(1) |
| `test` | O(1) |
| `reset` | O(n) via `Backend::mem_set` |
| `and_with` | O(n) via `Backend::bitset_and` |
| `or_with` | O(n) via `Backend::bitset_or` |
| `xor_with` | O(n) via `Backend::bitset_xor` |
| `negate` | O(n) via `Backend::bitset_not` |
| `popcount` | O(n) via `Backend::bitset_popcount_range` |

### Threading

- concurrent reads from distinct `BitArray` instances are safe
- concurrent writes to the same `BitArray` from multiple threads are unsafe

### Valid Usage

```cpp
u64 storage[4] = {};
BitArray<BitCheck::Bounded> bits(storage, 4);

bits.set(10);
bits.set(200);
bool b = bits.test(10); // true

bits.and_with(other); // bulk AND through backend
usize count = bits.popcount();
bits.reset();
```

### Invalid Usage

```cpp
u64 storage[2] = {};
BitArray<BitCheck::None> bits(storage, 2);

bits.set(200); // undefined behavior: index 200 exceeds 128 bits

BitArray<BitCheck::None> other(storage2, 4);
bits.and_with(other); // undefined behavior: word_count mismatch
```

### Invariants

- `words()` is constant after construction
- `data()` returns the pointer passed at construction
- after `reset()`, all bits are zero

### Integration Notes

- bulk operations are backend-dispatched — switching to SIMD requires no caller changes
- suitable for entity component masks, visibility sets, allocation bitmaps
- compile-time bounds checking eliminates runtime overhead in hot paths

---

## BitPack (`<AK/Core/Bits/BitPack.hpp>`)

### Summary

Bit-level serialization over a caller-provided byte buffer.
Writes and reads arbitrary-width values LSB-first.
Uses `BitCount` strong typedef to prevent accidental integer-passing errors.

### Guarantees

- `write` and `read` are O(1) per call
- `BitCount` prevents implicit conversion from raw integers
- stream position advances monotonically

### Non-Guarantees

- does not validate that writes stay within buffer bounds
- does not support endian selection — always LSB-first
- no alignment guarantees for serialized fields

### Failure Semantics

- writing past the end of the buffer produces undefined behavior (buffer overflow)
- reading past the end of written data reads zero bits from out-of-bounds memory
- no bounds checking is performed in any build mode

### Memory Behavior

- operates in-place on caller-provided buffer
- does not allocate or free memory
- stream position is stored as a single `usize` bit offset

### Complexity

| Operation | Complexity |
|-----------|-------------|
| `write` | O(1) |
| `read` | O(1) |
| `reset` | O(1) |
| `bit_offset` | O(1) |

### Threading

- external synchronization required for concurrent access
- concurrent read/write from multiple threads is unsafe

### Valid Usage

```cpp
u8 buffer[64] = {};
BitPack pack(buffer);

pack.write(0x1A, BitCount{5});
pack.write(0x1F3, BitCount{13});
pack.write(0x5, BitCount{3});

pack.reset();

u64 a = pack.read(BitCount{5});  // 0x1A
u64 b = pack.read(BitCount{13}); // 0x1F3
u64 c = pack.read(BitCount{3});  // 0x5
```

### Invalid Usage

```cpp
BitPack pack(buffer);
pack.write(0xFF, BitCount{8});

BitPack pack2(buffer); // shares same buffer
pack2.write(0xAA, BitCount{8}); // overwrites pack's data if position not managed

// Writing beyond buffer:
BitPack small(small_buf); // small_buf is 1 byte
small.write(0xFFFF, BitCount{16}); // undefined behavior: buffer overflow
```

### Invariants

- `bit_offset()` returns the total number of bits written or read so far
- `reset()` sets `bit_offset()` to zero without modifying buffer contents
- read-after-write at the same offset reproduces the original value

### Integration Notes

- suitable for network packet serialization, save data encoding, compact component storage
- `BitCount` strong typedef prevents `write(value, 5)` ambiguity — callers must write `write(value, BitCount{5})`
- no heap allocation during any operation
