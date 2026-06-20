# SPEC-027: GameAK Block Type DSL

Layer: Runtime

Status: DRAFT

Last validated by Ralph: never

---

## Summary

Replace manual `BlockTypeDescriptor` construction with a fluent builder. The developer declares block type structure using natural language: `rt.define("Player").has("x", &Player::x).done()`.

---

## Behavior

### Scenario: define a block type with fields

Given a `Runtime` and a C++ struct `Player { int x; int y; int hp; };`

When the user calls:

```cpp
rt.define("Player")
    .size_of<Player>()
    .has("x", &Player::x)
    .has("y", &Player::y)
    .has("hp", &Player::hp)
    .done();
```

Then a block type is registered with:
- `name = "Player"`
- `size = sizeof(Player)`
- `alignment = alignof(Player)`
- Three fields inferred via `offsetof` + `sizeof`
- Return type is `Result<void>` (success)

**Test:** `test_runtime.cpp` — `define_block_type_with_fields`

### Scenario: define with minimal info (auto-deduced)

When the user calls:

```cpp
rt.define<Player>("Player").done();
```

Then a block type is registered with `size = sizeof(Player)`, `alignment = alignof(Player)`, no explicit fields.

**Test:** `test_runtime.cpp` — `define_block_type_minimal`

### Scenario: define with explicit size and fields

When the user calls:

```cpp
rt.define("Score").size(4).align(4).has("value", 0, 4).done();
```

Then a block type is registered with exactly one field at offset 0 with size 4.

**Test:** `test_runtime.cpp` — `define_block_type_explicit`

### Scenario: duplicate name returns error

When the user defines the same block type name (or `type_id`) twice:

```cpp
rt.define<Player>("Player").done();
rt.define<Player>("Player").done(); // second call
```

Then the second call returns an error with `ErrorCode::DuplicateRegistration`.

**Test:** `test_runtime.cpp` — `define_block_type_duplicate_rejected`

### Scenario: ephemeral blocks

When the user calls:

```cpp
rt.define("Temp").size(4).align(4).ephemeral().done();
```

Then the registered block type has `ephemeral = true`.

**Test:** `test_runtime.cpp` — `define_ephemeral_block_type`

### Scenario: semantic constraint

When the user calls:

```cpp
rt.define("Health").size(0).semantic(core::SemanticConstraint::range(0, 100)).done();
```

Then the size is inferred from the semantic constraint (1 byte for range 0-100).

**Test:** `test_runtime.cpp` — `define_with_semantic_infers_size`

---

## Constraints

* `.done()` returns `Result<void>` to allow error propagation in `and_then` chains.
* After `.done()` is called, the builder is in a moved-from state (not reusable).
* `.size_of<T>()` sets both `size` and `alignment` from `T` using `sizeof` and `alignof`.
* `.has(name, member_ptr)` uses `offsetof` + `sizeof` via template metaprogramming to infer field offset and size.
* `.has(name, offset, size)` is the explicit variant (no template deduction).
* `.semantic(const SemanticConstraint&)` sets the semantic constraint; if `size == 0` it will be inferred.
* `.ephemeral()` sets the ephemeral flag to `true`.
* The builder auto-assigns `type_id` from an incrementing counter (collision-free per Runtime instance), OR the developer can call `.id(n)` to specify explicitly.

---

## Out of Scope

* Defining blocks with AoSoA chunk size overrides in the DSL (use `BlockTypeDescriptor` directly for now).
* Removing or modifying a block type after definition.
* Defining block types with no size and no semantic constraint (must have at least one).

---

## Open Questions

* [ ] Should `.done()` support an `&&`-qualified overload that is the only valid path (rvalue-only builder)?
* [ ] Should `define()` return a `BlockTypeBuilder` by value on the stack, or via `std::unique_ptr`?

---

## Definitions

### Member Pointer DSL

The pattern `.has("name", &Struct::member)` uses a pointer-to-member type to deduce field offset and size at compile time:

```cpp
template <typename T, typename U>
BlockTypeBuilder& has(const char* name, U T::*member) {
    FieldDescriptor fd;
    fd.name = name;
    fd.offset = reinterpret_cast<size_t>(&(static_cast<T*>(nullptr)->*member));
    fd.size = sizeof(U);
    fd.alignment = alignof(U);
    fields_.push_back(fd);
    return *this;
}
```
