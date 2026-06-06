#include <AK/EventLoop/EventBus.hpp>
#include <AK/EventLoop/Loop.hpp>

#include <ctime>

namespace GameAK::EventLoop {

namespace {

f64 now_seconds() noexcept {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return static_cast<f64>(ts.tv_sec) + static_cast<f64>(ts.tv_nsec) * 1e-9;
}

} // namespace

Loop::Loop(EventBus &bus, f64 fixed_dt) noexcept
    : m_bus(&bus),
      m_fixed_systems{},
      m_variable_systems{},
      m_fixed_count(0),
      m_variable_count(0),
      m_fixed_dt(fixed_dt),
      m_running(false) {}

void Loop::add_fixed_system(SystemFn fn) noexcept {
  if (m_fixed_count < kMaxSystems)
    m_fixed_systems[m_fixed_count++] = fn;
}

void Loop::add_variable_system(SystemFn fn) noexcept {
  if (m_variable_count < kMaxSystems)
    m_variable_systems[m_variable_count++] = fn;
}

void Loop::run(ExecContext &ctx) noexcept {
  m_running = true;
  f64 last = now_seconds();
  f64 accumulator = 0.0;
  ctx.frame_index = 0;

  while (m_running) {
    f64 now = now_seconds();
    ctx.delta_time = now - last;
    last = now;

    if (ctx.delta_time > 0.1)
      ctx.delta_time = 0.1;

    ++ctx.frame_index;
    accumulator += ctx.delta_time;

    while (accumulator >= m_fixed_dt) {
      for (usize i = 0; i < m_fixed_count; ++i)
        m_fixed_systems[i](ctx);
      accumulator -= m_fixed_dt;
    }

    for (usize i = 0; i < m_variable_count; ++i)
      m_variable_systems[i](ctx);

    m_bus->dispatch(ctx);
  }
}

void Loop::stop() noexcept { m_running = false; }

} // namespace GameAK::EventLoop
