#include <AK/Backend/ExecContext.hpp>
#include <AK/EventLoop/EventBus.hpp>
#include <AK/EventLoop/Loop.hpp>
#include <cest.h>

namespace {

static int g_call_count = 0;
static GameAK::u64 g_last_frame = 0;
static GameAK::f64 g_last_dt = 0.0;
static GameAK::EventLoop::Loop *g_test_loop = nullptr;

void count_call(GameAK::ExecContext &ctx) {
  ++g_call_count;
  g_last_frame = ctx.frame_index;
  g_last_dt = ctx.delta_time;
}

void stop_loop(GameAK::ExecContext &) {
  if (g_last_frame >= 5 && g_test_loop)
    g_test_loop->stop();
}

} // namespace

int main() {
  describe("GameAK::EventLoop::Loop", {
    it("should be stopped initially", {
      GameAK::EventLoop::EventBus bus;
      GameAK::EventLoop::Loop loop(bus);
      expect(loop.is_running()).toBeFalsy();
    });

    it("should accept fixed and variable systems", {
      GameAK::EventLoop::EventBus bus;
      GameAK::EventLoop::Loop loop(bus);
      loop.add_fixed_system(count_call);
      loop.add_variable_system(count_call);
    });

    it("should increment frame_index each frame", {
      GameAK::EventLoop::EventBus bus;
      GameAK::EventLoop::Loop loop(bus, 0.001);
      GameAK::ExecContext ctx{{}, 0, 0.0};

      g_test_loop = &loop;
      g_last_frame = 0;
      loop.add_variable_system(stop_loop);
      loop.add_variable_system(count_call);
      loop.run(ctx);
      expect(g_last_frame).toBe(6u);
      g_test_loop = nullptr;
    });

    it("should call fixed systems at least once per frame", {
      GameAK::EventLoop::EventBus bus;
      GameAK::EventLoop::Loop loop(bus, 0.1);
      GameAK::ExecContext ctx{{}, 0, 0.0};

      g_test_loop = &loop;
      g_call_count = 0;
      g_last_frame = 0;
      loop.add_fixed_system(count_call);
      loop.add_variable_system(stop_loop);
      loop.run(ctx);
      expect(g_call_count).toBeGreaterThan(0);
      g_test_loop = nullptr;
    });

    it("should stop correctly", {
      GameAK::EventLoop::EventBus bus;
      GameAK::EventLoop::Loop loop(bus);
      expect(loop.is_running()).toBeFalsy();
      loop.stop();
      expect(loop.is_running()).toBeFalsy();
    });
  });
  return cest_result();
}
