# SPEC-034: Parallel Controller Dispatch

Layer: Runtime

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-21

---

## Summary

This specification defines parallel execution of Controllers within a single tick.

Controllers that operate on disjoint sets of block types may execute concurrently without breaking determinism or requiring locks.

The Runtime remains responsible for ordering and synchronization. Controllers remain unaware of whether they execute sequentially or in parallel.

---

## Behavior

### Scenario: Controllers with disjoint type access execute in parallel

Given Controller A touches type_id = 1 and Controller B touches type_id = 2

When the Runtime executes a tick

Then A and B may execute concurrently

**Test:** `test_parallel.h` — `parallel_disjoint_types`

### Scenario: Controllers with overlapping type access execute sequentially

Given Controller A and Controller B both touch type_id = 1

When the Runtime executes a tick

Then A and B execute sequentially (order preserved by priority)

**Test:** `test_parallel.h` — `parallel_overlapping_types_sequential`

### Scenario: Determinism is preserved regardless of parallelism

Given a set of controllers with declared type access

When the same input is processed twice with parallel dispatch

Then both executions produce identical state

**Test:** `test_parallel.h` — `parallel_determinism`

### Scenario: Controller declares type access at registration

Given `register_controller(ctrl, priority, {1, 2})` declares access to type_ids 1 and 2

When the Runtime plans execution

Then it uses this declaration to determine parallelizability

**Test:** `test_parallel.h` — `parallel_type_declaration`

### Scenario: Empty declaration defaults to sequential

Given a controller registered without type access declaration

When the Runtime executes

Then the controller runs sequentially (conservative default)

**Test:** `test_parallel.h` — `parallel_empty_declaration_sequential`

### Scenario: Commands produced by parallel controllers are merged deterministically

Given Controller A produces create(1) and Controller B produces set_field(1, ...) in the same tick

When both run in parallel

Then both commands are submitted to the scheduler and executed in FIFO order

**Test:** `test_parallel.h` — `parallel_command_merge`

---

## Constraints

* Parallel execution must not change observable tick results.
  * **Test verification:** `test_parallel.h` — `parallel_same_result_as_sequential`
* Controller type access declarations must not be enforced at runtime for reads — only for scheduling.
  * **Test verification:** `test_parallel.h` — `parallel_declaration_is_scheduling_hint`
* The Runtime must fall back to sequential execution if parallel dispatch is not supported by the platform.
  * **Test verification:** `test_parallel.h` — `parallel_fallback_sequential`

---

## Out of Scope

* Lock-free data structures.
* GPU compute or SIMD dispatch.
* Dynamic repartitioning mid-tick.
* Nested parallelism (controllers spawning sub-controllers).

---

## Open Questions

* [x] How does the Runtime know which block types a Controller accesses?

**Answer:** The Controller declares its type access at registration via `register_controller(ctrl, priority, {type_ids})`. If no declaration is provided, the Controller runs sequentially (conservative default).

* [x] How are parallel execution errors reported?

**Answer:** Each parallel group produces its own `TickResult` fragment. These are merged after all groups complete, following the same rules as the sequential path (worst status wins, rejected commands are concatenated).

* [x] What thread pool / job system is used?

**Answer:** The Runtime uses a lightweight single-threaded fallback by default. A parallel backend (e.g., `std::thread` pool) can be injected via `RuntimeConfig`. The default backend is always sequential.

---

## Definitions

### Parallel Group

A set of Controllers whose declared type access sets are pairwise disjoint. These may execute concurrently.

### Type Access Declaration

A set of `uint32_t` type_ids provided when registering a Controller, indicating which block types the Controller may access.

### Deterministic Merge

The process of combining results from parallel groups in a fixed, repeatable order to produce the final TickResult.
