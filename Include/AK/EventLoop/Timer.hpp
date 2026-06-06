#pragma once

#include <AK/Core/Types.hpp>

namespace GameAK::EventLoop {

class Timer {
public:
  void start(f64 interval, bool repeating = false) noexcept {
    m_interval = interval;
    m_elapsed = 0.0;
    m_repeating = repeating;
    m_running = true;
    m_expired = (interval <= 0.0);
  }

  bool tick(f64 dt) noexcept {
    if (!m_running)
      return false;
    m_elapsed += dt;
    if (m_elapsed < m_interval)
      return false;
    m_expired = true;
    if (m_repeating)
      m_elapsed -= m_interval;
    else
      m_running = false;
    return true;
  }

  void stop() noexcept {
    m_running = false;
    m_expired = false;
  }

  void reset() noexcept {
    m_elapsed = 0.0;
    m_running = false;
    m_expired = false;
  }

  f64 remaining() const noexcept {
    if (!m_running)
      return 0.0;
    f64 rem = m_interval - m_elapsed;
    return rem < 0.0 ? 0.0 : rem;
  }

  bool expired() const noexcept { return m_expired; }

  bool running() const noexcept { return m_running; }

  f64 interval() const noexcept { return m_interval; }
  f64 elapsed() const noexcept { return m_elapsed; }

private:
  f64 m_interval = 0.0;
  f64 m_elapsed = 0.0;
  bool m_repeating = false;
  bool m_running = false;
  bool m_expired = false;
};

} // namespace GameAK::EventLoop
