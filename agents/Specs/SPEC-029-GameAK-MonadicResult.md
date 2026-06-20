# SPEC-029: GameAK Monadic Result

Layer: Foundation

Status: DRAFT

Last validated by Ralph: never

---

## Summary

Add monadic chaining to `Result<T>` so that sequences of fallible operations compose naturally: `r.and_then(f).and_then(g).or_else(h)`.

---

## Behavior

### Scenario: chain on success with and_then (same type)

Given a `Result<Identity>` that holds a valid identity

When the user calls:

```cpp
Result<void> r = rt.create_block(1).and_then([&](Identity id) {
    return rt.destroy_block(id);
});
```

Then the lambda is called with the identity, `destroy_block` is executed, and `r` holds the result of `destroy_block`.

**Test:** `test_result.h` — `result_and_then_chains_on_success`

### Scenario: and_then short-circuits on error

Given a `Result<Identity>` that holds an error

When the user calls:

```cpp
Result<void> r = rt.create_block(99).and_then([&](Identity id) {
    return rt.destroy_block(id); // never called
});
```

Then the lambda is NOT called, and `r` holds the original error from `create_block(99)`.

**Test:** `test_result.h` — `result_and_then_short_circuits_on_error`

### Scenario: and_then with type change

When the user calls:

```cpp
Result<int> r = rt.create_block(1).and_then([](Identity id) -> Result<int> {
    return 42; // wraps in Result<int>
});
```

Then `r.value() == 42`.

**Test:** `test_result.h` — `result_and_then_changes_type`

### Scenario: or_else handles error

Given a `Result<T>` in error state

When the user calls:

```cpp
bool handled = false;
rt.create_block(99).or_else([&](const Error& e) {
    handled = true;
    expect(e.code() == ErrorCode::TypeNotRegistered).toBeTruthy();
});
```

Then the lambda is called with the error details and `handled == true`.

**Test:** `test_result.h` — `result_or_else_handles_error`

### Scenario: or_else does not fire on success

Given a `Result<T>` in success state

When the user calls:

```cpp
bool handled = false;
rt.create_block(1).or_else([&](const Error&) {
    handled = true; // never called
});
```

Then the lambda is NOT called and `handled == false`.

**Test:** `test_result.h` — `result_or_else_not_called_on_success`

### Scenario: TRY macro for early return

When the user writes:

```cpp
core::Result<void> do_stuff(Runtime<>& rt) {
    TRY(auto id, rt.create_block(1));
    TRY(rt.destroy_block(id));
    return {};
}
```

Then on success `id` is bound and execution continues. On failure at any `TRY`, the function returns the error immediately.

**Test:** `test_result.h` — `result_try_macro_early_return`

---

## Constraints

* `and_then` is a template method: `template<typename F> auto and_then(F&& f) -> Result<decltype(f(value()))>`.
* `and_then` is only callable on rvalue `Result&&` (consumes the result).
* `or_else` returns `void` (side-effect only).
* `or_else` is callable on `const Result&` (does not consume).
* `TRY` macro uses a `do { ... } while(0)` pattern:

```cpp
#define TRY(var, expr) \
    if (auto _r = (expr); !_r) { return _r.error(); } \
    else { var = std::move(_r.value()); }
```

* `TRY` without variable binding:

```cpp
#define TRY(expr) \
    if (auto _r = (expr); !_r) { return _r.error(); }
```

---

## Out of Scope

* `map` / `map_error` methods (deferred; `and_then` covers the map case).
* `Result<void>::and_then` specialization (default `and_then` on void works via the template).
* `Option<T>` / `Maybe<T>` type.
* Exception-to-Result conversion utilities.

---

## Open Questions

* [ ] Should `and_then` be available on `const Result&` (non-consuming) or only on `&&`?
* [ ] Should we add `map` as a pure transform (non-fallible lambda) separate from `and_then` (fallible lambda)?

---

## Definitions

### Monadic Chain

A sequence of operations where each step receives the success value of the previous step, and the chain short-circuits on the first error. This eliminates nested `if (!result)` checks.

### TRY Macro

A macro that unwraps a `Result<T>` on success (binding the value) or returns the error on failure. Modeled after Rust's `?` operator and Zig's `try`.
