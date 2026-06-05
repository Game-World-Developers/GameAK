#pragma once

#include <AK/Core/Bits/BitArray.hpp>
#include <AK/Core/Bits/BitLayer.hpp>
#include <AK/Core/Bits/BitMask.hpp>
#include <AK/Core/Bits/BitOps.hpp>
#include <AK/Core/Bits/BitPack.hpp>

/// A lightweight RAII helper responsible for executing cleanup
/// operations when leaving a scope.
///
/// The Janitor does not own memory by itself.
/// Instead it guarantees that a cleanup routine is executed
/// for the managed object when the scope ends.
template <typename T> class Janitor {
public:
  constexpr Janitor() = default;
  constexpr explicit Janitor(T *ptr) : m_ptr(ptr) {}

  Janitor(const Janitor &) = delete;
  Janitor &operator=(const Janitor &) = delete;

  Janitor(Janitor &&other) noexcept : m_ptr(other.m_ptr) {
    other.m_ptr = nullptr;
  }

  Janitor &operator=(Janitor &&other) noexcept {
    if (this != &other) {
      Cleanup();

      m_ptr = other.m_ptr;
      other.m_ptr = nullptr;
    }

    return *this;
  }

  ~Janitor() { Cleanup(); }

  constexpr T *Get() const noexcept { return m_ptr; }

  constexpr explicit operator bool() const noexcept { return m_ptr != nullptr; }

  void Release() noexcept { m_ptr = nullptr; }

  void Reset() noexcept {
    if (m_ptr == nullptr) {
      return;
    }

    Cleanup();

    m_ptr = nullptr;
  }

private:
  void Cleanup() {
    if (m_ptr != nullptr) {
      m_ptr->~T();
    }
  }

  T *m_ptr{nullptr};
};
