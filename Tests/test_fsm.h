#pragma once

namespace {
namespace fsm_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_fsm_starts_initial() {
        auto fsm = FsmBuilder{}
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

        auto fsm = FsmBuilder{}
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
        auto fsm = FsmBuilder{}
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
        auto fsm = FsmBuilder{}
            .initial_state("A")
            .add_state("A")
            .add_state("B")
            .build();
        Controller c = std::move(fsm);
        expect(static_cast<bool>(c)).toBeTruthy();
    }

    void test_fsm_explicit_transitions() {
        auto fsm = FsmBuilder{}
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
}
}

inline void run_fsm_tests() {
describe("FSM", {
    it("starts in initial state",           { fsm_helpers::test_fsm_starts_initial(); });
    it("entry/exit actions",                { fsm_helpers::test_fsm_entry_exit_actions(); });
    it("ignores undefined transitions",     { fsm_helpers::test_fsm_ignores_undefined(); });
    it("is a Controller",                   { fsm_helpers::test_fsm_is_controller(); });
    it("explicit transitions only",         { fsm_helpers::test_fsm_explicit_transitions(); });
});
}
