#include <AK/Backend/ExecContext.hpp>
#include <AK/EventLoop/EventBus.hpp>
#include <cest.h>

namespace {

static int g_test_count = 0;
static GameAK::u32 g_last_a = 0;
static GameAK::f32 g_last_b = 0.0f;

void count_handler(const GameAK::EventLoop::Event &, GameAK::ExecContext &) {
  ++g_test_count;
}

struct TestPayload {
  GameAK::u32 a;
  GameAK::f32 b;
};

void typed_handler(const TestPayload &data, GameAK::ExecContext &) {
  g_last_a = data.a;
  g_last_b = data.b;
}

void event_data_handler(const GameAK::EventLoop::Event &e,
                        GameAK::ExecContext &) {
  auto &data = GameAK::EventLoop::event_data<TestPayload>(e);
  g_last_a = data.a;
  g_last_b = data.b;
}

} // namespace

int main() {
  describe("GameAK::EventLoop::EventBus", {
    it("should be empty initially", {
      GameAK::EventLoop::EventBus bus;
      expect(bus.pending()).toBe(0u);
      expect(bus.subscriber_count()).toBe(0u);
    });

    it("should dispatch to subscriber", {
      GameAK::EventLoop::EventBus bus;
      g_test_count = 0;
      GameAK::ExecContext ctx{{}, 0, 0.0};

      bus.subscribe(1, count_handler);
      GameAK::EventLoop::Event e{1, {}};
      bus.publish(e);
      expect(bus.pending()).toBe(1u);
      bus.dispatch(ctx);
      expect(g_test_count).toBe(1);
      expect(bus.pending()).toBe(0u);
    });

    it("should not dispatch duplicate subscription", {
      GameAK::EventLoop::EventBus bus;
      g_test_count = 0;

      bus.subscribe(1, count_handler);
      bus.subscribe(1, count_handler);
      expect(bus.subscriber_count()).toBe(1u);
    });

    it("should not dispatch to subscribers of different types", {
      GameAK::EventLoop::EventBus bus;
      g_test_count = 0;
      GameAK::ExecContext ctx{{}, 0, 0.0};

      bus.subscribe(2, count_handler);
      GameAK::EventLoop::Event e{1, {}};
      bus.publish(e);
      bus.dispatch(ctx);
      expect(g_test_count).toBe(0);
    });

    it("should clear pending events", {
      GameAK::EventLoop::EventBus bus;

      GameAK::EventLoop::Event e{1, {}};
      bus.publish(e);
      bus.clear();
      expect(bus.pending()).toBe(0u);
    });

    it("should drop events when queue is full", {
      GameAK::EventLoop::EventBus bus;

      g_test_count = 0;
      bus.subscribe(1, count_handler);
      for (GameAK::u32 i = 0; i < GameAK::EventLoop::kDefaultEventCapacity + 10; ++i) {
        GameAK::EventLoop::Event e{1, {}};
        bus.publish(e);
      }

      GameAK::ExecContext ctx{{}, 0, 0.0};
      bus.dispatch(ctx);
      expect(g_test_count).toBe(GameAK::u32(GameAK::EventLoop::kDefaultEventCapacity - 1));
    });

    it("should support typed publish and event_data access", {
      GameAK::EventLoop::EventBus bus;
      g_last_a = 0;
      g_last_b = 0.0f;
      GameAK::ExecContext ctx{{}, 0, 0.0};

      TestPayload sent{42, 3.14f};
      GameAK::EventLoop::publish(bus, 10, sent);
      bus.subscribe(10, event_data_handler);
      bus.dispatch(ctx);
      expect(g_last_a).toBe(42u);
      expect(g_last_b).toBe(3.14f);
    });

    it("should support typed subscribe", {
      GameAK::EventLoop::EventBus bus;
      g_last_a = 0;
      g_last_b = 0.0f;
      GameAK::ExecContext ctx{{}, 0, 0.0};

      TestPayload sent{99, 2.71f};
      GameAK::EventLoop::subscribe<TestPayload>(bus, 20, typed_handler);
      GameAK::EventLoop::publish(bus, 20, sent);
      bus.dispatch(ctx);
      expect(g_last_a).toBe(99u);
      expect(g_last_b).toBe(2.71f);
    });

    it("should dispatch in FIFO order", {
      GameAK::EventLoop::EventBus bus;
      GameAK::ExecContext ctx{{}, 0, 0.0};

      static int order = 0;
      order = 0;

      struct OrderHandler {
        static void first(const GameAK::EventLoop::Event &,
                          GameAK::ExecContext &) {
          order = order * 10 + 1;
        }
        static void second(const GameAK::EventLoop::Event &,
                           GameAK::ExecContext &) {
          order = order * 10 + 2;
        }
      };

      bus.subscribe(1, OrderHandler::first);
      bus.subscribe(1, OrderHandler::second);

      bus.publish({1, {}});
      bus.dispatch(ctx);
      expect(order).toBe(12);
    });

    it("[Edge] dispatch on empty bus is safe", {
      GameAK::EventLoop::EventBus bus;
      GameAK::ExecContext ctx{{}, 0, 0.0};
      bus.dispatch(ctx);
      expect(bus.pending()).toBe(0u);
    });

    it("[Edge] subscribe after publish delivers enqueued events", {
      GameAK::EventLoop::EventBus bus;
      g_test_count = 0;
      GameAK::ExecContext ctx{{}, 0, 0.0};

      bus.publish(GameAK::EventLoop::Event{1, {}});
      bus.subscribe(1, count_handler);
      bus.dispatch(ctx);
      expect(g_test_count).toBe(1);
    });
  });
  return cest_result();
}
