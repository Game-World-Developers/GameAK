#include <AK/Backend/ExecContext.hpp>
#include <AK/FSM/StateMachine.hpp>
#include <cest.h>

namespace {

enum class State { Root, Menu, MainMenu, Settings, Playing, Alive, Dead };
enum class Event : GameAK::u32 {
  Play,
  Die,
  Restart,
  OpenSettings,
  CloseSettings,
  Pause,
  Resume,
};

static int g_enter_count = 0;
static int g_exit_count = 0;
static int g_update_calls = 0;

void track_enter(GameAK::ExecContext &) { ++g_enter_count; }
void track_exit(GameAK::ExecContext &) { ++g_exit_count; }
void track_update(GameAK::ExecContext &) { ++g_update_calls; }

bool always_true(GameAK::ExecContext &) { return true; }
bool always_false(GameAK::ExecContext &) { return false; }

GameAK::ExecContext make_ctx() { return {{}, 0, 0.0}; }

} // namespace

int main() {
  describe("GameAK::FSM::StateMachine (flat)", {
    it("should start in initial state", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Menu);
      expect(fsm.current() == State::Menu).toBeTruthy();
      expect(fsm.is_active(State::Menu)).toBeTruthy();
    });

    it("should transition on event", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Menu);
      fsm.add_state({State::MainMenu, State::Menu, State::MainMenu});
      fsm.add_transition(State::Menu, Event::Play, State::MainMenu);
      auto ctx = make_ctx();
      expect(fsm.handle_event(Event::Play, ctx)).toBeTruthy();
      expect(fsm.current() == State::MainMenu).toBeTruthy();
    });

    it("should return false for unhandled event", {
      GameAK::FSM::StateMachine<State, Event> fsm{State::Root};
      auto ctx = make_ctx();
      expect(fsm.handle_event(Event::Play, ctx)).toBeFalsy();
      expect(fsm.current() == State::Root).toBeTruthy();
    });

    it("should respect guard condition", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Menu);
      fsm.add_state({State::Playing, State::Menu, State::Playing});
      fsm.add_transition(State::Menu, Event::Play, State::Playing, always_false);
      auto ctx = make_ctx();
      expect(fsm.handle_event(Event::Play, ctx)).toBeFalsy();
      expect(fsm.current() == State::Menu).toBeTruthy();
    });

    it("should pass guard with true predicate", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Menu);
      fsm.add_state({State::Playing, State::Menu, State::Playing});
      fsm.add_transition(State::Menu, Event::Play, State::Playing, always_true);
      auto ctx = make_ctx();
      expect(fsm.handle_event(Event::Play, ctx)).toBeTruthy();
      expect(fsm.current() == State::Playing).toBeTruthy();
    });

    it("should call on_enter and on_exit", {
      g_enter_count = 0;
      g_exit_count = 0;
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::Menu});
      fsm.add_state({State::Playing, State::Root, State::Playing});
      fsm.add_transition(State::Menu, Event::Play, State::Playing);
      fsm.set_on_enter(State::Playing, track_enter);
      fsm.set_on_exit(State::Menu, track_exit);
      auto ctx = make_ctx();
      fsm.transition_to(State::Menu, ctx);
      fsm.handle_event(Event::Play, ctx);
      expect(g_enter_count).toBe(1);
      expect(g_exit_count).toBe(1);
    });

    it("should call on_update only for active state", {
      g_update_calls = 0;
      GameAK::FSM::StateMachine<State, Event> fsm(State::Menu);
      fsm.add_state({State::Playing, State::Menu, State::Playing});
      fsm.set_on_update(State::Menu, track_update);
      fsm.set_on_update(State::Playing, track_update);
      auto ctx = make_ctx();
      fsm.update(ctx);
      expect(g_update_calls).toBe(1);
    });

    it("[Edge] transition to self is no-op", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Menu);
      auto ctx = make_ctx();
      fsm.transition_to(State::Menu, ctx);
      expect(fsm.current() == State::Menu).toBeTruthy();
    });
  });

  describe("GameAK::FSM::StateMachine (HFSM)", {
    it("should auto-enter initial child when transitioning to parent", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::MainMenu});
      fsm.add_state({State::MainMenu, State::Menu, State::MainMenu});
      auto ctx = make_ctx();
      fsm.transition_to(State::Menu, ctx);
      expect(fsm.current() == State::MainMenu).toBeTruthy();
      expect(fsm.is_active(State::MainMenu)).toBeTruthy();
      expect(fsm.is_active(State::Menu)).toBeTruthy();
      expect(fsm.is_active(State::Root)).toBeTruthy();
    });

    it("should bubble event from leaf to parent", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::MainMenu});
      fsm.add_state({State::MainMenu, State::Menu, State::MainMenu});
      fsm.add_state({State::Playing, State::Root, State::Alive});
      fsm.add_state({State::Alive, State::Playing, State::Alive});
      fsm.add_state({State::Dead, State::Playing, State::Dead});
      fsm.add_transition(State::Playing, Event::Die, State::Dead);
      auto ctx = make_ctx();
      fsm.transition_to(State::Alive, ctx);
      expect(fsm.current() == State::Alive).toBeTruthy();
      expect(fsm.handle_event(Event::Die, ctx)).toBeTruthy();
      expect(fsm.current() == State::Dead).toBeTruthy();
    });

    it("should bubble unhandled event up to ancestor", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::MainMenu});
      fsm.add_state({State::MainMenu, State::Menu, State::MainMenu});
      fsm.add_state({State::Playing, State::Root, State::Alive});
      fsm.add_state({State::Alive, State::Playing, State::Alive});
      fsm.add_transition(State::Playing, Event::Die, State::Dead);
      fsm.add_state({State::Dead, State::Playing, State::Dead});

      auto ctx = make_ctx();
      fsm.transition_to(State::MainMenu, ctx);
      expect(fsm.current() == State::MainMenu).toBeTruthy();
      fsm.add_transition(State::Root, Event::Play, State::Playing);
      expect(fsm.handle_event(Event::Play, ctx)).toBeTruthy();
      expect(fsm.current() == State::Alive).toBeTruthy();
    });

    it("should compute LCA for sibling transitions", {
      g_enter_count = 0;
      g_exit_count = 0;
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::Settings});
      fsm.add_state({State::MainMenu, State::Menu, State::MainMenu});
      fsm.add_state({State::Settings, State::Menu, State::Settings});

      fsm.set_on_enter(State::Settings, track_enter);
      fsm.set_on_exit(State::MainMenu, track_exit);

      auto ctx = make_ctx();
      fsm.transition_to(State::MainMenu, ctx);
      expect(fsm.current() == State::MainMenu).toBeTruthy();

      // MainMenu -> Settings: LCA is Menu, exit MainMenu, enter Settings
      fsm.add_transition(State::MainMenu, Event::OpenSettings, State::Settings);
      fsm.handle_event(Event::OpenSettings, ctx);
      expect(fsm.current() == State::Settings).toBeTruthy();
      expect(g_exit_count).toBe(1);
      expect(g_enter_count).toBe(1);
      expect(fsm.is_active(State::Menu)).toBeTruthy();
      expect(fsm.is_active(State::Root)).toBeTruthy();
    });

    it("should transition between unrelated subtrees", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::MainMenu});
      fsm.add_state({State::MainMenu, State::Menu, State::MainMenu});
      fsm.add_state({State::Playing, State::Root, State::Alive});
      fsm.add_state({State::Alive, State::Playing, State::Alive});

      auto ctx = make_ctx();
      fsm.transition_to(State::MainMenu, ctx);
      fsm.add_transition(State::Root, Event::Play, State::Playing);
      fsm.handle_event(Event::Play, ctx);
      expect(fsm.current() == State::Alive).toBeTruthy();
      expect(fsm.is_active(State::Playing)).toBeTruthy();
      expect(fsm.is_active(State::Alive)).toBeTruthy();
      expect(fsm.is_active(State::MainMenu)).toBeFalsy();
      expect(fsm.is_active(State::Menu)).toBeFalsy();
    });

    it("should call on_update for entire active chain", {
      g_update_calls = 0;
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::MainMenu});
      fsm.add_state({State::MainMenu, State::Menu, State::MainMenu});
      fsm.add_state({State::Playing, State::Root, State::Alive});
      fsm.add_state({State::Alive, State::Playing, State::Alive});

      fsm.set_on_update(State::Root, track_update);
      fsm.set_on_update(State::Menu, track_update);
      fsm.set_on_update(State::MainMenu, track_update);
      fsm.set_on_update(State::Playing, track_update);
      fsm.set_on_update(State::Alive, track_update);

      auto ctx = make_ctx();
      fsm.transition_to(State::MainMenu, ctx);
      g_update_calls = 0;
      fsm.update(ctx);
      expect(g_update_calls).toBe(3);
    });

    it("[Edge] should not crash when add_state for existing id", {
      GameAK::FSM::StateMachine<State, Event> fsm(State::Root);
      fsm.add_state({State::Menu, State::Root, State::Menu});
      fsm.add_state({State::Menu, State::Root, State::Menu});
      auto ctx = make_ctx();
      fsm.transition_to(State::Menu, ctx);
      expect(fsm.is_active(State::Menu)).toBeTruthy();
    });
  });

  return cest_result();
}
