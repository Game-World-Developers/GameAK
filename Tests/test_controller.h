#pragma once

inline void run_controller_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Controllers", {
    it("creates a block via controller", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "counter";
        auto r1 = rt.register_block_type(desc);
        expect(r1.has_value()).toBeTruthy();

        auto controller = [](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
            auto result = producer.produce(Command{CommandCreateBlock{1}});
            if (!result) return result.error();
            return {};
        };

        auto reg = rt.register_controller(std::move(controller));
        expect(reg.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(result.controllers_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    });

    it("produces multiple commands in one tick", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        auto r1 = rt.register_block_type(desc);
        expect(r1.has_value()).toBeTruthy();

        auto controller = [](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
            auto r1 = producer.produce(Command{CommandCreateBlock{1}});
            if (!r1) return r1.error();
            auto r2 = producer.produce(Command{CommandCreateBlock{1}});
            if (!r2) return r2.error();
            return {};
        };

        auto reg = rt.register_controller(std::move(controller));
        expect(reg.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(result.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    });

    it("controller failure produces PartialFailure", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto controller = [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return Error(ErrorCode::ControllerFailed, "controller error");
        };

        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::PartialFailure).toBeTruthy();
        expect(result.controllers_executed == 1).toBeTruthy();
    });

    it("StateView detects block existence", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        auto controller = [id = block.value()](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            if (!view.has_block(id)) {
                return Error(ErrorCode::BlockNotFound, "block not found");
            }
            return {};
        };

        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
    });

    it("rejects registration of empty controller", {
        DefaultRuntime rt;
        Controller empty;
        auto result = rt.register_controller(std::move(empty));
        expect(result.has_value()).toBeFalsy();
        expect(result.error().code() == ErrorCode::InvalidOperation).toBeTruthy();
    });

    it("StateView provides time delta", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        float captured_delta = 0.0f;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            captured_delta = view.time_delta();
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto result = rt.tick(0.033f);
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(captured_delta == 0.033f).toBeTruthy();
    });

    it("StateView time delta defaults to 16ms", {
        DefaultRuntime rt;
        float captured_delta = 0.0f;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            captured_delta = view.time_delta();
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(captured_delta == 0.016f).toBeTruthy();
    });

    it("controllers execute in priority order", {
        DefaultRuntime rt;
        std::vector<int> order;

        auto low = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(1);
            return {};
        };
        auto mid = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(2);
            return {};
        };
        auto high = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(3);
            return {};
        };

        expect(rt.register_controller(std::move(low),  -10).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(mid),    0).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(high),  10).has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(result.controllers_executed == 3).toBeTruthy();
        expect(order.size() == 3).toBeTruthy();
        // High (10) should execute first, then mid (0), then low (-10)
        expect(order[0] == 3).toBeTruthy();
        expect(order[1] == 2).toBeTruthy();
        expect(order[2] == 1).toBeTruthy();
    });

    it("controllers with equal priority preserve registration order", {
        DefaultRuntime rt;
        std::vector<int> order;

        auto first = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(1);
            return {};
        };
        auto second = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(2);
            return {};
        };
        auto third = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(3);
            return {};
        };

        expect(rt.register_controller(std::move(first),  0).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(second), 0).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(third),  0).has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(result.controllers_executed == 3).toBeTruthy();
        // Same priority → registration order preserved (stable sort)
        expect(order[0] == 1).toBeTruthy();
        expect(order[1] == 2).toBeTruthy();
        expect(order[2] == 3).toBeTruthy();
    });

    it("single-arg register_controller uses default priority 0", {
        DefaultRuntime rt;
        int exec_count = 0;
        auto controller = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            exec_count++;
            return {};
        };

        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(exec_count == 1).toBeTruthy();
        expect(result.controllers_executed == 1).toBeTruthy();
    });

    it("conversation_command_producer_create", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto controller = [&](StateView&, CommandProducer& prod, EphemeralProducer&) -> Result<void> {
            return prod.create(1);
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(result.commands_executed == 1).toBeTruthy();
    });

    it("conversation_command_producer_destroy", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();
        auto block_id = block.value();

        auto controller = [&](StateView&, CommandProducer& prod, EphemeralProducer&) -> Result<void> {
            auto _ = prod.destroy(block_id);
            (void)_;
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(rt.has_block(block_id)).toBeFalsy();
    });
});
}
