# SPEC-009: Runtime API

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-13

---

## Summary

This specification defines the public API surface of the GameAK Runtime.

The Runtime API is the entry point through which consumers interact with simulation state, controllers, commands, and queries.

---

## Core Requirements

### Runtime Creation

A Runtime instance must be created through an explicit creation API.

Creation must accept a configuration object.

A Runtime instance must be destroyed through an explicit destruction API.

### Execution Tick Ordering

Within a single `tick()`, execution proceeds in the following order:

1. **Controller Execution Phase**: All registered Controllers are executed. Controllers read the current state and produce Commands.
2. **Command Processing Phase**: All pending Commands (including those just produced by Controllers) are validated and executed by the Scheduler.
3. **Result Reporting**: A TickResult is produced summarizing the tick execution.

Commands submitted between ticks (via `submit_command()`) are queued and processed during the next `tick()`. This ordering guarantees deterministic execution: given the same state and same Controllers, the same Commands are produced and applied in the same order.

### Pause / Resume

The Runtime must provide an API for pausing and resuming tick execution.

When paused:
- `tick()` returns immediately with an empty TickResult (all fields zero).
- No Controllers are executed and no Commands are processed.
- Commands submitted while paused are queued and will be processed after resume.

**Test:** `test_runtime.cpp` — `pause skips tick execution`, `resume allows tick execution after pause`, `paused runtime still accepts commands but does not process them`

### Fixed Timestep

The Runtime must provide an API for configuring a fixed timestep.

When a fixed timestep is set:
- `tick(time_delta)` accumulates the delta into an internal accumulator.
- As long as the accumulator is greater than or equal to the fixed timestep, sub-ticks are executed using the fixed timestep as the delta passed to Controllers via `StateView::time_delta()`.
- Each sub-tick executes all Controllers and processes all pending Commands.
- Combined results across sub-ticks are returned with summed counters and worst-status propagation.

The fixed timestep can be cleared to revert to variable timestep mode.

```cpp
void set_fixed_timestep(float dt);
void clear_fixed_timestep();
float fixed_timestep() const;
```

**Tests:** `test_runtime.cpp` — `fixed timestep accumulates and runs multiple sub-ticks`, `fixed timestep sub-ticks see the fixed delta, not accumulated delta`, `clear_fixed_timestep reverts to variable timestep`

### Data Block Lifecycle

The Runtime must provide an API for creating Data Blocks.

The Runtime must provide an API for destroying Data Blocks.

Both operations must accept a Data Block type identifier.

Both operations must return a Result type.

### Command Submission

The Runtime must provide an API for submitting Runtime Commands.

Submission must accept an immutable Command.

Submission must return a Result type.

### Execution Tick

The Runtime must provide an API for advancing simulation state.

The tick function must process pending Commands.

The tick function must execute registered Controllers.

The tick function must return execution results.

### Query Operations

The Runtime must provide an API for checking Data Block existence by identity.

The Runtime must provide an API for counting Data Blocks by type.

The Runtime must provide an API for finding blocks by type identifier:
```cpp
std::vector<core::Identity> find_blocks_by_type(uint32_t type_id) const;
```

The Runtime must provide an API for finding blocks by arbitrary predicate:
```cpp
std::vector<core::Identity> find_blocks(
    std::function<bool(const DataBlock&)> pred) const;
```

**Tests:** `test_runtime.cpp` — `find_blocks_by_type returns blocks of matching type`, `find_blocks with predicate filters correctly`, `find_blocks returns empty when nothing matches`, `find_blocks returns all blocks when predicate always true`

### Serialization (Snapshot)

The Runtime must provide an API for capturing the full simulation state and restoring it later.

The `Snapshot` struct contains:
- All Data Blocks (identity, type_id, raw data)
- All registered Block Type Descriptors
- The next identity counter

```cpp
struct Snapshot {
    std::unordered_map<core::Identity, DataBlock> blocks;
    std::unordered_map<uint32_t, BlockTypeDescriptor> types;
    uint64_t next_identity;
};

Snapshot save() const;
void load(const Snapshot& snapshot);
```

`load()` replaces all current state with the snapshot's state and rebuilds internal type counts.

The scheduler state (pending commands, history) is NOT captured in the snapshot.

**Tests:** `test_runtime.cpp` — `save captures current state`, `load restores previously saved state`, `load rebuilds type_counts`

### Block Relationships

The Runtime must provide an API for creating and querying parent-child relationships between blocks.

Relationships form a bidirectional graph:
```cpp
Result<void> relate(Identity parent, Identity child);
Result<void> unrelate(Identity parent, Identity child);
std::vector<Identity> children_of(Identity parent) const;
std::vector<Identity> parents_of(Identity child) const;
```

Constraints:
- Both identities must be valid and reference existing blocks.
- A block cannot be related to itself.
- Relationships are not automatically cleaned up when blocks are destroyed.

**Tests:** `test_runtime.cpp` — `relate creates parent-child edge`, `unrelate removes parent-child edge`, `multiple children per parent`, `multiple parents per child`, `relating self fails`, `relating nonexistent block fails`

### Type Registration

Data Block types must be registered with the Runtime before use.

Type registration must occur before creating Data Blocks of that type.

---

## Constraints

* Runtime API does not throw exceptions.
* All Runtime API functions return Result types.
* Runtime is single-threaded by default.
* Type registration is required before block creation.
* Runtime creation and destruction are explicit operations.

---

## Out of Scope

* Thread-safe API.
* Network-distributed Runtime.
* Persistent Runtime state.
* Hot-reloading.

---

## Open Questions

* [ ] Should Runtime be a concrete class or a handle-based API?

**Answer:** Runtime is a concrete class.

Consumers create a Runtime instance directly. The class provides RAII semantics through its constructor and destructor.

* [ ] What configuration does Runtime creation accept?

**Answer:** The configuration object accepts:

* Log level (optional, defaults to `warn`)

Configuration is intentionally minimal for the initial implementation. Future versions may introduce allocator customization and capacity hints.

Default configuration values:
* Log level: `warn`

* [ ] How are Controllers registered?

**Answer:** Controllers are registered through the Runtime API before execution.

Registration accepts a Controller and optional scheduling hints.

* [ ] How is a Data Block type defined?

**Answer:** A Data Block type is defined using a descriptor structure.

```cpp
struct BlockTypeDescriptor {
    uint32_t type_id;        // Unique type identifier
    size_t size;             // Size of the block data in bytes
    size_t alignment;        // Alignment requirement
    const char* name;        // Human-readable type name for diagnostics
};
```

Types are registered via `Runtime::register_block_type(descriptor)`.

Fixed-size Data Blocks use `size` directly. Variable-size Data Blocks specify a maximum size and use runtime-managed storage.

The `type_id` must be unique within a Runtime instance. Using a reserved or duplicate `type_id` produces a `DuplicateRegistration` error.

* [ ] What does `tick()` return?

**Answer:** `tick()` returns a `TickResult` containing:

* Number of commands executed
* Number of commands rejected
* Number of controllers executed
* Execution status

The `ExecutionStatus` is an enumeration:

```cpp
enum class ExecutionStatus : uint32_t {
    Success,
    PartialFailure,  // some commands rejected, some executed
    CriticalFailure, // tick could not complete
};
```

A tick with all commands applied successfully returns `Success`. A tick where some commands were rejected returns `PartialFailure`. A tick that could not run (e.g., internal error) returns `CriticalFailure`.

---

## Definitions

### Runtime Instance

A concrete object that owns simulation state and coordinates execution.

### Tick

A single execution cycle of the Runtime.

### Type Registration

The process of informing the Runtime about a Data Block type before use.
