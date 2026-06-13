# SPEC-009: Runtime API

Status: READY

Last validated by Ralph: never

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
