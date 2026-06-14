# SPEC-017: Event System

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-017 defines the event system for the GameAK Runtime. The spec is clear, implementable, and has been implemented with 6 passing tests.

## Implemented API

```cpp
enum class EventType : uint32_t {
    TickBegin,
    TickEnd,
    BlockCreated,
    BlockDestroyed,
};

struct Event {
    EventType type;
    core::Identity identity;
    uint32_t block_type_id;
};

using EventHandler = std::function<void(const Event&)>;
using EventId = uint64_t;

EventId listen(EventType type, EventHandler handler);
void unlisten(EventId id);
```

## Test Coverage

| Scenario | Test | Status |
|----------|------|--------|
| TickBegin and TickEnd fire during tick | `test_runtime.cpp` | ✅ |
| BlockCreated fires on command-created block | `test_runtime.cpp` | ✅ |
| BlockDestroyed fires on command-destroyed block | `test_runtime.cpp` | ✅ |
| unlisten removes event handler | `test_runtime.cpp` | ✅ |
| Multiple handlers on same event type | `test_runtime.cpp` | ✅ |
| Fixed timestep fires events per sub-tick | `test_runtime.cpp` | ✅ |

## Implementation Notes

- Block events are detected by diffing the blocks map before and after command processing. This diff is only performed when at least one handler is registered for `BlockCreated` or `BlockDestroyed`, avoiding unnecessary copies when no handlers are active.
- Event handlers are stored per `EventType` in an `unordered_map<EventType, vector<pair<EventId, EventHandler>>>`.
- `fire_event()` is a private method that dispatches to all registered handlers for the given type.

## Open Questions

None. All aspects are clearly specified and implemented.

## Final Determination

**SPEC-017 is READY.**
