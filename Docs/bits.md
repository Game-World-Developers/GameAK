# Bits Module

Four headers provide bit-level operations, containers, and serialization.

## BitOps (`<AK/Core/Bits/BitOps.hpp>`)

Lightweight `constexpr` bit operations — all functions are `[[nodiscard]]` and usable in `static_assert`.

### `bit(n)`

Returns `T(1) << n` — a single-bit mask at position `n`.

```cpp
GameAK::Bits::bit(0u);   // 0b0001
GameAK::Bits::bit(3u);   // 0b1000
```

### `mask(n)`

Returns `(T(1) << n) - 1` — a mask covering the lowest `n` bits.

```cpp
GameAK::Bits::mask(0u);  // 0b0000
GameAK::Bits::mask(3u);  // 0b0111
GameAK::Bits::mask(8u);  // 0xFF
```

### `is_power_of_two(value)`

Returns `true` if `value` is a power of two (and non-zero). Classic `v && !(v & (v-1))`.

```cpp
GameAK::Bits::is_power_of_two(16u);  // true
GameAK::Bits::is_power_of_two(0u);   // false
GameAK::Bits::is_power_of_two(3u);   // false
```

### `align_up(value, alignment)`

Rounds `value` up to the next multiple of `alignment`. Alignment must be a power of two.

```cpp
GameAK::Bits::align_up(15u, 16u);    // 16
GameAK::Bits::align_up(16u, 16u);    // 16
GameAK::Bits::align_up(0u, 4u);      // 0
```

Used extensively by `ArenaAllocator` for aligned bump allocation.

### `popcount(value)`

Returns the number of set bits. Uses the Brian Kernighan algorithm (one iteration per set bit).

```cpp
GameAK::Bits::popcount(0xFFu);       // 8
GameAK::Bits::popcount(0u);          // 0
```

## BitMask (`<AK/Core/Bits/BitMask.hpp>`)

Type-safe bitmask wrapper over a `u64`. The enum values name bit positions.

```cpp
enum class EntityFlag : GameAK::u64 {
  Active     = 0,   // bit 0
  Visible    = 1,   // bit 1
  Collidable = 2,   // bit 2
};

GameAK::Bits::BitMask<EntityFlag> flags;

flags.set(EntityFlag::Active);
flags.set(EntityFlag::Collidable);

if (flags.has(EntityFlag::Active)) { /* ... */ }

flags.clear(EntityFlag::Collidable);

GameAK::u64 raw = flags.raw();  // underlying u64
```

### API

| Method | Description |
|--------|-------------|
| `BitMask()` | Empty mask (no flags) |
| `BitMask(Enum)` | Mask with initial flag set |
| `set(Enum)` | Set flag on |
| `clear(Enum)` | Set flag off |
| `has(Enum)` | Test if flag is on |
| `raw()` | Underlying `u64` |

All methods are `constexpr`.

## BitArray (`<AK/Core/Bits/BitArray.hpp>`)

Non-owning view over a `u64`-based bitset with optional compile-time bounds checking.

```cpp
#include <AK/Core/Bits/BitArray.hpp>

GameAK::u64 storage[4] = {};
GameAK::Bits::BitArray<GameAK::Bits::BitCheck::Bounded> bits(storage, 4);
//                     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//                     BitCheck::None = no bounds checks (zero overhead)
//                     BitCheck::Bounded = runtime bounds checking

bits.set(10);
bits.set(200);

bool b = bits.test(10);  // true

bits.and_with(other);    // bulk AND via Backend
bits.negate();           // bulk NOT via Backend

GameAK::usize count = bits.popcount();  // via Backend
bits.reset();           // zeroes all bits via Backend::mem_set
```

### API

| Method | Description |
|--------|-------------|
| `BitArray(u64* data, usize words)` | Wrap externally-owned array |
| `set(bit)` | Set a single bit |
| `clear(bit)` | Clear a single bit |
| `test(bit)` | Test a single bit |
| `reset()` | Zero all bits |
| `and_with(other)` | Bulk bitwise AND |
| `or_with(other)` | Bulk bitwise OR |
| `xor_with(other)` | Bulk bitwise XOR |
| `negate()` | Bulk bitwise NOT |
| `popcount()` | Count set bits |
| `words()` | Number of u64 words |
| `data()` | Underlying pointer |

Bulk operations (AND, OR, XOR, NOT, popcount) are delegated to the `Backend` VTable, enabling transparent SIMD acceleration. The `BitCheck` template parameter controls bounds checking **at compile time** — `BitCheck::None` has zero overhead.

## BitPack (`<AK/Core/Bits/BitPack.hpp>`)

Bit-level serialization — writes and reads arbitrary-width values to/from a byte buffer, LSB first.

```cpp
#include <AK/Core/Bits/BitPack.hpp>

GameAK::u8 buffer[64] = {};
GameAK::Bits::BitPack pack(buffer);

// Write values: 5 bits, 13 bits, 3 bits
pack.write(0x1A, GameAK::Bits::BitCount{5});
pack.write(0x1F3, GameAK::Bits::BitCount{13});
pack.write(0x5, GameAK::Bits::BitCount{3});

// Reset stream position
pack.reset();

// Read back in same order
GameAK::u64 a = pack.read(GameAK::Bits::BitCount{5});  // 0x1A
GameAK::u64 b = pack.read(GameAK::Bits::BitCount{13}); // 0x1F3
GameAK::u64 c = pack.read(GameAK::Bits::BitCount{3});  // 0x5

// Current stream position (in bits)
GameAK::usize pos = pack.bit_offset();
```

### API

| Method | Description |
|--------|-------------|
| `BitPack(u8* buffer)` | Wrap externally-owned buffer |
| `write(u64 value, BitCount bits)` | Write low N bits |
| `read(BitCount bits)` | Read N bits, LSB first |
| `reset()` | Reset stream position to 0 |
| `bit_offset()` | Current position in bits |

`BitCount` is a strong typedef over `usize` — prevents accidentally passing a raw integer instead of an explicit bit count.
