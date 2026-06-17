#pragma once

namespace {
namespace pipeline_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_stages_execute_in_order() {
        std::vector<int> order;
        Pipeline pipe;
        pipe.add_stage("A", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { order.push_back(1); return {}; });
        pipe.add_stage("B", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { order.push_back(2); return {}; });
        pipe.add_stage("C", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { order.push_back(3); return {}; });

        DefaultRuntime rt;
        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(order.size() == 3).toBeTruthy();
        expect(order[0] == 1).toBeTruthy();
        expect(order[1] == 2).toBeTruthy();
        expect(order[2] == 3).toBeTruthy();
    }

    void test_stage_output_feeds_next() {
        Pipeline pipe;
        pipe.add_stage("Creator", [](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
            auto r = producer.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        });
        pipe.add_stage("Checker", [](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            if (!view.has_block(Identity{1})) {
                return Error(ErrorCode::BlockNotFound, "not found");
            }
            return {};
        });

        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.commands_executed == 1).toBeTruthy();
    }

    void test_empty_pipeline() {
        Pipeline pipe;
        DefaultRuntime rt;
        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.commands_executed == 0).toBeTruthy();
    }

    void test_stage_is_controller() {
        Pipeline pipe;
        pipe.add_stage("test", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; });
        Controller c = pipe.build();
        expect(static_cast<bool>(c)).toBeTruthy();
    }

    void test_stage_diagnostics() {
        Pipeline pipe;
        pipe.add_stage("Movement", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; });
        pipe.add_stage("Collision", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; });
        pipe.add_stage("Cleanup", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; });

        DefaultRuntime rt;
        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();

        auto results = pipe.last_results();
        expect(results.size() == 3).toBeTruthy();
        expect(results[0].name == "Movement").toBeTruthy();
        expect(results[1].name == "Collision").toBeTruthy();
        expect(results[2].name == "Cleanup").toBeTruthy();
    }

    void test_pipeline_is_controller() {
        Pipeline pipe;
        Controller c = pipe.build();
        expect(static_cast<bool>(c)).toBeTruthy();

        DefaultRuntime rt;
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();
    }

    void test_stage_failure_isolation() {
        std::vector<int> order;
        Pipeline pipe;
        pipe.add_stage("Failing", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(1);
            return Error(ErrorCode::ControllerFailed, "boom");
        });
        pipe.add_stage("After", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(2);
            return {};
        });

        DefaultRuntime rt;
        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(order.size() == 2).toBeTruthy();
        expect(order[0] == 1).toBeTruthy();
        expect(order[1] == 2).toBeTruthy();
    }

    void test_dynamic_add_remove() {
        Pipeline pipe;
        auto id = pipe.add_stage("temp", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; });
        expect(pipe.stage_count() == 1).toBeTruthy();
        pipe.remove_stage(id);
        expect(pipe.stage_count() == 0).toBeTruthy();
        pipe.add_stage("permanent", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; });
        expect(pipe.stage_count() == 1).toBeTruthy();
    }
}
}

inline void run_pipeline_tests() {
describe("Pipeline", {
    it("stages execute in order",           { pipeline_helpers::test_stages_execute_in_order(); });
    it("stage output feeds next",           { pipeline_helpers::test_stage_output_feeds_next(); });
    it("empty pipeline",                    { pipeline_helpers::test_empty_pipeline(); });
    it("stage is a Controller",             { pipeline_helpers::test_stage_is_controller(); });
    it("stage diagnostics",                 { pipeline_helpers::test_stage_diagnostics(); });
    it("Pipeline is a Controller",          { pipeline_helpers::test_pipeline_is_controller(); });
    it("stage failure isolation",           { pipeline_helpers::test_stage_failure_isolation(); });
    it("dynamic add/remove",                { pipeline_helpers::test_dynamic_add_remove(); });
});
}
