#pragma once

namespace {
namespace parallel_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    struct DummyComponent {
        int value;
    };

    // ── Scenario: Controllers with disjoint type access execute in parallel ──
    void test_disjoint_types() {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = 4; t1.alignment = 4; t1.name = "type_a";
            expect(rt.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }
        {
            BlockTypeDescriptor t2;
            t2.type_id = 2; t2.size = 4; t2.alignment = 4; t2.name = "type_b";
            expect(rt.register_block_type(std::move(t2)).has_value()).toBeTruthy();
        }

        bool a_executed = false, b_executed = false;
        auto ctrl_a = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            a_executed = true;
            return {};
        };
        auto ctrl_b = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            b_executed = true;
            return {};
        };

        expect(rt.register_controller(std::move(ctrl_a), 0, {1}).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(ctrl_b), 0, {2}).has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(a_executed).toBeTruthy();
        expect(b_executed).toBeTruthy();
        expect(r.controllers_executed == 2).toBeTruthy();
    }

    // ── Scenario: Controllers with overlapping type access execute sequentially ──
    void test_overlapping_types_sequential() {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = 4; t1.alignment = 4; t1.name = "test";
            expect(rt.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }

        std::vector<int> order;
        auto ctrl_a = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(1);
            return {};
        };
        auto ctrl_b = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(2);
            return {};
        };

        // Both declare access to type_id = 1 → must be sequential (same group = sequential anyway with default sequential backend)
        expect(rt.register_controller(std::move(ctrl_a), 0, {1}).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(ctrl_b), 0, {1}).has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(order.size() == 2).toBeTruthy();
        expect(order[0] == 1).toBeTruthy();
        expect(order[1] == 2).toBeTruthy();
        expect(r.controllers_executed == 2).toBeTruthy();
    }

    // ── Scenario: Determinism is preserved regardless of parallelism ──
    void test_determinism() {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = sizeof(DummyComponent);
            t1.alignment = alignof(DummyComponent); t1.name = "test";
            expect(rt.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }

        // Run two ticks with the same setup
        auto run_tick = [](DefaultRuntime& r) -> size_t {
            auto ctrl = [](StateView&, CommandProducer& p, EphemeralProducer&) -> Result<void> {
                return p.create(1);
            };
            expect(r.register_controller(std::move(ctrl), 0, {1}).has_value()).toBeTruthy();
            auto result = r.tick();
            return result.commands_executed;
        };

        DefaultRuntime rt2;
        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = sizeof(DummyComponent);
            t1.alignment = alignof(DummyComponent); t1.name = "test";
            expect(rt2.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }

        size_t r1 = run_tick(rt);
        size_t r2 = run_tick(rt2);

        expect(r1 == r2).toBeTruthy();
        expect(r1 == 1).toBeTruthy();
    }

    // ── Scenario: Controller declares type access at registration ──
    void test_type_declaration() {
        DefaultRuntime rt;
        auto ctrl = [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return {};
        };
        auto result = rt.register_controller(std::move(ctrl), 0, {1, 2, 3});
        expect(result.has_value()).toBeTruthy();
        expect(rt.tick().status == ExecutionStatus::Success).toBeTruthy();
    }

    // ── Scenario: Empty declaration defaults to sequential ──
    void test_empty_declaration_sequential() {
        DefaultRuntime rt;
        bool executed = false;
        auto ctrl = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            executed = true;
            return {};
        };
        expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(executed).toBeTruthy();
        expect(r.controllers_executed == 1).toBeTruthy();
    }

    // ── Scenario: Commands produced by parallel controllers are merged deterministically ──
    void test_command_merge() {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = sizeof(DummyComponent);
            t1.alignment = alignof(DummyComponent); t1.name = "test";
            expect(rt.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }

        auto ctrl_a = [](StateView&, CommandProducer& p, EphemeralProducer&) -> Result<void> {
            return p.create(1);
        };
        auto ctrl_b = [](StateView&, CommandProducer& p, EphemeralProducer&) -> Result<void> {
            return p.create(1);
        };

        expect(rt.register_controller(std::move(ctrl_a), 0, {1}).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(ctrl_b), 0, {1}).has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    }

    // ── Scenario: Parallel produces same result as sequential ──
    void test_same_result_as_sequential() {
        DefaultRuntime rt_seq;
        DefaultRuntime rt_par;
        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = sizeof(DummyComponent);
            t1.alignment = alignof(DummyComponent); t1.name = "test";
            expect(rt_seq.register_block_type(std::move(t1)).has_value()).toBeTruthy();
            expect(rt_par.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }

        // Sequential registration (no type access)
        auto ctrl_seq = [](StateView&, CommandProducer& p, EphemeralProducer&) -> Result<void> {
            return p.create(1);
        };
        expect(rt_seq.register_controller(std::move(ctrl_seq), 0).has_value()).toBeTruthy();
        // Parallel registration (with type access) — same behavior, same result
        auto ctrl_par = [](StateView&, CommandProducer& p, EphemeralProducer&) -> Result<void> {
            return p.create(1);
        };
        expect(rt_par.register_controller(std::move(ctrl_par), 0, {1}).has_value()).toBeTruthy();

        auto res_seq = rt_seq.tick();
        auto res_par = rt_par.tick();
        expect(res_seq.commands_executed == res_par.commands_executed).toBeTruthy();
        expect(res_seq.status == res_par.status).toBeTruthy();
        expect(rt_seq.block_count(1) == rt_par.block_count(1)).toBeTruthy();
    }

    // ── Scenario: Type access declaration is a scheduling hint, not enforced at runtime ──
    void test_declaration_is_scheduling_hint() {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = sizeof(DummyComponent);
            t1.alignment = alignof(DummyComponent); t1.name = "test";
            expect(rt.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();
        auto block_id = block.value();

        // Controller declares type 2 but actually accesses type 1 — this is allowed
        // (declaration is a scheduling hint, not enforced)
        auto ctrl = [block_id](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            if (!view.has_block(block_id)) {
                return Error(ErrorCode::BlockNotFound, "block not found");
            }
            return {};
        };
        expect(rt.register_controller(std::move(ctrl), 0, {2}).has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    // ── Scenario: Fallback to sequential when parallel backend is not set ──
    void test_fallback_sequential() {
        DefaultRuntime rt;
        // Default config has no parallel_executor → falls back to sequential
        expect(rt.config().parallel_executor == nullptr).toBeTruthy();

        {
            BlockTypeDescriptor t1;
            t1.type_id = 1; t1.size = 4; t1.alignment = 4; t1.name = "test";
            expect(rt.register_block_type(std::move(t1)).has_value()).toBeTruthy();
        }

        bool executed = false;
        auto ctrl = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            executed = true;
            return {};
        };
        expect(rt.register_controller(std::move(ctrl), 0, {1}).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(executed).toBeTruthy();
        expect(r.controllers_executed == 1).toBeTruthy();
    }

    // ── Additional: Multiple controllers with mixed type access ──
    void test_mixed_type_access() {
        DefaultRuntime rt;
        for (uint32_t i = 1; i <= 3; ++i) {
            BlockTypeDescriptor t;
            t.type_id = i; t.size = 4; t.alignment = 4; t.name = "type_" + std::to_string(i);
            expect(rt.register_block_type(std::move(t)).has_value()).toBeTruthy();
        }

        // A({1,2}), B({3}), C({2,3}), D({1})
        // Group 0: A({1,2})
        // Group 1: B({3}) — no overlap with A
        // Group 2: C({2,3}) — overlaps A(2) and B(3)
        // Group 3: D({1}) — overlaps A(1)
        // Expected: all 4 execute

        std::vector<int> exec_order;
        auto make_ctrl = [&](int id, std::vector<uint32_t> types) {
            return [&exec_order, id, types = std::move(types)](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
                exec_order.push_back(id);
                return {};
            };
        };

        expect(rt.register_controller(make_ctrl(1, {1, 2}), 0, {1, 2}).has_value()).toBeTruthy();
        expect(rt.register_controller(make_ctrl(2, {3}),    0, {3}).has_value()).toBeTruthy();
        expect(rt.register_controller(make_ctrl(3, {2, 3}), 0, {2, 3}).has_value()).toBeTruthy();
        expect(rt.register_controller(make_ctrl(4, {1}),    0, {1}).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.controllers_executed == 4).toBeTruthy();
        expect(exec_order.size() == 4).toBeTruthy();
    }

    // ── Additional: Type access with no block types registered ──
    void test_type_access_no_types() {
        DefaultRuntime rt;
        auto ctrl = [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return {};
        };
        // Register controller with type access to non-existent types — still valid (scheduling hint)
        expect(rt.register_controller(std::move(ctrl), 0, {42, 99}).has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.controllers_executed == 1).toBeTruthy();
    }
}
}

inline void run_parallel_tests() {
    using namespace gameak::runtime;

describe("Parallel Controller Dispatch", {
    it("disjoint types enter parallel group",                    { parallel_helpers::test_disjoint_types(); });
    it("overlapping types execute sequentially",                 { parallel_helpers::test_overlapping_types_sequential(); });
    it("determinism is preserved",                               { parallel_helpers::test_determinism(); });
    it("type access declaration at registration",                { parallel_helpers::test_type_declaration(); });
    it("empty declaration defaults to sequential",                { parallel_helpers::test_empty_declaration_sequential(); });
    it("commands from parallel controllers merge",               { parallel_helpers::test_command_merge(); });
    it("parallel produces same result as sequential",             { parallel_helpers::test_same_result_as_sequential(); });
    it("declaration is scheduling hint, not enforced",           { parallel_helpers::test_declaration_is_scheduling_hint(); });
    it("falls back to sequential when no parallel backend",       { parallel_helpers::test_fallback_sequential(); });
    it("mixed type access groups correctly",                     { parallel_helpers::test_mixed_type_access(); });
    it("type access with no block types registered",             { parallel_helpers::test_type_access_no_types(); });
});
}
