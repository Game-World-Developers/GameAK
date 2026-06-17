# SPEC-024: Ephemeral Data Blocks

Status: IMPLEMENTED

Last validated by Ralph: never

---

## Summary

This specification defines Ephemeral Data Blocks — a second class of Data Block with a single-frame lifetime.

Ephemeral Data Blocks live only during the current tick. They are created by Controllers via an `EphemeralProducer` (provided alongside `CommandProducer`), remain visible to all Controllers via standard queries during the same tick, and are automatically destroyed by the Runtime at TickEnd.

This replaces ad-hoc mechanisms (MessageBox, EventLoop timing tricks, intra-tick communication channels) with a single primitive: a Data Block that dies at end of frame.

---

## Core Requirements

### Ephemeral Flag

`BlockTypeDescriptor` gains an `ephemeral` flag:

```cpp
struct BlockTypeDescriptor {
    uint32_t type_id;
    size_t size;
    size_t alignment;
    const char* name;
    LayoutStrategy layout{LayoutStrategy::AoS};
    AoSoAConfig aosoa_config{};
    std::vector<FieldDescriptor> fields;
    bool ephemeral{false};              // NEW
};
```

When `ephemeral` is `true`, blocks of this type are destroyed automatically at the end of each tick.

---

### EphemeralProducer

A new class `EphemeralProducer` is provided to Controllers alongside `CommandProducer`.

```cpp
class EphemeralProducer {
public:
    using CreateFn = std::function<core::Identity(uint32_t type_id)>;

    explicit EphemeralProducer(CreateFn create_fn);

    /// Create an ephemeral block. The type must have ephemeral=true.
    core::Result<core::Identity> create(uint32_t type_id);
};
```

The Controller signature is updated to accept both:

```cpp
using Controller = std::function<core::Result<void>(
    StateView&, CommandProducer&, EphemeralProducer&)>;
```

---

### Lifecycle

1. **Creation**: A Controller calls `ephemeral.create(TYPE_ID)` during the Controller Execution Phase. The Runtime creates a Data Block with the given type and assigns an Identity. The block is stored in the same block map as persistent blocks.

2. **Visibility**: The ephemeral block is visible immediately to any Controller (including the creating Controller) via standard `StateView` queries (`find_blocks`, `find_blocks_by_type`, `has_block`). No command or flush is required.

3. **Destruction**: At the end of every `execute_single_tick()`, after the Command Processing Phase and event system, the Runtime destroys all Data Blocks whose type has `ephemeral == true`.

4. **Snapshots**: Ephemeral blocks are NOT included in `Runtime::save()` / `Runtime::load()`.

5. **Relationship**: Ephemeral blocks may participate in relationships (`relate`/`unrelate`) and commands during their lifetime. Relationships involving ephemeral blocks are not cleaned up automatically on destruction.

6. **Persistence Commands**: Controllers may produce Commands that read ephemeral block state and produce persistent blocks. This is the intended pattern for "spill to persistent":

```cpp
auto combat = [](StateView& view, CommandProducer& cmd, EphemeralProducer& ephem) -> Result<void> {
    auto id = *ephem.create(DAMAGE_SIGNAL);
    // write to id...
    // later
    for (auto& id : view.find_blocks_by_type(DAMAGE_SIGNAL)) {
        // produce persistent commands based on ephemeral data
        cmd.produce(Command::set_field(player_id, "hp", ...));
    }
    return {};
};
```

---

### No New Tick Phase

Ephemeral Data Blocks do not introduce a new tick phase. The tick proceeds as:

1. **Controller Execution Phase** — Controllers read state, create ephemeral blocks, produce Commands
2. **Block Snapshot** — The event system captures the block map snapshot *after* Controller Execution Phase. Ephemeral blocks created during phase 1 are present in both the snapshot and the current map, so no BlockCreated event fires for them.
3. **Command Processing Phase** — Commands (including those referencing ephemeral blocks) are validated and applied
4. **TickEnd** — Ephemeral blocks are destroyed automatically

This means the existing event system diff (introduced in SPEC-017) must move its `before_blocks` capture from before Controller Execution to after Controller Execution but before Command Processing.

---

## Constraints

* An ephemeral Data Block's type must have `ephemeral == true` in its `BlockTypeDescriptor`.
  * **Test verification:** `test_ephemeral.h` — `rejects create for non-ephemeral type`
* Ephemeral Data Blocks are destroyed at TickEnd, not at any other point.
  * **Test verification:** `test_ephemeral.h` — `ephemeral blocks are destroyed at tick end`
* Ephemeral Data Blocks are visible to all Controllers within the same tick they are created.
  * **Test verification:** `test_ephemeral.h` — `ephemeral visible to other controllers same tick`
* Ephemeral Data Blocks are not persisted in snapshots.
  * **Test verification:** `test_ephemeral.h` — `ephemeral not included in snapshot`
* Creating an ephemeral block does not require a Command — it is a direct Runtime operation via EphemeralProducer.
  * **Test verification:** `test_ephemeral.h` — `create ephemeral without command`
* The Controller signature gains a third parameter (`EphemeralProducer&`). All existing Controller callables must be updated.
  * **Test verification:** compilation — all existing tests compile with the new signature

---

## Out of Scope

* Automatic cleanup of relationships involving ephemeral blocks.
* Ephemeral blocks in SoA layout (may be added later).
* Ephemeral block event firing (BlockCreated/BlockDestroyed for ephemeral blocks).
* Nesting or stacking ephemeral lifetimes (they always die at TickEnd).

---

## Open Questions

* [ ] Should ephemeral blocks fire BlockCreated/BlockDestroyed events?

**Answer:** No. Ephemeral blocks are a high-frequency, intra-tick mechanism. Firing events for them would add overhead without benefit — Controllers that care about ephemeral blocks can query for them directly.

* [ ] Should ephemeral blocks support SoA layout?

**Answer:** Not initially. Ephemeral blocks are expected to be small and short-lived. AoS is sufficient. SoA may be added later if profiling shows a need.

* [ ] Can ephemeral blocks be created via Command?

**Answer:** No. Ephemeral blocks are created directly via `EphemeralProducer::create()` during the Controller Execution Phase. They exist only for the current tick and should not appear in the Command history.

* [ ] What happens if a Controller queries for ephemeral blocks before any are created?

**Answer:** The query returns an empty result, as with any query for a type with no blocks. No error.

---

## Definitions

### Ephemeral Data Block

A Data Block with `ephemeral == true` in its type descriptor. Lives only for one tick. Created directly by Controllers via `EphemeralProducer`.

### EphemeralProducer

An interface provided to Controllers for creating ephemeral Data Blocks during the Controller Execution Phase.

### Frame State

State that exists only during the current tick and is automatically cleaned up — the purpose of ephemeral Data Blocks.

### Persistent State

State that persists across ticks and is mutated via Commands — the original Data Block concept.
