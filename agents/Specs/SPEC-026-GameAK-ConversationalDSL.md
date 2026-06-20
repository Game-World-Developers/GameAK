# SPEC-026: GameAK Conversational DSL

Layer: Runtime

Status: DRAFT

Last validated by Ralph: never

---

## Summary

GameAK adopts a conversational DSL philosophy inspired by Ruby on Rails. Every public API method should read like an English sentence. The developer expresses intent; the framework handles mechanics.

This spec is the **master** specification. It defines the overarching principles. Individual specs (SPEC-027 to SPEC-032) detail each pattern.

---

## Core Requirements

### Principle 1: Verbs, not Nouns

Public API methods are imperative verbs. The developer tells the framework *what to do*, not *what to configure*.

```cpp
// Good (conversational):
rt.define("Player")
    .has("x", &Player::x)
    .done();

// Avoid (procedural):
BlockTypeDescriptor desc;
desc.type_id = 1;
desc.size = sizeof(Player);
rt.register_block_type(desc);
```

### Principle 2: Fluent Chaining

Builder methods return `*this` to enable method chaining. Terminal methods (`.done()`, `.build()`) finalize the chain and produce the result.

```cpp
auto fsm = Fsm<>{}
    .initial_state("idle")
    .add_transition("idle", "start", "attacking")
    .build();
```

### Principle 3: Convention over Configuration

Sensible defaults minimize required parameters. The developer only specifies what differs from the convention.

```cpp
// Default time delta (60 fps), default scheduler (Fifo), default log level (Warn)
Runtime rt;

// Override only what's needed
auto rt2 = Runtime<>::configure()
    .log_level(LogLevel::Info)
    .build();
```

### Principle 4: Error Handling as Conversation

`Result<T>` supports monadic chaining so error handling reads as a natural progression, not an interruption.

```cpp
producer.create(1)
    .and_then([&](Identity id) { return producer.set(id, 0, 42); })
    .or_else([](Error e) { SPDLOG_WARN("Failed: {}", e.message()); });
```

### Principle 5: Relationships as Prepositional Phrases

Relationships read as "relate X to Y", not "relate(X, Y)".

```cpp
rt.relate(parent).to(child);
rt.unrelate(parent).from(child);
```

### Principle 6: Callbacks as Temporal Markers

Callbacks use temporal prepositions (`before`, `after`, `when`) to describe *when* they execute.

```cpp
rt.before_tick([&] { /* setup */ });
rt.after_tick([&](const TickResult& r) { /* cleanup */ });
rt.when_block_created([](Identity id, uint32_t t) { /* react */ });
```

---

## Constraints

* All builder methods return `*this` by reference (`T&`).
* Terminal methods (`.done()`, `.build()`) consume the builder (move semantics).
* Method names use `snake_case` (C++ convention).
* No macros are required for the DSL (except the optional `TRY` for `Result<T>`).
* The DSL is additive — existing non-DSL APIs remain functional.

---

## Related Specs

| Spec | Domain |
|------|--------|
| SPEC-027 | Block Type DSL (`define`, `has`, `done`) |
| SPEC-028 | Fluent Queries (`blocks().of_type().where().count()`) |
| SPEC-029 | Monadic Result (`and_then`, `or_else`, `TRY`) |
| SPEC-030 | Config Builder (`configure().log_level().build()`) |
| SPEC-031 | Relationship DSL (`relate(x).to(y)`) |
| SPEC-032 | Conversational Callbacks (`before_tick`, `after_tick`) |

---

## Definitions

### Conversational DSL

An API design philosophy where method names, parameter order, and chaining patterns mimic natural English sentences. The developer writes code that reads like instructions to a colleague.

### Builder

An intermediate object returned by a DSL method that accumulates configuration and provides a terminal method (`.done()`, `.build()`) to produce the final result.

### Terminal Method

A method that ends a fluent chain and produces a side effect or return value. After calling a terminal method, the builder is typically consumed (move semantics) and cannot be reused.

