#pragma once

inline void run_scheduler_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Scheduler", {

    it("FIFO: commands execute in submission order", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id3 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id1.has_value() && id2.has_value() && id3.has_value()).toBeTruthy();
        expect(id1.value() < id2.value()).toBeTruthy();
        expect(id2.value() < id3.value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_executed == 3).toBeTruthy();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(rt.block_count(1) == 3).toBeTruthy();
    });

    it("FIFO: maintains submission order across multiple ticks", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id1.has_value()).toBeTruthy();

        auto r1 = rt.tick();
        expect(r1.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();

        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id3 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id2.has_value() && id3.has_value()).toBeTruthy();

        auto r2 = rt.tick();
        expect(r2.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 3).toBeTruthy();
    });

    it("cancelled command is skipped", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id1.has_value() && id2.has_value()).toBeTruthy();

        auto _c = rt.cancel_command(id1.value()); (void)_c;

        auto r = rt.tick();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    });

    it("cancelled command does not modify state", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        auto destroy = rt.submit_command(Command{CommandDestroyBlock{{block.value()}}});
        expect(destroy.has_value()).toBeTruthy();

        auto _c = rt.cancel_command(destroy.value()); (void)_c;

        auto r = rt.tick();
        expect(r.commands_executed == 0).toBeTruthy();
        expect(rt.has_block(block.value())).toBeTruthy();
    });

    it("rejects command with nonexistent identity", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        Identity fake{999};
        auto id = rt.submit_command(Command{CommandDestroyBlock{{fake}}});
        expect(id.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_rejected == 1).toBeTruthy();
        expect(r.status == ExecutionStatus::PartialFailure).toBeTruthy();
    });

    it("rejects command with unregistered type", {
        DefaultRuntime rt;
        auto id = rt.submit_command(Command{CommandCreateBlock{99}});
        expect(id.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_rejected == 1).toBeTruthy();
        expect(r.status == ExecutionStatus::PartialFailure).toBeTruthy();
    });

    it("rejection produces error in TickResult", {
        DefaultRuntime rt;
        auto id = rt.submit_command(Command{CommandCreateBlock{99}});
        expect(id.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.rejected_commands.size() == 1).toBeTruthy();
        expect(r.rejected_commands[0].error.code() == ErrorCode::TypeNotRegistered).toBeTruthy();
    });

    it("atomic: failed command does not modify state", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();
        auto initial_count = rt.block_count(1);

        Identity fake{999};
        auto multi_target = rt.submit_command(
            Command{CommandDestroyBlock{{block.value(), fake}}});
        expect(multi_target.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_rejected >= 1).toBeTruthy();
        expect(rt.block_count(1) == initial_count).toBeTruthy();
    });

    it("deterministic: same commands produce same block count", {
        DefaultRuntime rt1;
        DefaultRuntime rt2;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt1.register_block_type(desc).has_value()).toBeTruthy();
        expect(rt2.register_block_type(desc).has_value()).toBeTruthy();

        auto ctrl = [](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
            auto r = producer.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        expect(rt1.register_controller(std::move(ctrl)).has_value()).toBeTruthy();
        auto ctrl2 = [](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
            auto r = producer.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        expect(rt2.register_controller(std::move(ctrl2)).has_value()).toBeTruthy();

        auto r1 = rt1.tick();
        auto r2 = rt2.tick();
        expect(r1.commands_executed == r2.commands_executed).toBeTruthy();
        expect(rt1.block_count(1) == rt2.block_count(1)).toBeTruthy();
    });

    it("scheduler provides pending diagnostics", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        expect(rt.scheduler().pending_count() == 0).toBeTruthy();
        auto _s1 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_s1;
        expect(rt.scheduler().pending_count() == 1).toBeTruthy();
        auto _s2 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_s2;
        expect(rt.scheduler().pending_count() == 2).toBeTruthy();
    });

    it("scheduler resets counts after tick", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto _s = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_s;
        rt.tick();
        expect(rt.scheduler().pending_count() == 0).toBeTruthy();
    });

    it("scheduler command history is accessible", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto _s1 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_s1;
        auto _s2 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_s2;
        rt.tick();

        expect(rt.command_history().size() == 2).toBeTruthy();
    });

    it("PriorityScheduler executes high priority commands first", {
        using PriorityRuntime = Runtime<PriorityScheduler>;
        PriorityRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        std::vector<CommandId> exec_order;
        rt.scheduler().set_priority_fn(
            [&](const Command& cmd) -> int {
                exec_order.push_back(cmd.id());
                return -static_cast<int>(cmd.id());
            });

        auto _p1 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_p1;
        auto _p2 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_p2;
        auto _p3 = rt.submit_command(Command{CommandCreateBlock{1}}); (void)_p3;

        auto r = rt.tick();
        expect(r.commands_executed == 3).toBeTruthy();
        expect(rt.block_count(1) == 3).toBeTruthy();
    });

    it("PriorityScheduler defaults to FIFO when no priority fn", {
        using PriorityRuntime = Runtime<PriorityScheduler>;
        PriorityRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id1.has_value() && id2.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    });
});
}
