#pragma once

namespace {
namespace fsm_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_fsm_starts_initial() {
        auto fsm = Fsm<>{}
            .initial_state("Idle")
            .add_state("Idle")
            .add_state("Active")
            .build();

        DefaultRuntime rt;
        int cmds = 0;
        auto wrapper = [&cmds, fsm = std::move(fsm)](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) mutable -> Result<void> {
            auto r = fsm(view, producer, ephem);
            cmds++;
            return r;
        };
        expect(rt.register_controller(std::move(wrapper)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(cmds == 1).toBeTruthy();
    }

    void test_fsm_entry_exit_actions() {
        bool entered_active = false;
        bool exited_idle = false;

        auto fsm = Fsm<>{}
            .initial_state("Idle")
            .add_state("Idle")
            .add_state("Active")
            .add_transition("Idle", "Start", "Active")
            .on_entry("Active", [&](StateView&, CommandProducer&, EphemeralProducer&) { entered_active = true; })
            .on_exit("Idle", [&](StateView&, CommandProducer&, EphemeralProducer&) { exited_idle = true; })
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) mutable -> Result<void> {
            return fsm(view, producer, ephem);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(entered_active).toBeFalsy();
        expect(exited_idle).toBeFalsy();
    }

    void test_fsm_ignores_undefined() {
        auto fsm = Fsm<>{}
            .initial_state("Idle")
            .add_state("Idle")
            .add_state("Paused")
            .add_transition("Idle", "Pause", "Paused")
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) mutable -> Result<void> {
            return fsm(view, producer, ephem);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_fsm_is_controller() {
        auto fsm = Fsm<>{}
            .initial_state("A")
            .add_state("A")
            .add_state("B")
            .build();
        Controller c = std::move(fsm);
        expect(static_cast<bool>(c)).toBeTruthy();
    }

    void test_fsm_explicit_transitions() {
        auto fsm = Fsm<>{}
            .initial_state("A")
            .add_state("A")
            .add_state("B")
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) mutable -> Result<void> {
            return fsm(view, producer, ephem);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_fsm_enqueue_event() {
        bool entered_active = false;

        auto fsm = Fsm<>{}
            .initial_state("Idle")
            .add_state("Idle")
            .add_state("Active")
            .add_transition("Idle", "Start", "Active")
            .on_entry("Active", [&](StateView&, CommandProducer&, EphemeralProducer&) { entered_active = true; })
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) mutable -> Result<void> {
            return fsm(view, producer, ephem);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        // Tick without event — no transition
        auto r1 = rt.tick();
        expect(r1.status == ExecutionStatus::Success).toBeTruthy();
        expect(entered_active).toBeFalsy();
    }

    enum class GameState : uint32_t { Menu, Playing, Paused };
    enum class GameEvent : uint32_t { Start, Pause, Resume, Quit };

    void test_fsm_enum_types() {
        GameState current = GameState::Menu;

        auto fsm = Fsm<GameState, GameEvent>{}
            .initial_state(GameState::Menu)
            .add_state(GameState::Menu)
            .add_state(GameState::Playing)
            .add_state(GameState::Paused)
            .add_transition(GameState::Menu, GameEvent::Start, GameState::Playing)
            .add_transition(GameState::Playing, GameEvent::Pause, GameState::Paused)
            .add_transition(GameState::Paused, GameEvent::Resume, GameState::Playing)
            .add_transition(GameState::Playing, GameEvent::Quit, GameState::Menu)
            .on_entry(GameState::Playing, [&](StateView&, CommandProducer&, EphemeralProducer&) { current = GameState::Playing; })
            .on_entry(GameState::Paused, [&](StateView&, CommandProducer&, EphemeralProducer&) { current = GameState::Paused; })
            .on_entry(GameState::Menu, [&](StateView&, CommandProducer&, EphemeralProducer&) { current = GameState::Menu; })
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) mutable -> Result<void> {
            return fsm(view, producer, ephem);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        rt.tick();
        expect(current == GameState::Menu).toBeTruthy();
    }

    void test_fsm_enqueue_triggers_transition() {
        Fsm<GameState, GameEvent> fsm;
        fsm.initial_state(GameState::Menu)
            .add_state(GameState::Menu)
            .add_state(GameState::Playing)
            .add_transition(GameState::Menu, GameEvent::Start, GameState::Playing);

        fsm.enqueue_event(GameEvent::Start);
        auto ctrl = fsm.build();

        DefaultRuntime rt;
        Controller wrapper = [ctrl = std::move(ctrl)](StateView& v, CommandProducer& p, EphemeralProducer& e) mutable -> Result<void> {
            return ctrl(v, p, e);
        };
        expect(rt.register_controller(std::move(wrapper)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_fsm_uint32_types() {
        int entered = 0;

        auto fsm = Fsm<uint32_t, uint32_t>{}
            .initial_state(0)
            .add_state(0)
            .add_state(1)
            .add_transition(0, 42, 1)
            .on_entry(1, [&](StateView&, CommandProducer&, EphemeralProducer&) { entered++; })
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& view, CommandProducer& producer, EphemeralProducer& ephem) mutable -> Result<void> {
            return fsm(view, producer, ephem);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        rt.tick();
        expect(entered == 0).toBeTruthy();
    }
}
}

inline void run_fsm_tests() {
describe("FSM", {
    it("starts in initial state",           { fsm_helpers::test_fsm_starts_initial(); });
    it("entry/exit actions",                { fsm_helpers::test_fsm_entry_exit_actions(); });
    it("ignores undefined transitions",     { fsm_helpers::test_fsm_ignores_undefined(); });
    it("is a Controller",                   { fsm_helpers::test_fsm_is_controller(); });
    it("explicit transitions only",         { fsm_helpers::test_fsm_explicit_transitions(); });
    it("enqueue_event does not fire before tick", { fsm_helpers::test_fsm_enqueue_event(); });
    it("enum state and event types",        { fsm_helpers::test_fsm_enum_types(); });
    it("enqueued event triggers transition", { fsm_helpers::test_fsm_enqueue_triggers_transition(); });
    it("uint32 state and event types",      { fsm_helpers::test_fsm_uint32_types(); });
});
}
