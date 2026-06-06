#include <AK/EventLoop/Timer.hpp>
#include <cest.h>

int main() {
  describe("GameAK::EventLoop::Timer", {
    it("should be stopped by default", {
      GameAK::EventLoop::Timer t;
      expect(t.running()).toBeFalsy();
      expect(t.expired()).toBeFalsy();
      expect(t.remaining()).toBe(0.0);
      expect(t.elapsed()).toBe(0.0);
    });

    it("should not expire before interval", {
      GameAK::EventLoop::Timer t;
      t.start(5.0);
      expect(t.running()).toBeTruthy();
      expect(t.tick(3.0)).toBeFalsy();
      expect(t.expired()).toBeFalsy();
      expect(t.remaining()).toBeGreaterThan(0.0);
    });

    it("should expire exactly at interval (oneshot)", {
      GameAK::EventLoop::Timer t;
      t.start(5.0);
      expect(t.tick(5.0)).toBeTruthy();
      expect(t.expired()).toBeTruthy();
      expect(t.running()).toBeFalsy();
    });

    it("should expire after interval (oneshot)", {
      GameAK::EventLoop::Timer t;
      t.start(3.0);
      expect(t.tick(5.0)).toBeTruthy();
      expect(t.expired()).toBeTruthy();
      expect(t.running()).toBeFalsy();
    });

    it("should repeat automatically", {
      GameAK::EventLoop::Timer t;
      t.start(2.0, true);
      expect(t.tick(2.0)).toBeTruthy();
      expect(t.running()).toBeTruthy();
      expect(t.elapsed()).toBe(0.0);
      expect(t.tick(2.0)).toBeTruthy();
      expect(t.running()).toBeTruthy();
    });

    it("should stop correctly", {
      GameAK::EventLoop::Timer t;
      t.start(10.0);
      t.stop();
      expect(t.running()).toBeFalsy();
      expect(t.tick(100.0)).toBeFalsy();
    });

    it("should reset correctly", {
      GameAK::EventLoop::Timer t;
      t.start(1.0);
      t.tick(0.5);
      t.reset();
      expect(t.running()).toBeFalsy();
      expect(t.elapsed()).toBe(0.0);
    });

    it("should report remaining time correctly", {
      GameAK::EventLoop::Timer t;
      t.start(10.0);
      t.tick(4.0);
      expect(t.remaining()).toBe(6.0);
    });

    it("[Edge] tick on stopped timer is no-op", {
      GameAK::EventLoop::Timer t;
      expect(t.tick(5.0)).toBeFalsy();
    });

    it("[Edge] zero-interval timer expires immediately", {
      GameAK::EventLoop::Timer t;
      t.start(0.0);
      expect(t.expired()).toBeTruthy();
      expect(t.tick(0.0)).toBeTruthy();
    });
  });
  return cest_result();
}
