# SPEC-028: GameAK Fluent Queries

Layer: Runtime

Status: DRAFT

Last validated by Ralph: never

---

## Summary

Provide an ActiveRecord-style fluent query interface for blocks. The developer expresses block queries as a chain of filter/transform operations: `rt.blocks().of_type("Player").where(pred).count()`.

---

## Behavior

### Scenario: count blocks of a type

Given a Runtime with 5 blocks of type "Player"

When the user calls:

```cpp
size_t n = rt.blocks().of_type("Player").count();
```

Then `n == 5`.

**Test:** `test_runtime.cpp` — `blocks_query_count_by_name`

### Scenario: count blocks by numeric type_id

When the user calls:

```cpp
size_t n = rt.blocks().of_type(1).count();
```

Then `n` is the number of blocks with `type_id == 1`.

**Test:** `test_runtime.cpp` — `blocks_query_count_by_type_id`

### Scenario: filter then map

When the user calls:

```cpp
auto ids = rt.blocks()
    .of_type("Player")
    .where([](const DataBlock& b) { return b.field<int>(0) > 0; })
    .map([](const DataBlock& b) { return b.identity(); });
```

Then `ids` contains identities of Player blocks whose first field is positive.

**Test:** `test_runtime.cpp` — `blocks_query_filter_then_map`

### Scenario: iterate with each

When the user calls:

```cpp
rt.blocks().of_type("Player").each([](const DataBlock& b) {
    total += b.field<int>(0);
});
```

Then `total` accumulates the first field of all Player blocks.

**Test:** `test_runtime.cpp` — `blocks_query_each`

### Scenario: any / all predicates

When the user calls:

```cpp
bool has_any = rt.blocks().of_type("Player").any(pred);
bool has_all = rt.blocks().of_type("Player").all(pred);
```

Then `any` returns `true` if any block matches `pred`, `all` returns `true` if all match.

**Test:** `test_runtime.cpp` — `blocks_query_any_all`

### Scenario: first matching block

When the user calls:

```cpp
auto* block = rt.blocks().of_type("Player").first(pred);
```

Then `block` points to the first DataBlock matching `pred`, or `nullptr` if none match.

**Test:** `test_runtime.cpp` — `blocks_query_first`

### Scenario: chaining without type filter

When the user calls:

```cpp
auto n = rt.blocks().count();
```

Then `n` is the total number of blocks of all types.

**Test:** `test_runtime.cpp` — `blocks_query_count_all`

### Scenario: empty query returns zero

When no blocks match the criteria, terminal methods return empty results:

```cpp
rt.blocks().of_type("NonExistent").count()   == 0;
rt.blocks().of_type("NonExistent").any(pred) == false;
rt.blocks().of_type("NonExistent").first(pred) == nullptr;
```

**Test:** `test_runtime.cpp` — `blocks_query_empty`

---

## Constraints

* `BlockQuery` is lazy: predicates accumulate, execution only happens on terminal methods.
* `BlockQuery` is a value type (copyable, movable).
* `.of_type()` accepts both `uint32_t type_id` and `const char* name`.
* `.where()` accepts `std::function<bool(const DataBlock&)>` or a lambda.
* Terminal methods:
  * `.count()` → `size_t`
  * `.map<U>(f)` → `std::vector<U>` where `f: DataBlock → U`
  * `.each(f)` → `void` where `f: DataBlock → void`
  * `.any(pred)` → `bool`
  * `.all(pred)` → `bool`
  * `.first(pred)` → `const DataBlock*`
* Intermediate methods return `BlockQuery&` allowing further chaining.
* `BlockQuery` stores a const reference to the block map (no copies of blocks).

---

## Out of Scope

* Sorting / ordering results.
* Joins or cross-type queries.
* Lazy streaming (all results materialized on terminal call).
* Pagination (`limit`, `offset`).
* Modifying blocks during iteration (read-only query).

---

## Definitions

### Lazy Query

A query that accumulates filter predicates without executing them. Execution is deferred until a terminal method (`.count()`, `.map()`, `.each()`, etc.) is called. This allows the implementation to optimize iteration order and short-circuit where possible.

### Terminal Method

A method that triggers query execution and produces a result. After calling a terminal method, the `BlockQuery` is still valid for further queries (it does not consume itself).

```cpp
class BlockQuery {
public:
    BlockQuery& of_type(uint32_t type_id);
    BlockQuery& of_type(const char* name);
    BlockQuery& where(std::function<bool(const DataBlock&)> pred);

    // Terminals
    size_t count() const;
    template <typename U> std::vector<U> map(std::function<U(const DataBlock&)> f) const;
    void each(std::function<void(const DataBlock&)> f) const;
    bool any(std::function<bool(const DataBlock&)> pred) const;
    bool all(std::function<bool(const DataBlock&)> pred) const;
    const DataBlock* first(std::function<bool(const DataBlock&)> pred) const;

private:
    const std::unordered_map<core::Identity, DataBlock>* blocks_;
    uint32_t filter_type_{0};
    bool has_type_filter_{false};
    std::function<bool(const DataBlock&)> filter_pred_;
};
```
