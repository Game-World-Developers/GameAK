# SPEC-010: Controller API

**Validation timestamp:** 2026-06-13

**Verdict:** READY / IMPLEMENTED

## Assessment

SPEC-010 defines the Controller interface. All original open questions have been answered. The spec has been extended with Controller Priorities.

## What Changed Since Previous Validation

### Previously Answered: 4 Questions — All Resolved

| # | Question | Answer | Status |
|---|----------|--------|--------|
| 1 | Can a Controller fail? | Yes. Failures reported through Result return type. | ✅ |
| 2 | Can a Controller produce multiple Commands? | Yes. Zero or more per execution. | ✅ |
| 3 | Can a Controller read state from other Controllers? | No. Controllers read from Runtime through state view. | ✅ |
| 4 | Can a Controller access the time delta? | Yes. State view includes time delta value. | ✅ |

### New Feature Added (Post-Validation)

| Feature | Spec Section | Tests |
|---------|-------------|-------|
| **Controller Priorities** | Priorities | `controllers execute in priority order`, `controllers with equal priority preserve registration order`, `single-arg register_controller uses default priority 0` |

### Priority Semantics

- Controllers may be registered with an integer priority via `register_controller(Controller, int)`.
- Controllers with a single argument default to priority 0.
- Higher priorities execute first.
- Equal priorities preserve registration order (stable sort).
- Sorting by priority happens before each tick's Controller Execution Phase.

## Minor Considerations

- **StateView and CommandProducer types are not fully defined in the spec** — their exact API is implicit. This is acceptable because their purpose is clear and they are exercised in tests.
- **Controller's Result value type** — the success value is `void` (the Controller communicates state changes through Commands, not return values).

## Open Questions

None. All questions are answered.

## Final Determination

**SPEC-010 is READY and IMPLEMENTED.** All requirements are covered by passing tests.
