#pragma once

inline void run_e2e_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("E2E", {

    it("full game loop with multiple controller types", {
        DefaultRuntime rt;

        BlockTypeDescriptor player;
        player.type_id = 1; player.size = sizeof(int) * 4; player.alignment = alignof(int);
        player.name = "player";
        expect(rt.register_block_type(player).has_value()).toBeTruthy();

        BlockTypeDescriptor enemy;
        enemy.type_id = 2; enemy.size = sizeof(int) * 4; enemy.alignment = alignof(int);
        enemy.name = "enemy";
        expect(rt.register_block_type(enemy).has_value()).toBeTruthy();

        BlockTypeDescriptor damage_signal;
        damage_signal.type_id = 3; damage_signal.size = sizeof(int); damage_signal.alignment = alignof(int);
        damage_signal.name = "damage_signal"; damage_signal.ephemeral = true;
        expect(rt.register_block_type(damage_signal).has_value()).toBeTruthy();

        auto player_id = rt.create_block(1);
        auto enemy_id = rt.create_block(2);
        expect(player_id.has_value() && enemy_id.has_value()).toBeTruthy();

        // Combat FSM: player idle/attacking states
        FsmBuilder fsm_builder;
        fsm_builder.initial_state("idle")
            .add_state("idle")
            .add_state("attacking")
            .add_transition("idle", "start_attack", "attacking")
            .add_transition("attacking", "end_attack", "idle");
        auto combat_fsm = fsm_builder.build();
        expect(rt.register_controller(std::move(combat_fsm), 10).has_value()).toBeTruthy();

        // Event loop for damage processing
        int damage_processed = 0;
        EventLoop<int> damage_loop;
        damage_loop.on(1, [&](const int& dmg, CommandProducer& prod, EphemeralProducer& ephem) {
            damage_processed += dmg;
            auto _e = ephem.create(3); (void)_e;
            auto r = prod.produce(Command{CommandCreateBlock{2}});
            (void)r;
        });
        auto loop_ctrl = damage_loop.build();
        expect(rt.register_controller(std::move(loop_ctrl), 0).has_value()).toBeTruthy();

        // Rule system for health monitoring
        RuleSystem rules;
        rules.add_rule(RuleDef{
            .name = "health_monitor",
            .condition = [&](StateView& view) {
                return view.has_block(player_id.value());
            },
            .action = [&](StateView&, CommandProducer&, EphemeralProducer&) {
                damage_loop.enqueue(1, 10);
            },
            .priority = 0
        });
        auto rules_ctrl = rules.build();
        expect(rt.register_controller(std::move(rules_ctrl), -10).has_value()).toBeTruthy();

        // Run 20 ticks simulating a game loop
        for (int tick = 0; tick < 20; ++tick) {
            auto r = rt.tick(0.016f);
            expect(r.status == ExecutionStatus::Success).toBeTruthy();
            expect(r.controllers_executed >= 3).toBeTruthy();
        }

        expect(damage_processed > 0).toBeTruthy();
        expect(rt.block_count(2) >= 10).toBeTruthy();
    });

    it("save, destroy, load, continue — state is consistent", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        int ctrl_count = 0;
        auto ctrl = [&](StateView&, CommandProducer& prod, EphemeralProducer&) -> Result<void> {
            ctrl_count++;
            auto r = prod.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();

        for (int i = 0; i < 5; ++i) rt.tick(0.016f);
        int blocks_after_5 = static_cast<int>(rt.block_count(1));

        auto snap = rt.save();

        for (int i = 0; i < 5; ++i) rt.tick(0.016f);
        int blocks_after_10 = static_cast<int>(rt.block_count(1));

        DefaultRuntime rt2;
        expect(rt2.register_block_type(desc).has_value()).toBeTruthy();
        int ctrl2_count = 0;
        auto ctrl2 = [&](StateView&, CommandProducer& prod, EphemeralProducer&) -> Result<void> {
            ctrl2_count++;
            auto r = prod.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        expect(rt2.register_controller(std::move(ctrl2)).has_value()).toBeTruthy();

        rt2.load(snap);
        expect(rt2.block_count(1) == static_cast<size_t>(blocks_after_5)).toBeTruthy();

        for (int i = 0; i < 5; ++i) rt2.tick(0.016f);
        expect(rt2.block_count(1) == static_cast<size_t>(blocks_after_10)).toBeTruthy();
        expect(ctrl2_count == 5).toBeTruthy();
    });

    it("controller failure does not prevent other controllers from executing", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        int good_count = 0;

        auto failing = [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return Error(ErrorCode::ControllerFailed);
        };
        auto good = [&](StateView&, CommandProducer& prod, EphemeralProducer&) -> Result<void> {
            good_count++;
            auto r = prod.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        auto good2 = [&](StateView&, CommandProducer& prod, EphemeralProducer&) -> Result<void> {
            good_count++;
            auto r = prod.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };

        expect(rt.register_controller(std::move(failing), 10).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(good), 0).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(good2), -10).has_value()).toBeTruthy();

        auto r = rt.tick(0.016f);
        expect(r.status == ExecutionStatus::PartialFailure).toBeTruthy();
        expect(r.controllers_executed == 3).toBeTruthy();
        expect(good_count == 2).toBeTruthy();
        expect(r.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    });

    it("layout migration preserves data across full lifecycle", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int) * 4; desc.alignment = alignof(int);
        desc.name = "test"; desc.layout = LayoutStrategy::AoS;

        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id1 = rt.create_block(1);
        auto id2 = rt.create_block(1);
        expect(id1.has_value() && id2.has_value()).toBeTruthy();

        rt.tick(0.016f);

        // Convert to SoA — blocks move out of blocks_ map
        rt.convert_to_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();
        // SoA blocks are not in blocks_ map, so has_block returns false
        // But find_blocks_by_type still works via SoA storage

        int controller_runs = 0;
        auto ctrl = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            controller_runs++;
            return {};
        };
        expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();

        for (int i = 0; i < 5; ++i) {
            auto r = rt.tick(0.016f);
            expect(r.status == ExecutionStatus::Success).toBeTruthy();
        }
        expect(controller_runs == 5).toBeTruthy();

        // Convert back to AoS — blocks return to blocks_ map
        rt.convert_from_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.has_block(id1.value())).toBeTruthy();
        expect(rt.has_block(id2.value())).toBeTruthy();
        // Tick once to rebuild type_counts_
        rt.tick(0.016f);
        expect(rt.block_count(1) == 2).toBeTruthy();
    });
});
}
