# SPEC-022: Runtime Pipelines

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-17

---

## Summary

This specification defines Runtime Pipelines as a mechanism for composing state transformations.

A Pipeline is a Controller composed of an ordered sequence of stages. Each stage transforms state by producing Commands. Stages are executed sequentially, and the output of one stage feeds the input of the next through shared state.

Pipelines enable structured, composable transformation chains.

---

## Behavior

### Scenario: Pipeline executes stages in order

Given a Pipeline with stages [A, B, C]

When the Pipeline is executed

Then A executes first, then B, then C

**Test:** `test_pipeline.cpp` — `stages_execute_in_order`

### Scenario: Stage produces Commands visible to subsequent stages

Given Pipeline stage A creates a Data Block and stage B queries blocks of that type

When the Pipeline executes

Then stage B sees the block created by stage A

**Test:** `test_pipeline.cpp` — `stage_output_feeds_next`

### Scenario: Empty Pipeline produces no Commands

Given a Pipeline with no stages

When executed

Then no Commands are produced

**Test:** `test_pipeline.cpp` — `empty_pipeline`

### Scenario: Pipeline stage is a Controller

Given a Pipeline stage defined as a callable

When the Pipeline executes

Then each stage receives StateView and CommandProducer, and returns Result

**Test:** `test_pipeline.cpp` — `stage_is_controller`

### Scenario: Pipeline stages may have names for diagnostics

Given a Pipeline with stages ["Movement", "Collision", "Cleanup"]

When diagnostics are queried

Then each stage name is reported

**Test:** `test_pipeline.cpp` — `pipeline_stage_diagnostics`

### Scenario: Pipeline is itself a Controller

Given a Pipeline

When registered with the Runtime as a Controller

Then it is called during the Controller Execution Phase like any other Controller

**Test:** `test_pipeline.cpp` — `pipeline_is_controller`

---

## Constraints

* Pipeline must be a Controller (callable type receiving StateView + CommandProducer).
  * **Test verification:** `test_pipeline.cpp` — `pipeline_is_controller`
* Each stage must be a callable compatible with the Controller signature.
  * **Test verification:** `test_pipeline.cpp` — `stage_is_controller`
* Stages must execute in deterministic order.
  * **Test verification:** `test_pipeline.cpp` — `deterministic_stage_order`
* Stage failure must not skip subsequent stages (stage isolation).
  * **Test verification:** `test_pipeline.cpp` — `stage_failure_does_not_skip`
* A Pipeline may contain zero or more stages.
  * **Test verification:** `test_pipeline.cpp` — `empty_pipeline`

---

## Out of Scope

* Dynamic stage insertion/removal during execution.
* Conditional stage execution (if/else branching in pipeline).
* Parallel stage execution.
* Pipeline branching or forking.
* Stage retry logic.

---

## Open Questions

* [x] Should stages have access to exclusive sub-CommandProducers or share one?

**Answer:** Shared. All stages share a single CommandProducer.

* [x] Should Pipeline expose add/remove stage APIs between ticks?

**Answer:** Yes. Pipeline exposes add/remove stage APIs between ticks.

* [x] Should Pipeline support stage-local state (isolated from other stages)?

**Answer:** Yes. Pipeline supports stage-local state isolated from other stages.

* [x] How should stage failure be reported — per-stage or aggregate?

**Answer:** Per-stage. Stage failures are reported individually.

* [x] Should Pipeline be nestable (Pipeline containing Pipelines)?

**Answer:** Yes. Pipeline is nestable (a Pipeline may contain other Pipelines).

---

## Definitions

### Pipeline

A Controller that executes an ordered sequence of stages. Each stage receives the current state and produces Commands.

### Stage

An individual Controller within a Pipeline. Receives StateView and CommandProducer, returns Result.

### Stage Isolation

The property that a stage failure does not prevent subsequent stages from executing.
