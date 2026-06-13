# SPEC-006: Data Blocks

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-006 defines Data Blocks — the fundamental unit of simulation state. All 10 open questions have been answered clearly:

| Question | Answer |
|---|---|
| Is a Data Block typed? | Yes, type defines structure and semantics |
| Can it contain multiple fields? | Yes |
| Can it contain other Data Blocks? | No (relationships use identities) |
| Can it exist without an identity? | No |
| Fixed or variable size? | Both supported |
| Can representation change? | Yes, preserving observable behavior |
| Can layout change? | Yes, transparently |
| How created? | By Runtime via APIs |
| How destroyed? | By Runtime via APIs |
| Can it be empty? | Yes (tags, markers) |

The core requirements (data-only, runtime-managed, identity-based, representation-independent, queryable) are all clearly stated with well-defined constraints.

## Open Questions

None. All 10 open questions have been answered.

## Notes

- "Observable behavior" (representation change constraint) is not formally defined — a developer must use engineering judgment on what constitutes observable vs. internal representation detail. This is acceptable for a data-oriented runtime.
- The spec correctly delegates layout implementations, queries, commands, scheduler behavior, and controller behavior to other specs.
- The `Status: DRAFT` header should be updated to `READY` once this verdict is accepted.
