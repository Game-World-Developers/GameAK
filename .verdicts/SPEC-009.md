# SPEC-009: Runtime API

**Validation timestamp:** 2026-06-13

**Verdict:** READY / IMPLEMENTED

## Assessment

SPEC-009 defines the public API surface of the GameAK Runtime. All originally identified blockers have been resolved. The spec has been extended with the following additions implemented in the codebase:

## What Changed Since Previous Validation

### Previously Blocked: 6 Issues — All Resolved (unchanged)

| # | Issue | Resolution | Status |
|---|-------|-----------|--------|
| 1 | **Undefined tick execution ordering** | Now explicitly defined: (1) Controllers execute, (2) pending Commands are validated and executed by the Scheduler, (3) TickResult is produced. Commands submitted between ticks are queued for the next `tick()`. | ✅ |
| 2 | **Undefined allocator interface** | Removed from v1 configuration. Config now contains only log level. | ✅ |
| 3 | **Undefined block capacity hints** | Removed from v1 configuration. | ✅ |
| 4 | **Undefined sensible defaults** | Default configuration values documented: Log level defaults to `warn`. | ✅ |
| 5 | **Undefined ExecutionStatus** | Enumeration explicitly defined. | ✅ |
| 6 | **Undefined type descriptor structure** | `BlockTypeDescriptor` struct explicitly defined. | ✅ |

### New Features Added (Post-Validation)

| Feature | Spec Section | Tests |
|---------|-------------|-------|
| **Pause / Resume** | Pause / Resume | `pause skips tick execution`, `resume allows tick execution after pause`, `paused runtime still accepts commands but does not process them` |
| **Fixed Timestep** | Fixed Timestep | `fixed timestep accumulates and runs multiple sub-ticks`, `fixed timestep sub-ticks see the fixed delta`, `clear_fixed_timestep reverts to variable timestep` |
| **Query Operations** | Query Operations | `find_blocks_by_type returns blocks of matching type`, `find_blocks with predicate filters correctly`, `find_blocks returns empty when nothing matches`, `find_blocks returns all blocks when predicate always true` |
| **Serialization (Snapshot)** | Serialization (Snapshot) | `save captures current state`, `load restores previously saved state`, `load rebuilds type_counts` |
| **Block Relationships** | Block Relationships | `relate creates parent-child edge`, `unrelate removes parent-child edge`, `multiple children per parent`, `multiple parents per child`, `relating self fails`, `relating nonexistent block fails` |

## Cross-Reference Verification

All API concepts referenced in SPEC-009 are defined in companion specs:

| Concept | Definition Location |
|---------|-------------------|
| **Controller** | SPEC-010 |
| **Controller Priority** | SPEC-010 |
| **Command** | SPEC-007 |
| **Runtime Identity** | SPEC-008 |
| **Data Block** | SPEC-006 |
| **Result type** | SPEC-012 |
| **Scheduler** | SPEC-011 |
| **Event System** | SPEC-017 |

## Remaining Observations (Non-Blocking)

- The `ControllerEntry` struct and priority sorting are implementation details not exposed in the spec.
- `create_block()` and `destroy_block()` now update `type_counts_` incrementally (previously only rebuilt at tick end).
- The `RuntimeConfig` struct is intentionally minimal. Future versions may add allocator customization and capacity hints.

## Open Questions

All resolved. No unanswered questions remain.

## Final Determination

**SPEC-009 is READY and IMPLEMENTED.** All requirements are covered by passing tests.
