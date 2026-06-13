# SPEC-003: Platform Architecture

Status: READY

Last validated by Ralph: never

---

## Summary

This specification defines how GameAK supports multiple compilers, operating systems, CPU architectures, and hardware capabilities.

The goal is to provide portable behavior while allowing platform-specific optimizations.

All optimizations must preserve the behavior defined by the generic implementation.

---

## Architectural Model

GameAK is organized into implementation layers.

```text
Behavior
    ↓
Generic Implementation
    ↓
Compiler Specialization
    ↓
CPU Specialization
    ↓
Operating System Integration
```

The generic implementation is the source of truth.

Specialized implementations exist only to improve performance or platform integration.

---

## Generic Layer

The generic layer defines the canonical behavior of every subsystem.

Requirements:

* Must be portable.
* Must be standards-compliant.
* Must not depend on platform-specific APIs.
* Must not depend on CPU-specific instructions.

Every subsystem must provide a generic implementation before any optimization is introduced.

---

## Compiler Layer

Compiler-specific optimizations may be provided.

Examples:

* GCC
* Clang
* MSVC

Requirements:

* Must preserve generic behavior.
* Must remain isolated from business logic.
* Must compile independently.

Compiler-specific code must not become the primary implementation.

---

## CPU Layer

CPU-specific optimizations may be provided.

Examples:

* SSE2
* SSE4
* AVX2
* AVX512
* ARM NEON

Requirements:

* Must preserve generic behavior.
* Must remain interchangeable.
* Must provide identical results.

Optimized implementations must never change runtime semantics.

---

## Operating System Layer

Operating-system integrations may be provided when required.

Examples:

* Linux
* Windows
* macOS
* BSD

Requirements:

* Must remain isolated.
* Must expose consistent interfaces.
* Must not leak platform-specific behavior into the runtime.

---

## Runtime Selection

Implementations may be selected:

* At compile time.
* At startup.
* Through capability detection.

The selection mechanism must be transparent to runtime consumers.

---

## Testing Requirements

Every optimized implementation must pass the same test suite as the generic implementation.

The generic implementation defines correctness.

Optimized implementations define performance.

---

## Constraints

* Generic implementations are mandatory.
* Optimized implementations are optional.
* Optimized implementations must preserve behavior.
* Platform-specific code must remain isolated.
* Runtime behavior must remain deterministic.

---

## Out of Scope

* Build system configuration.
* Compiler flags.
* Packaging.
* Benchmarking methodology.
* Platform support matrix.

---

## Open Questions

* [ ] Should implementation selection occur at compile time, runtime, or both?

**Answer:**

Both.

Compile-time selection determines the available implementation set.

Runtime selection chooses the most capable implementation supported by the current environment.

* [ ] Which CPU instruction sets receive first-class support?

**Answer:**

The following instruction sets receive first-class support:

* Generic Scalar
* SSE2
* AVX2
* AVX512
* ARM NEON
* WebAssembly SIMD

Additional instruction sets may be supported in the future.

* [ ] Should unsupported platforms automatically fall back to generic implementations?

**Answer:**

Yes.

Every subsystem must provide a generic implementation that serves as the universal fallback.

* [ ] How should capability detection be exposed for diagnostics?

**Answer:**

Capability detection is planned as a separate specification (SPEC-013).

The initial implementation does not require a diagnostics API.

The diagnostics API will expose:

* Compiler information
* CPU architecture information
* Operating system information
* Available instruction sets
* Active implementation backend

This information is intended for diagnostics and debugging purposes.

---

## Definitions

### Generic Implementation

The canonical implementation that defines runtime behavior.

### Specialized Implementation

An implementation optimized for a compiler, CPU feature, or operating system.

### Capability Detection

The process of identifying available platform features.

### Runtime Semantics

The observable behavior of the runtime regardless of implementation.

### WebAssembly

WebAssembly is a first-class supported platform.

GameAK must provide generic implementations compatible with WebAssembly environments.

Platform-specific optimizations must degrade gracefully when unavailable.

WebAssembly support must not require changes to the public API.
