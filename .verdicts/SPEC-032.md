# Verdict: SPEC-032

**Spec:** GameAK Conversational Callbacks
**Validation timestamp:** 2026-06-20
**Verdict:** READY
**Layer:** Runtime

---

## Assessment

All 5 behaviors are clearly specified. The callback pattern is simple and well-defined.

## Open Questions

None.

## Notes

* `before_tick(handler)` delegates to `EventBus::on_tick_begin()` — handler is `void()`, NOT `void(const Event&)`.
* `after_tick(handler)` needs new EventBus type or a wrapper that captures TickResult. Decision: `after_tick` wraps the handler and internally listens to `TickEnd`, but provides the `TickResult` to the user's handler. The Runtime stores the TickResult from the most recent tick for this purpose.
* `when_block_created` and `when_block_destroyed` delegate to existing EventBus events.
* All methods return `EventId` for `unlisten()`.
* Existing `on_tick_begin()`, `on_tick_end()`, `on_block_created()`, `on_block_destroyed()` remain for backward compatibility.

## Next Steps

Add `before_tick`, `after_tick`, `when_block_created`, `when_block_destroyed` methods to `Runtime`.
