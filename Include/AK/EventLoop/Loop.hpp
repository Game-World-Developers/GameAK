#pragma once

#include <AK/Backend/ExecContext.hpp>
#include <AK/Core/Types.hpp>

namespace GameAK::EventLoop {

class EventBus;

static constexpr usize kMaxSystems = 64;
static constexpr f64 kDefaultFixedDt = 1.0 / 60.0;

using SystemFn = void (*)(ExecContext &);

class Loop {
public:
  explicit Loop(EventBus &bus, f64 fixed_dt = kDefaultFixedDt) noexcept;

  void add_fixed_system(SystemFn fn) noexcept;
  void add_variable_system(SystemFn fn) noexcept;

  void run(ExecContext &ctx) noexcept;
  void stop() noexcept;

  bool is_running() const noexcept { return m_running; }
  f64 fixed_dt() const noexcept { return m_fixed_dt; }

private:
  EventBus *m_bus;
  SystemFn m_fixed_systems[kMaxSystems];
  SystemFn m_variable_systems[kMaxSystems];
  usize m_fixed_count;
  usize m_variable_count;
  f64 m_fixed_dt;
  bool m_running;
};

} // namespace GameAK::EventLoop
