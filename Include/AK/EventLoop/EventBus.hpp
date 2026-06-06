#pragma once

#include <AK/Backend/ExecContext.hpp>
#include <AK/Core/Macros.hpp>
#include <AK/Core/Types.hpp>

#include <cstring>

namespace GameAK::EventLoop {

static constexpr usize kDefaultEventCapacity = 64;
static constexpr usize kDefaultSubscriberCount = 32;

struct Event {
  u32 type;
  u64 data[2];
};

using Handler = void (*)(const Event &, ExecContext &);

struct Subscription {
  u32 type;
  Handler handler;
};

class EventBus {
public:
  EventBus() noexcept
      : m_events{},
        m_subs{},
        m_head(0), m_tail(0),
        m_sub_count(0) {}

  void subscribe(u32 type, Handler handler) noexcept {
    for (usize i = 0; i < m_sub_count; ++i) {
      if (m_subs[i].type == type && m_subs[i].handler == handler)
        return;
    }
    if (GAMEAK_LIKELY(m_sub_count < kDefaultSubscriberCount)) {
      m_subs[m_sub_count].type = type;
      m_subs[m_sub_count].handler = handler;
      ++m_sub_count;
    }
  }

  void publish(const Event &event) noexcept {
    usize next = (m_head + 1) % kDefaultEventCapacity;
    if (GAMEAK_UNLIKELY(next == m_tail))
      return;
    m_events[m_head] = event;
    m_head = next;
  }

  void dispatch(ExecContext &ctx) noexcept {
    while (m_tail != m_head) {
      const Event &e = m_events[m_tail];
      for (usize i = 0; i < m_sub_count; ++i) {
        if (m_subs[i].type == e.type)
          m_subs[i].handler(e, ctx);
      }
      m_tail = (m_tail + 1) % kDefaultEventCapacity;
    }
  }

  void clear() noexcept { m_head = m_tail = 0; }

  usize pending() const noexcept {
    return (m_head + kDefaultEventCapacity - m_tail) % kDefaultEventCapacity;
  }

  usize subscriber_count() const noexcept { return m_sub_count; }

private:
  Event m_events[kDefaultEventCapacity];
  Subscription m_subs[kDefaultSubscriberCount];
  usize m_head;
  usize m_tail;
  usize m_sub_count;
};

template <typename T>
const T &event_data(const Event &e) noexcept {
  static_assert(sizeof(T) <= sizeof(e.data));
  return *reinterpret_cast<const T *>(e.data);
}

template <typename T>
void publish(EventBus &bus, u32 type, const T &data) noexcept {
  static_assert(sizeof(T) <= sizeof(Event::data));
  Event e;
  e.type = type;
  __builtin_memcpy(e.data, &data, sizeof(T));
  bus.publish(e);
}

namespace Detail {

template <typename T>
struct TypedHandler {
  using Fn = void (*)(const T &, ExecContext &);
  static Fn s_handler;

  static void trampoline(const Event &e, ExecContext &ctx) noexcept {
    s_handler(event_data<T>(e), ctx);
  }
};

template <typename T>
typename TypedHandler<T>::Fn TypedHandler<T>::s_handler = nullptr;

} // namespace Detail

template <typename T>
void subscribe(EventBus &bus, u32 type, void (*handler)(const T &, ExecContext &)) noexcept {
  Detail::TypedHandler<T>::s_handler = handler;
  bus.subscribe(type, Detail::TypedHandler<T>::trampoline);
}

} // namespace GameAK::EventLoop
