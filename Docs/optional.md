# Optional (`<AK/Core/Optional.hpp>`)

## Summary

`Optional<T>` is a stack-allocated optional value type.
No dynamic allocation, no exceptions.
Empty access triggers `GAMEAK_DEBUG_BREAK()` in debug builds and is undefined behavior in release builds.

## Guarantees

- no heap allocation occurs during any operation
- destructor is elided for `IsTriviallyDestructible<T>` types — zero overhead
- storage is `alignas(T) u8[sizeof(T)]` — no separate allocation
- `has_value()` and `operator bool()` are O(1)
- `value_or()` never triggers a debug break — returns the fallback instead

## Non-Guarantees

- does not support reference types (`T&`) — use pointers instead
- no monadic operations (`and_then`, `or_else`, `transform`)
- no support for `Optional<Optional<T>>` — flattening is not provided
- `operator*()` does not validate — callers must check `has_value()` first

## Failure Semantics

- `value()` on an empty optional triggers `GAMEAK_DEBUG_BREAK()` in debug builds (`GAMEAK_DEBUG_VALIDATE` defined)
- `value()` on an empty optional in release builds is undefined behavior
- `operator*()` on an empty optional is always undefined behavior
- `reset()` on an already-empty optional is a no-op

## Memory Behavior

- storage is inline within the `Optional` object — no dynamic memory
- sizeof `Optional<T>` is `sizeof(T) + sizeof(bool)` (plus padding)
- for trivially copyable types, copy/move are trivial

## Complexity

| Operation | Complexity |
|-----------|-------------|
| construction | O(1) |
| copy/move | O(1) |
| `has_value` | O(1) |
| `value` | O(1) |
| `value_or` | O(1) |
| `reset` | O(1) |

## Threading

- external synchronization required for concurrent modification
- concurrent reads from a const reference are safe only when no concurrent writes occur

## Valid Usage

```cpp
Optional<int> maybe;

maybe = 42;
if (maybe) {
    int a = *maybe;               // unchecked
    int b = maybe.value();        // checked
    int c = maybe.value_or(-1);   // 42
}

maybe.reset();
// maybe.has_value() == false

// Trivially destructible types have zero overhead:
Optional<i32> opt;
static_assert(sizeof(opt) == sizeof(i32) + sizeof(bool));
```

## Invalid Usage

```cpp
Optional<int> empty;
int x = *empty;         // undefined behavior — empty check required
int y = empty.value();  // DEBUG_BREAK() in debug, UB in release
```

## Invariants

- `m_has_value` is `true` iff storage contains a constructed `T`
- `reset()` destroys the contained `T` if present, then sets `m_has_value = false`
- move construction leaves the source in an empty state

## Integration Notes

- used internally by `ArenaAllocator::alloc_impl()` to express allocation success or failure
- compatible with `IsTriviallyDestructible` — trivial destructor is elided, reducing codegen
- `NulloptT` is a tag type for explicit empty construction; `GameAK::Nullopt` is the sentinel value

---

## NulloptT

### Summary

Tag type for explicit empty `Optional` construction.

### Guarantees

- `NulloptT` is an empty class — zero overhead
- `Optional<T>` can be constructed from or assigned to `NulloptT` to produce an empty state

### Valid Usage

```cpp
Optional<int> opt = Nullopt;  // empty
opt = Nullopt;                // equivalent to opt.reset()
```
