# SPEC-032: GameAK Conversational Callbacks

Layer: Runtime

Status: DRAFT

Last validated by Ralph: never

---

## Summary

Provide Rails-style temporal callback naming: `rt.before_tick(...)`, `rt.after_tick(...)`, `rt.when_block_created(...)`. The method name tells you *when* the callback fires.

---

## Behavior

### Scenario: register before_tick callback

When the user calls:

```cpp
int setup_count = 0;
rt.before_tick([&] {
    setup_count++;
});
rt.tick();
```

Then the callback is executed once during `tick()` (immediately after `TickBegin` event fires), and `setup_count == 1`.

**Test:** `test_runtime.cpp` — `before_tick_callback_fires`

### Scenario: register after_tick callback with TickResult

When the user calls:

```cpp
TickResult captured;
rt.after_tick([&](const TickResult& result) {
    captured = result;
});
rt.tick();
```

Then the callback receives the `TickResult` from the tick that just completed, and `captured.commands_executed` reflects the tick's execution.

**Test:** `test_runtime.cpp` — `after_tick_callback_receives_result`

### Scenario: register block created callback

When the user calls:

```cpp
int created_count = 0;
rt.when_block_created([&](core::Identity id, uint32_t type_id) {
    created_count++;
});
auto id = rt.create_block(1).value();
rt.tick();
```

Then the callback fires for each block created via command during the tick. `created_count == 1`.

**Test:** `test_runtime.cpp` — `when_block_created_callback_fires`

### Scenario: register block destroyed callback

When the user calls:

```cpp
int destroyed_count = 0;
rt.when_block_destroyed([&](core::Identity id) {
    destroyed_count++;
});
```

Then the callback fires for each block destroyed via command during the tick.

**Test:** `test_runtime.cpp` — `when_block_destroyed_callback_fires`

### Scenario: multiple callbacks on same event

When the user registers two `before_tick` callbacks:

```cpp
int a = 0, b = 0;
rt.before_tick([&] { a++; });
rt.before_tick([&] { b++; });
rt.tick();
```

Then both callbacks fire (order matches registration order), and `a == 1 && b == 1`.

**Test:** `test_runtime.cpp` — `before_tick_multiple_callbacks`

### Scenario: unlisten returns EventId

When the user calls:

```cpp
auto id = rt.before_tick(handler);
rt.unlisten(id);
```

Then the handler is removed and will not fire on subsequent ticks.

**Test:** `test_runtime.cpp` — `callback_unlisten`

---

## Constraints

* Callback registration methods delegate to the internal `EventBus`.
* `before_tick(handler)` signature: `handler` is `void()` or `std::function<void()>`.
* `after_tick(handler)` signature: `handler` is `void(const TickResult&)`.
* `when_block_created(handler)` signature: `handler` is `void(core::Identity, uint32_t)`.
* `when_block_destroyed(handler)` signature: `handler` is `void(core::Identity)`.
* All registration methods return `EventId` (uint64_t) for `unlisten()`.
* `before_tick` fires after `TickBegin` event (callbacks are additional handlers).
* `after_tick` fires before `TickEnd` event.
* The existing `on_tick_begin()` / `on_tick_end()` / `on_block_created()` / `on_block_destroyed()` methods remain available for backward compatibility.

---

## Out of Scope

* One-shot callbacks (auto-unregister after first fire).
* Callback priority or ordering (callbacks fire in registration order).
* Async or deferred callbacks.
* Callback removal by handler identity comparison (use `EventId`).

---

## Definitions

### Temporal Callback

A callback whose method name indicates *when* in the tick lifecycle it executes. Prefixes `before_`, `after_`, and `when_` serve as temporal markers.

| Method | Fires |
|--------|-------|
| `before_tick` | After TickBegin event, before Controller execution |
| `after_tick` | After command processing, before TickEnd event |
| `when_block_created` | After a block is created via command during tick |
| `when_block_destroyed` | After a block is destroyed via command during tick |
