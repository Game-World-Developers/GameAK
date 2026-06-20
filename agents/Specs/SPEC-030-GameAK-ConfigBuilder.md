# SPEC-030: GameAK Config Builder

Layer: Runtime

Status: DRAFT

Last validated by Ralph: never

---

## Summary

Replace direct `RuntimeConfig` struct construction with a fluent builder. The developer configures the `Runtime` via a conversational chain: `Runtime<>::configure().log_level(Info).fixed_timestep(1/60.f).build()`.

---

## Behavior

### Scenario: configure log level

When the user calls:

```cpp
auto rt = Runtime<>::configure()
    .log_level(LogLevel::Info)
    .build();
```

Then the Runtime is created with `config().log_level == LogLevel::Info`.

**Test:** `test_runtime.cpp` — `config_builder_log_level`

### Scenario: configure fixed timestep

When the user calls:

```cpp
auto rt = Runtime<>::configure()
    .fixed_timestep(1.0f / 60.0f)
    .build();
```

Then `rt.fixed_timestep() == 1.0f/60.0f`.

**Test:** `test_runtime.cpp` — `config_builder_fixed_timestep`

### Scenario: configure scheduler

When the user calls:

```cpp
auto rt = Runtime<>::configure()
    .scheduler<PriorityScheduler>()
    .build();

// Must compile and use PriorityScheduler internally
```

Then the Runtime uses `PriorityScheduler` instead of the default `FifoScheduler`.

**Test:** `test_runtime.cpp` — `config_builder_priority_scheduler`

### Scenario: all defaults

When the user calls:

```cpp
auto rt = Runtime<>::configure().build();
```

Then the Runtime is created with default `RuntimeConfig` (same as `Runtime<>{}`).

**Test:** `test_runtime.cpp` — `config_builder_defaults`

### Scenario: builder consumed after build

When the user calls:

```cpp
auto builder = Runtime<>::configure().log_level(LogLevel::Error);
auto rt = builder.build();
builder.build(); // compilation error (builder is moved-from)
```

Then the second `.build()` call should fail to compile (builder should be move-only or the method should be `&&`-qualified).

**Test:** `test_runtime.cpp` — `config_builder_consumed_after_build` (compile-time check)

---

## Constraints

* `RuntimeBuilder` is a separate class from `Runtime`.
* `RuntimeBuilder` is move-only (no copy).
* `.build()` is `&&`-qualified (only callable on rvalue builder).
* `.scheduler<S>()` sets the scheduler template parameter using CRTP or a type tag.
* `RuntimeBuilder` stores a `RuntimeConfig` internally and exposes chainable setters for each field.
* Method naming follows conversational style: `log_level()`, `fixed_timestep()`, `scheduler()`.

---

## Out of Scope

* Runtime construction with external block types pre-loaded (use `register_block_type` after `build()`).
* Builder validation beyond type checking (e.g., negative timestep is caught by Runtime).

---

## Open Questions

* [ ] Should `scheduler<S>()` return `RuntimeBuilder<S>` (changing the builder type mid-chain) or store a `std::function` factory?
* [ ] Is `configure()` a static method on `Runtime` or a free function?

---

## Definitions

### &&-qualified method

A method that can only be called on an rvalue (temporary) object:

```cpp
RuntimeBuilder&& build() &&;
```

This prevents calling `.build()` twice on the same builder instance because after the first call the object is in a moved-from state.
