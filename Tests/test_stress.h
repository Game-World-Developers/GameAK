#pragma once

inline void run_stress_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Stress", {

    it("create and destroy 10,000 blocks without leaking identities", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        for (int round = 0; round < 10; ++round) {
            std::vector<Identity> ids;
            ids.reserve(1000);
            for (int i = 0; i < 1000; ++i) {
                auto id = rt.create_block(1);
                expect(id.has_value()).toBeTruthy();
                ids.push_back(id.value());
            }
            for (auto id : ids) {
                expect(rt.destroy_block(id).has_value()).toBeTruthy();
            }
        }
        expect(rt.block_count(1) == 0).toBeTruthy();
        auto diag = rt.collect_diagnostics();
        expect(diag.blocks_count == 0).toBeTruthy();
        expect(diag.next_identity == 10000).toBeTruthy();
    });

    it("create then destroy 1000 blocks each round", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        for (int round = 0; round < 10; ++round) {
            std::vector<Identity> ids;
            ids.reserve(1000);
            for (int i = 0; i < 1000; ++i) {
                auto id = rt.create_block(1);
                expect(id.has_value()).toBeTruthy();
                ids.push_back(id.value());
            }
            expect(rt.block_count(1) == 1000).toBeTruthy();
            for (auto id : ids) {
                expect(rt.destroy_block(id).has_value()).toBeTruthy();
            }
            expect(rt.block_count(1) == 0).toBeTruthy();
        }
        auto diag = rt.collect_diagnostics();
        expect(diag.blocks_count == 0).toBeTruthy();
    });

    it("100 controllers producing commands over 100 ticks", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        constexpr int NUM_CONTROLLERS = 100;
        for (int i = 0; i < NUM_CONTROLLERS; ++i) {
            auto ctrl = [](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
                auto r = producer.produce(Command{CommandCreateBlock{1}});
                if (!r) return r.error();
                return {};
            };
            expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();
        }

        constexpr int NUM_TICKS = 100;
        int total_commands = 0;
        for (int t = 0; t < NUM_TICKS; ++t) {
            auto r = rt.tick(0.016f);
            expect(r.status == ExecutionStatus::Success).toBeTruthy();
            expect(r.controllers_executed == NUM_CONTROLLERS).toBeTruthy();
            expect(r.commands_executed == NUM_CONTROLLERS).toBeTruthy();
            total_commands += r.commands_executed;
        }
        expect(total_commands == NUM_CONTROLLERS * NUM_TICKS).toBeTruthy();
        expect(rt.block_count(1) == NUM_CONTROLLERS * NUM_TICKS).toBeTruthy();
    });

    it("pipeline with 50 stages over 100 ticks", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        constexpr int NUM_STAGES = 50;
        Pipeline pipe;
        for (int i = 0; i < NUM_STAGES; ++i) {
            pipe.add_stage(
                "s" + std::to_string(i),
                [i](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
                    if (i % 5 == 0) {
                        auto r = producer.produce(Command{CommandCreateBlock{1}});
                        if (!r) return r.error();
                    }
                    return {};
                });
        }
        auto pipeline_ctrl = pipe.build();
        expect(rt.register_controller(std::move(pipeline_ctrl)).has_value()).toBeTruthy();

        constexpr int NUM_TICKS = 100;
        for (int t = 0; t < NUM_TICKS; ++t) {
            auto r = rt.tick(0.016f);
            expect(r.status == ExecutionStatus::Success).toBeTruthy();
        }
        int expected_blocks = (NUM_STAGES / 5) * NUM_TICKS;
        expect(rt.block_count(1) == static_cast<size_t>(expected_blocks)).toBeTruthy();
    });

    it("rapid AoS/SoA conversion 100 times", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int) * 4; desc.alignment = alignof(int);
        desc.name = "test"; desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        std::vector<Identity> ids;
        for (int i = 0; i < 100; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            ids.push_back(id.value());
        }

        for (int i = 0; i < 100; ++i) {
            rt.convert_to_soa(1);
            expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();
            rt.convert_from_soa(1);
            expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        }

        for (auto id : ids) {
            expect(rt.has_block(id)).toBeTruthy();
        }
        expect(rt.block_count(1) == 100).toBeTruthy();
    });

    it("ephemeral storm: 100 controllers creating ephemerals over 100 ticks", {
        DefaultRuntime rt;
        BlockTypeDescriptor persistent;
        persistent.type_id = 1; persistent.size = sizeof(int); persistent.alignment = alignof(int);
        persistent.name = "persistent";
        expect(rt.register_block_type(persistent).has_value()).toBeTruthy();

        BlockTypeDescriptor ephemeral;
        ephemeral.type_id = 2; ephemeral.size = sizeof(int); ephemeral.alignment = alignof(int);
        ephemeral.name = "ephemeral"; ephemeral.ephemeral = true;
        expect(rt.register_block_type(ephemeral).has_value()).toBeTruthy();

        constexpr int NUM_CTRLS = 100;
        for (int i = 0; i < NUM_CTRLS; ++i) {
            auto ctrl = [](StateView&, CommandProducer& producer, EphemeralProducer& ephem) -> Result<void> {
                auto _e = ephem.create(2); (void)_e;
                auto r = producer.produce(Command{CommandCreateBlock{1}});
                if (!r) return r.error();
                return {};
            };
            expect(rt.register_controller(std::move(ctrl)).has_value()).toBeTruthy();
        }

        constexpr int NUM_TICKS = 100;
        for (int t = 0; t < NUM_TICKS; ++t) {
            auto r = rt.tick(0.016f);
            expect(r.status == ExecutionStatus::Success).toBeTruthy();
            expect(r.controllers_executed == NUM_CTRLS).toBeTruthy();
            expect(r.commands_executed == NUM_CTRLS).toBeTruthy();
        }
        expect(rt.block_count(1) == NUM_CTRLS * NUM_TICKS).toBeTruthy();
    });
});
}
