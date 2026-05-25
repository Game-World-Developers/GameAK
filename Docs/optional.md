# Optional

## `<AK/Core/Optional.hpp>`

A stack-allocated optional value type — no dynamic allocation, no exceptions.

### Design

- Storage is a `alignas(T) u8 m_storage[sizeof(T)]` byte array
- A `bool m_has_value` flag tracks state
- Destructor elision: if `IsTriviallyDestructible<T>`, the destructor is a no-op (optimized away by the compiler)
- Empty access in debug builds triggers `GAMEAK_DEBUG_BREAK()` (via `value()`)
- Empty access in release builds is undefined behavior (maximum performance)

### API

| Method | Description |
|--------|-------------|
| `Optional()` | Empty |
| `Optional(Nullopt)` | Empty |
| `Optional(const T&)` | Construct from value |
| `Optional(T&&)` | Move construct |
| `Optional(const Optional&)` | Copy construct |
| `Optional(Optional&&)` | Move construct |
| `operator=(const T&)` | Assign value |
| `operator=(T&&)` | Move assign |
| `operator=(const Optional&)` | Copy assign |
| `operator=(Optional&&)` | Move assign |
| `has_value()` / `operator bool()` | Check if value is present |
| `operator*()` | Unchecked access |
| `value()` | Checked access (`GAMEAK_DEBUG_BREAK()` if empty) |
| `value_or(T fallback)` | Return value or fallback |
| `reset()` | Destroy value and set empty |

### Example

```cpp
#include <AK/Core/Optional.hpp>

GameAK::Optional<int> maybe;

maybe = 42;
if (maybe) {
  int a = *maybe;               // 42 (unchecked)
  int b = maybe.value();        // 42 (checked)
  int c = maybe.value_or(-1);   // 42
}

maybe.reset();
// maybe.value() would trigger DEBUG_BREAK in debug builds

// With trivially destructible types, zero overhead:
GameAK::Optional<GameAK::i32> opt;
static_assert(sizeof(opt) == sizeof(GameAK::i32) + sizeof(bool));
```

### Nullopt

```cpp
namespace GameAK {
  struct NulloptT {};
  inline constexpr NulloptT Nullopt{};
}
```

Used to explicitly clear or construct an empty Optional:

```cpp
GameAK::Optional<int> opt = GameAK::Nullopt;
opt = GameAK::Nullopt;  // equivalent to opt.reset()
```

### Used Internally

`Optional<usize>` is used as the return type of `ArenaAllocator::alloc_impl()` to cleanly express allocation success or failure:

```cpp
Optional<usize> ArenaAllocator::alloc_impl(usize size, usize alignment) const noexcept {
  const usize aligned_offset = Bits::align_up(m_offset, alignment);
  if (size > m_capacity - aligned_offset) {
    return Nullopt;  // allocation failed
  }
  return aligned_offset;  // success
}
```
