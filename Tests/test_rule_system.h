#pragma once

namespace {
namespace rule_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_matching_condition_fires() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "hp";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        int fires = 0;
        RuleSystem rs;
        rs.add_rule("destroy_dead",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer& producer, EphemeralProducer&) { fires++; auto _ = producer.produce(Command{CommandCreateBlock{1}}); (void)_; },
            10);

        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(fires == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    }

    void test_non_matching_does_not_fire() {
        int fires = 0;
        RuleSystem rs;
        rs.add_rule("never_fires",
            [](StateView&) { return false; },
            [&](StateView&, CommandProducer&, EphemeralProducer&) { fires++; },
            0);

        DefaultRuntime rt;
        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(fires == 0).toBeTruthy();
    }

    void test_multiple_rules_fire() {
        int fire_a = 0;
        int fire_b = 0;
        RuleSystem rs;
        rs.add_rule("rule_a",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer&, EphemeralProducer&) { fire_a++; },
            0);
        rs.add_rule("rule_b",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer&, EphemeralProducer&) { fire_b++; },
            1);

        DefaultRuntime rt;
        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(fire_a == 1).toBeTruthy();
        expect(fire_b == 1).toBeTruthy();
    }

    void test_priority_order() {
        std::vector<int> order;
        RuleSystem rs;
        rs.add_rule("low",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer&, EphemeralProducer&) { order.push_back(1); },
            -10);
        rs.add_rule("mid",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer&, EphemeralProducer&) { order.push_back(2); },
            0);
        rs.add_rule("high",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer&, EphemeralProducer&) { order.push_back(3); },
            10);

        DefaultRuntime rt;
        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(order.size() == 3).toBeTruthy();
        expect(order[0] == 3).toBeTruthy();
        expect(order[1] == 2).toBeTruthy();
        expect(order[2] == 1).toBeTruthy();
    }

    void test_action_produces_command() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        RuleSystem rs;
        rs.add_rule("spawn",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer& producer, EphemeralProducer&) { auto _ = producer.produce(Command{CommandCreateBlock{1}}); (void)_; },
            0);

        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    }

    void test_empty_rule_system() {
        RuleSystem rs;
        DefaultRuntime rt;
        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();
        expect(rs.rule_count() == 0).toBeTruthy();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_rule_system_is_controller() {
        RuleSystem rs;
        Controller c = rs.build();
        expect(static_cast<bool>(c)).toBeTruthy();
    }

    void test_dynamic_add_remove() {
        int fires = 0;
        RuleSystem rs;
        auto id = rs.add_rule("temp",
            [](StateView&) { return true; },
            [&](StateView&, CommandProducer&, EphemeralProducer&) { fires++; },
            0);
        expect(rs.rule_count() == 1).toBeTruthy();
        rs.remove_rule(id);
        expect(rs.rule_count() == 0).toBeTruthy();

        DefaultRuntime rt;
        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(fires == 0).toBeTruthy();
    }
}
}

inline void run_rule_system_tests() {
describe("RuleSystem", {
    it("matching condition fires",          { rule_helpers::test_matching_condition_fires(); });
    it("non-matching does not fire",        { rule_helpers::test_non_matching_does_not_fire(); });
    it("multiple rules fire",               { rule_helpers::test_multiple_rules_fire(); });
    it("priority order",                    { rule_helpers::test_priority_order(); });
    it("action produces command",           { rule_helpers::test_action_produces_command(); });
    it("empty system",                      { rule_helpers::test_empty_rule_system(); });
    it("is a Controller",                   { rule_helpers::test_rule_system_is_controller(); });
    it("dynamic add/remove",                { rule_helpers::test_dynamic_add_remove(); });
});
}
