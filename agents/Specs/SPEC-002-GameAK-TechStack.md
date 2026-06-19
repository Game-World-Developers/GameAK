
# SPEC-002: GameAK Tech Stack

Status: READY

Last validated by Ralph: 2026-06-13

---

## Summary

This specification defines the technology stack used to implement GameAK.

Technology choices exist to support the architecture defined by GameAK and must not dictate architectural decisions.

The purpose of this specification is to reduce ambiguity during development and provide a consistent implementation foundation.

---

## Programming Language

### C++

GameAK is implemented in modern C++.

Requirements:

* The minimum supported standard is C++20.
* Standard Library facilities should be preferred whenever practical.
* Experimental language features should be avoided.
* Reflection is not permitted.
* Compiler-specific extensions must remain isolated.

---

## Logging

### spdlog

GameAK uses spdlog as its logging backend.

Requirements:

* Support structured logging.
* Support multiple log levels.
* Support runtime diagnostics.
* Support configurable sinks.

Logging must remain optional for runtime consumers.

---

## Testing

### Cest

GameAK uses Cest as its testing framework.

Requirements:

* Unit testing support.
* Integration testing support.
* Regression testing support.
* Test discovery support.

All runtime features must be testable through Cest.

---

## Dependency Philosophy

External dependencies should remain minimal.

Dependencies must:

* Provide clear value.
* Reduce maintenance burden.
* Improve developer productivity.

Dependencies must not become architectural requirements.

---

## Constraints

* C++20 is required.
* spdlog is the standard logging backend.
* Cest is the standard testing framework.
* Runtime consumers are not required to use spdlog.
* Runtime consumers are not required to use Cest.
* Reflection is prohibited.
* Experimental language features should be avoided.

---

## Out of Scope

* Coding standards.
* Build systems.
* Package managers.
* Continuous integration.
* Deployment pipelines.
* Documentation tooling.
* Platform-specific optimizations.

---

## Open Questions

* [ ] Which compilers are officially supported?

**Answer:**

The officially supported compilers are:

* GCC
* Clang
* MSVC

Other compilers are considered best-effort.

* [ ] What are the minimum compiler versions?

**Answer:**

The minimum supported versions are:

* GCC 13+
* Clang 17+
* MSVC 2022 (17.8+) or newer

* [ ] Which package manager should be officially supported?

**Answer:**

vcpkg is the official package manager.

Support for other package managers is optional.

* [ ] Should logging be removable at compile time?

**Answer:**

Yes.

Logging must be completely removable at compile time.

The runtime must not require logging facilities when logging support is disabled.

* [ ] Is CMake mandatory?

**Answer:**

Xmake is the primary build system.

CMake generation may be provided as a compatibility layer when required by external tooling.

---

## Definitions

### Runtime Consumer

An application or library that depends on GameAK.

### Dependency

An external library used during development or execution.

### Logging Backend

A library responsible for recording diagnostics and runtime events.

### Testing Framework

A library responsible for validating runtime behavior.
