
# Project Rules

## Spec Driven Development

NEVER implement anything that is not covered by a spec in Specs/.

Before writing any code, run @ralph on the relevant spec file.

Only proceed if Ralph returns:

VERDICT: READY

If Ralph returns:

VERDICT: NOT READY

stop immediately and report the unresolved questions.

Do not assume.
Do not infer.
Do not invent behavior.

## Spec lifecycle

Every spec must have one of the following states:

* DRAFT
* REVIEWED
* BLOCKED
* READY
* IMPLEMENTED

Definitions:

* DRAFT → initial version
* REVIEWED → reviewed but not validated
* BLOCKED → waiting for answers
* READY → validated by Ralph
* IMPLEMENTED → code completed

## Implementation rules

* Every new file must correspond to a spec.
* Every new behavior must correspond to a spec.
* If a spec is silent, ask.
* Tests must cover every behavior explicitly described in the spec.
* If a spec changes, re-run Ralph before modifying code.

## Validation artifacts

After validation Ralph must generate:

.verdicts/<SPEC-ID>.md

Example:

.veredicts/SPEC-001.md

The verdict file must contain:

* Spec identifier
* Validation timestamp
* Verdict
* Open questions
* Notes

## Commit discipline

Every commit must reference the spec:

feat(SPEC-001): implement authentication

fix(SPEC-001): handle expired sessions

refactor(SPEC-002): simplify inventory storage
