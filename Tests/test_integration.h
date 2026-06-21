#pragma once

inline void run_integration_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Integration", {

    it("all four composable controllers execute in same tick", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        int pipeline_count = 0;
        Pipeline pipe;
        pipe.add_stage("a", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            pipeline_count++;
            return {};
        });
        auto pipe_ctrl = pipe.build();

        int loop_count = 0;
        EventLoop<int> loop;
        loop.on(1, [&](const int&, CommandProducer&, EphemeralProducer&) {
            loop_count++;
        });
        loop.enqueue(1, 42);
        auto loop_ctrl = loop.build();

        int rule_count = 0;
        RuleSystem rules;
        rules.add_rule(RuleDef{
            .name = "always_true",
            .condition = [](StateView&) { return true; },
            .action = [&](StateView&, CommandProducer&, EphemeralProducer&) {
                rule_count++;
            },
            .priority = 0
        });
        auto rules_ctrl = rules.build();

        auto fsm_ctrl = Fsm<>{}
            .initial_state("idle")
            .add_state("idle")
            .add_state("attacking")
            .add_transition("idle", "tick", "attacking")
            .build();

        expect(rt.register_controller(std::move(pipe_ctrl), 10).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(loop_ctrl), 0).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(rules_ctrl), -10).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(fsm_ctrl), -20).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.controllers_executed == 4).toBeTruthy();
        expect(pipeline_count == 1).toBeTruthy();
        expect(loop_count == 1).toBeTruthy();
        expect(rule_count == 1).toBeTruthy();
    });

    it("ephemeral blocks do not fire BlockCreated event", {
        DefaultRuntime rt;
        BlockTypeDescriptor persistent;
        persistent.type_id = 1; persistent.size = sizeof(int); persistent.alignment = alignof(int);
        persistent.name = "persistent"; persistent.ephemeral = false;
        expect(rt.register_block_type(std::move(persistent)).has_value()).toBeTruthy();

        BlockTypeDescriptor ephemeral;
        ephemeral.type_id = 2; ephemeral.size = sizeof(int); ephemeral.alignment = alignof(int);
        ephemeral.name = "ephemeral"; ephemeral.ephemeral = true;
        expect(rt.register_block_type(std::move(ephemeral)).has_value()).toBeTruthy();

        int created_count = 0;
        int destroyed_count = 0;
        rt.listen(Runtime<FifoScheduler>::EventType::BlockCreated,
            [&](const Runtime<FifoScheduler>::Event&) { created_count++; });
        rt.listen(Runtime<FifoScheduler>::EventType::BlockDestroyed,
            [&](const Runtime<FifoScheduler>::Event&) { destroyed_count++; });

        auto ctrl = [](StateView&, CommandProducer& producer, EphemeralProducer& ephem) -> Result<void> {
            auto _e = ephem.create(2); (void)_e;
            auto r = producer.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(created_count == 1).toBeTruthy();
        expect(destroyed_count == 0).toBeTruthy();
    });

    it("ephemeral block relationships work within tick", {
        DefaultRuntime rt;
        BlockTypeDescriptor persistent;
        persistent.type_id = 1; persistent.size = sizeof(int); persistent.alignment = alignof(int);
        persistent.name = "persistent";
        expect(rt.register_block_type(std::move(persistent)).has_value()).toBeTruthy();

        BlockTypeDescriptor ephemeral;
        ephemeral.type_id = 2; ephemeral.size = sizeof(int); ephemeral.alignment = alignof(int);
        ephemeral.name = "ephemeral"; ephemeral.ephemeral = true;
        expect(rt.register_block_type(std::move(ephemeral)).has_value()).toBeTruthy();

        Identity persistent_id;
        Identity ephemeral_id;
        bool related_ok = false;

        auto creator = [&](StateView&, CommandProducer&, EphemeralProducer& ephem) -> Result<void> {
            auto p = rt.create_block(1);
            if (!p) return p.error();
            persistent_id = p.value();
            auto e = ephem.create(2);
            if (!e) return e.error();
            ephemeral_id = e.value();
            auto rel = rt.relate(persistent_id, ephemeral_id);
            if (!rel) return rel.error();
            related_ok = true;
            return {};
        };
        expect(rt.register_controller(std::move(creator), 10).has_value()).toBeTruthy();

        auto checker = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            if (related_ok && persistent_id.is_valid() && ephemeral_id.is_valid()) {
                auto children = rt.children_of(persistent_id);
                expect(children.size() == 1).toBeTruthy();
                expect(children[0] == ephemeral_id).toBeTruthy();
            }
            return {};
        };
        expect(rt.register_controller(std::move(checker), 0).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(related_ok).toBeTruthy();
    });

    it("SoA layout conversion preserves block relationships", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int);
        desc.name = "test"; desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto parent = rt.create_block(1);
        auto child = rt.create_block(1);
        expect(parent.has_value() && child.has_value()).toBeTruthy();
        expect(rt.relate(parent.value(), child.value()).has_value()).toBeTruthy();

        rt.convert_to_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();

        auto children_soa = rt.children_of(parent.value());
        expect(children_soa.size() == 1).toBeTruthy();
        expect(children_soa[0] == child.value()).toBeTruthy();

        rt.convert_from_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();

        auto children_aos = rt.children_of(parent.value());
        expect(children_aos.size() == 1).toBeTruthy();
        expect(children_aos[0] == child.value()).toBeTruthy();
    });

    it("save and load preserves layout state", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int) * 4; desc.alignment = alignof(int);
        desc.name = "test"; desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        rt.convert_to_soa(1);
        auto snap_soa = rt.save();
        expect(snap_soa.blocks.size() == 0).toBeTruthy();

        rt.convert_from_soa(1);
        auto snap_aos = rt.save();
        expect(snap_aos.blocks.size() == 2).toBeTruthy();

        DefaultRuntime rt2;
        BlockTypeDescriptor desc2;
        desc2.type_id = 1; desc2.size = sizeof(int) * 4; desc2.alignment = alignof(int); desc2.name = "test";
        expect(rt2.register_block_type(std::move(desc2)).has_value()).toBeTruthy();
        rt2.load(std::move(snap_aos));
        expect(rt2.block_count(1) == 2).toBeTruthy();
    });

    it("save and load preserves block identities (relationships not serialized)", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto p = rt.create_block(1);
        auto c = rt.create_block(1);
        expect(p.has_value() && c.has_value()).toBeTruthy();
        expect(rt.relate(p.value(), c.value()).has_value()).toBeTruthy();

        auto snap = rt.save();
        expect(rt.children_of(p.value()).size() == 1).toBeTruthy();

        DefaultRuntime rt2;
        BlockTypeDescriptor desc2;
        desc2.type_id = 1; desc2.size = sizeof(int); desc2.alignment = alignof(int); desc2.name = "test";
        expect(rt2.register_block_type(std::move(desc2)).has_value()).toBeTruthy();
        rt2.load(std::move(snap));

        expect(rt2.has_block(p.value())).toBeTruthy();
        expect(rt2.has_block(c.value())).toBeTruthy();
    });

    it("mixed direct commands and controller-produced commands execute together", {
        using PriorityRuntime = Runtime<PriorityScheduler>;
        PriorityRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        rt.scheduler().set_priority_fn([](const Command&) { return 0; });

        int ctrl_exec = 0;
        auto ctrl = [&](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
            ctrl_exec++;
            auto r = producer.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.controllers_executed == 1).toBeTruthy();
        expect(r.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
        expect(ctrl_exec == 1).toBeTruthy();
    });

    it("fixed timestep with pause only processes after resume", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        int ctrl_count = 0;
        auto ctrl = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            ctrl_count++;
            return {};
        };
        expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();

        rt.set_fixed_timestep(0.01f);
        rt.pause();

        auto r1 = rt.tick(0.025f);
        expect(r1.controllers_executed == 0).toBeTruthy();
        expect(ctrl_count == 0).toBeTruthy();

        rt.resume();
        auto r2 = rt.tick(0.025f);
        expect(r2.controllers_executed == 2).toBeTruthy();
        expect(ctrl_count == 2).toBeTruthy();
    });
});
}
