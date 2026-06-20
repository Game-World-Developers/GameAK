# Verdict: SPEC-030

**Spec:** GameAK Config Builder
**Validation timestamp:** 2026-06-20
**Verdict:** READY
**Layer:** Runtime

---

## Assessment

All 5 behaviors are clearly specified. The `RuntimeBuilder` pattern is well-defined.

## Open Questions

1. **`scheduler<S>()` type change:** Changing the scheduler template parameter mid-chain changes the builder's type (`RuntimeBuilder<FifoScheduler>` → `RuntimeBuilder<PriorityScheduler>`). Decision: Return a new `RuntimeBuilder<S>` from `.scheduler<S>()` (covariant return via move-construct). The default is `RuntimeBuilder<FifoScheduler>`.

2. **`Runtime::configure()` return type:** Must return `RuntimeBuilder<FifoScheduler>` (the default). Decision: `configure()` is a static method on `Runtime`, returns `RuntimeBuilder<FifoScheduler>`.

3. **Builder consumed after build:** `build()` is `&&`-qualified. Decision: The builder is a value type but `build() &&` prevents reuse. A moved-from builder can exist but calling any method on it (except destructor) is UB.

## Notes

* The builder stores a `RuntimeConfig` and the scheduler type tag.
* `.build()` template instantiates the correct `Runtime<SchedulerType>`.
* The existing `Runtime(RuntimeConfig)` constructor remains for backward compatibility.

## Next Steps

Implement `RuntimeBuilder` class and `Runtime::configure()` static factory.
