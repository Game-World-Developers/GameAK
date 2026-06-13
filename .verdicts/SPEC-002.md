# SPEC-002: GameAK Tech Stack

**Validation timestamp:** 2026-06-13

**Verdict:** READY

## Assessment

SPEC-002 defines the technology stack with clear decisions:

| Concern | Decision |
|---|---|
| Language | C++20 (no reflection, avoid experimental features) |
| Logging | spdlog (compile-time removable, optional for consumers) |
| Testing | Cest |
| Compilers | GCC 13+, Clang 17+, MSVC 2022 (17.8+) |
| Package manager | vcpkg |
| Build system | Xmake (primary), CMake generation optional |
| Exceptions | Not permitted in runtime core |

All five open questions have been answered with concrete detail. The dependency philosophy is well-defined.

## Minor Ambiguities

- "Experimental language features should be avoided" is subjective but workable with engineering judgment.
- Build system tooling is listed in "Out of Scope" but the Q&A specifies Xmake as primary. This is not a contradiction — the out-of-scope section refers to *configuration* (compiler flags, CI), not the choice of build tool.

## Open Questions

None. All five open questions have been answered.

## Notes

- This spec requires no code implementation itself — it establishes constraints for the build setup and dependency management.
- The `Status: DRAFT` header should be updated to `READY` once this verdict is accepted.
