# SPEC-031: GameAK Relationship DSL

Layer: Runtime

Status: DRAFT

Last validated by Ralph: never

---

## Summary

Provide a conversational API for block relationships. Instead of `rt.relate(parent, child)`, the developer writes `rt.relate(parent).to(child)` — which reads like English.

---

## Behavior

### Scenario: relate parent to child

Given a Runtime with valid block identities `parent_id` and `child_id`

When the user calls:

```cpp
auto result = rt.relate(parent_id).to(child_id);
```

Then a parent-child relationship is established. The call returns `Result<void>` (success on valid identities).

**Test:** `test_runtime.cpp` — `relate_conversational`

### Scenario: unrelate parent from child

When the user calls:

```cpp
auto result = rt.unrelate(parent_id).from(child_id);
```

Then the parent-child relationship is removed.

**Test:** `test_runtime.cpp` — `unrelate_conversational`

### Scenario: invalid parent identity returns error

When the user calls:

```cpp
core::Identity invalid;
auto result = rt.relate(invalid).to(child_id);
```

Then `result` contains `ErrorCode::InvalidIdentity`.

**Test:** `test_runtime.cpp` — `relate_invalid_parent`

### Scenario: invalid child identity returns error

When the user calls:

```cpp
core::Identity invalid;
auto result = rt.relate(parent_id).to(invalid);
```

Then `result` contains `ErrorCode::InvalidIdentity`.

**Test:** `test_runtime.cpp` — `relate_invalid_child`

---

## Constraints

* `rt.relate(identity)` returns a `RelationshipBuilder` temporary object.
* `RelationshipBuilder::to(identity)` performs the actual operation and returns `Result<void>`.
* `rt.unrelate(identity)` returns an `UnrelateBuilder`.
* `UnrelateBuilder::from(identity)` performs the removal and returns `Result<void>`.
* Both builders validate identities at the `to()` / `from()` call (not at construction).
* The intermediate builder objects are not stored (they exist only for the method chain).
* The existing `rt.relate(parent, child)` overload remains available for backward compatibility.

---

## Out of Scope

* Named relationships (e.g., `.as("inventory")`).
* Relationship cardinality constraints.
* Cascading deletes.
* Querying by relationship name.

---

## Definitions

### RelationshipBuilder

A temporary object returned by `Runtime::relate()` that exposes `.to()` to complete the relationship:

```cpp
class RelationshipBuilder {
    Runtime* rt_;
    core::Identity parent_;
public:
    explicit RelationshipBuilder(Runtime* rt, core::Identity parent)
        : rt_{rt}, parent_{parent} {}
    core::Result<void> to(core::Identity child) {
        return rt_->relate(parent_, child);
    }
};
```

Because `RelationshipBuilder` is a temporary, the chain `rt.relate(parent).to(child)` reads as a single sentence. The builder cannot be stored and reused (no public constructor).
