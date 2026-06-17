# SPEC-021: Rule Systems

Status: IMPLEMENTED

Last validated by Ralph: 2026-06-17

---

## Summary

This specification defines Rule Systems as a runtime state transformation mechanism.

A Rule System is a Controller that evaluates a set of rules — each rule is a condition paired with an action — and produces Commands for all matching rules.

Rules provide a declarative, composable way to express state transformation logic.

---

## Behavior

### Scenario: Rule with matching condition fires

Given a Rule System with a rule: if block.has("health") < 0 -> DestroyBlock

When a Data Block has health = -5

Then the Rule System produces a DestroyBlock command for that block

**Test:** `test_rule_system.cpp` — `matching_condition_fires`

### Scenario: Rule with non-matching condition does not fire

Given a Rule System with a rule: if block.has("health") < 0 -> DestroyBlock

When all Data Blocks have health >= 0

Then the Rule System produces no Commands

**Test:** `test_rule_system.cpp` — `non_matching_condition_does_not_fire`

### Scenario: Multiple rules may fire in a single tick

Given a Rule System with rule A (health < 0 -> DestroyBlock) and rule B (score > 100 -> SetField score=0)

When a block has health = -1 and another block has score = 200

Then the Rule System produces both a DestroyBlock and a SetField command

**Test:** `test_rule_system.cpp` — `multiple_rules_fire`

### Scenario: Rules are evaluated in priority order

Given rules R1 (priority=10), R2 (priority=5), R3 (priority=0)

When all three conditions match

Then R1 fires first, then R2, then R3

**Test:** `test_rule_system.cpp` — `rule_priority_order`

### Scenario: Rule actions produce Commands

Given a rule with condition true -> SetField(position.x = position.x + 1)

When the rule fires

Then a SetField command is produced with the specified field and value

**Test:** `test_rule_system.cpp` — `rule_action_produces_command`

### Scenario: Rule System produces no Commands when no rules match

Given Rule System with no rules (empty)

When executed

Then no Commands are produced

**Test:** `test_rule_system.cpp` — `empty_rule_system`

---

## Constraints

* Rule System must be a Controller (callable type receiving StateView + CommandProducer).
  * **Test verification:** `test_rule_system.cpp` — `rule_system_is_controller`
* Rules must be evaluated deterministically (same state -> same Commands).
  * **Test verification:** `test_rule_system.cpp` — `deterministic_evaluation`
* Rule System must not hold mutable state between ticks.
  * **Test verification:** `test_rule_system.cpp` — `no_mutable_state`
* Each rule must have a unique priority value to ensure stable ordering.
  * **Test verification:** `test_rule_system.cpp` — `unique_priorities`
* Rules may not directly mutate state — they produce Commands.
  * **Test verification:** `test_rule_system.cpp` — `rules_produce_commands`

---

## Out of Scope

* Rule conflict resolution (multiple rules targeting the same block/field in the same tick).
* Rule chaining (rule A result triggers rule B condition within the same tick).
* External rule definition files or DSL parsing.
* Rete network or other optimization algorithms.
* Rule retraction / truth maintenance.

---

## Open Questions

* [x] Should rule conditions be predicates (bool function) or expressions (query-based)?

**Answer:** Expressions (query-based). Conditions are query-based expressions.

* [x] Should rules support AND/OR/NOT composition?

**Answer:** Yes. Rules support AND/OR/NOT composition.

* [x] How should rule evaluation order be determined when priorities are equal?

**Answer:** Registration order. When priorities are equal, rules are evaluated in registration order.

* [x] Should Rule System support dynamic add/remove of rules between ticks?

**Answer:** Yes. Rule System supports dynamic add/remove of rules between ticks.

* [x] Should rule conditions have access to the full StateView or only specific block types?

**Answer:** Full StateView. Rule conditions have access to the complete StateView.

---

## Definitions

### Rule

A pair (condition, action) where condition is a predicate over state and action produces Commands.

### Condition

A predicate function that receives state context and returns true if the rule should fire.

### Action

A function that receives a CommandProducer and emits Commands representing the state modification.

### Rule System

A Controller that evaluates a set of rules against the current state and produces their Commands.

### Priority

An integer value determining rule evaluation order. Higher values are evaluated first.
