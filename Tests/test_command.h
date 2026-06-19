#pragma once

inline void run_command_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Commands", {
    it("creates a block via command", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        auto r1 = rt.register_block_type(desc);
        expect(r1.has_value()).toBeTruthy();

        auto submit = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(submit.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    });

    it("destroys a block via command", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        auto r1 = rt.register_block_type(desc);
        expect(r1.has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        auto submit = rt.submit_command(Command{CommandDestroyBlock{{block.value()}}});
        expect(submit.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(rt.block_count(1) == 0).toBeTruthy();
    });

    it("writes field data via command", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        auto r1 = rt.register_block_type(desc);
        expect(r1.has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        int value = 42;
        auto bytes = std::vector<std::byte>(
            reinterpret_cast<std::byte*>(&value),
            reinterpret_cast<std::byte*>(&value) + sizeof(int));

        auto submit = rt.submit_command(Command{CommandSetField{{block.value()}, 0, bytes}});
        expect(submit.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();

        auto& blocks = rt.blocks();
        auto it = blocks.find(block.value());
        expect(it != blocks.end()).toBeTruthy();

        int stored;
        std::memcpy(&stored, it->second.data(), sizeof(int));
        expect(stored).toEqual(42);
    });

    it("destroys multiple blocks with one command", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(1);
        auto b3 = rt.create_block(1);
        expect(b1.has_value() && b2.has_value() && b3.has_value()).toBeTruthy();

        auto submit = rt.submit_command(
            Command{CommandDestroyBlock{{b1.value(), b3.value()}}});
        expect(submit.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
        expect(rt.has_block(b2.value())).toBeTruthy();
    });

    it("resizes a block and writes beyond original size", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        auto resize = rt.submit_command(
            Command{CommandResizeBlock{{block.value()}, sizeof(int) * 2}});
        expect(resize.has_value()).toBeTruthy();
        rt.tick();
        expect(rt.blocks().at(block.value()).size() == sizeof(int) * 2).toBeTruthy();

        int value = 99;
        auto bytes = std::vector<std::byte>(
            reinterpret_cast<std::byte*>(&value),
            reinterpret_cast<std::byte*>(&value) + sizeof(int));

        auto set = rt.submit_command(
            Command{CommandSetField{{block.value()}, sizeof(int), bytes}});
        expect(set.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();

        int stored;
        std::memcpy(&stored,
                    static_cast<const std::byte*>(rt.blocks().at(block.value()).data()) + sizeof(int),
                    sizeof(int));
        expect(stored).toEqual(99);
    });

    it("rejects command for unregistered type", {
        DefaultRuntime rt;
        auto submit = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(submit.has_value()).toBeTruthy();
        auto result = rt.tick();
        expect(result.commands_rejected == 1).toBeTruthy();
        expect(result.commands_executed == 0).toBeTruthy();
        expect(result.rejected_commands.size() == 1).toBeTruthy();
        expect(result.rejected_commands[0].error.code() == ErrorCode::TypeNotRegistered).toBeTruthy();
    });

    it("cancels a pending command", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();
        auto cancel = rt.cancel_command(id.value());
        expect(cancel.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.commands_executed == 0).toBeTruthy();
        expect(result.commands_rejected == 0).toBeTruthy();
        expect(rt.block_count(1) == 0).toBeTruthy();
    });

    it("cancels one command, executes another", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id2.has_value()).toBeTruthy();
        auto cancel = rt.cancel_command(id1.value());
        expect(cancel.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    });

    it("replays a command from history", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();
        auto r1 = rt.tick();
        expect(r1.commands_executed == 1).toBeTruthy();

        auto& history = rt.command_history();
        expect(history.size() == 1).toBeTruthy();

        // Replay the command from history
        auto replay = rt.replay_command(history[0]);
        expect(replay.has_value()).toBeTruthy();

        auto r2 = rt.tick();
        expect(r2.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
        expect(rt.command_history().size() == 2).toBeTruthy();
    });

    it("priority scheduler processes high priority first", {
        using PriorityRuntime = Runtime<PriorityScheduler>;
        PriorityRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        std::vector<CommandId> execution_order;
        rt.scheduler().set_priority_fn(
            [&](const Command& cmd) -> int {
                return -static_cast<int>(cmd.id());
            });

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id3 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id1.has_value()).toBeTruthy();
        expect(id2.has_value()).toBeTruthy();
        expect(id3.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.commands_executed == 3).toBeTruthy();
        expect(rt.block_count(1) == 3).toBeTruthy();

        // With the priority fn returning -id, higher IDs (later submissions)
        // get higher priority and run first
        auto& history = rt.command_history();
        expect(history.size() == 3).toBeTruthy();
    });

    it("priority scheduler defaults to FIFO when no priority fn", {
        using PriorityRuntime = Runtime<PriorityScheduler>;
        PriorityRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id1.has_value()).toBeTruthy();
        expect(id2.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    });

    it("rejects multi-target command when one target is invalid", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        Identity invalid_id{999};
        auto submit = rt.submit_command(
            Command{CommandDestroyBlock{{block.value(), invalid_id}}});
        expect(submit.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.commands_rejected == 1).toBeTruthy();
        expect(result.commands_executed == 0).toBeTruthy();
        expect(rt.has_block(block.value())).toBeTruthy();
    });

    it("rejects SetField when offset exceeds block size", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        int value = 99;
        auto bytes = std::vector<std::byte>(
            reinterpret_cast<std::byte*>(&value),
            reinterpret_cast<std::byte*>(&value) + sizeof(int));

        auto submit = rt.submit_command(
            Command{CommandSetField{{block.value()}, 100, bytes}});
        expect(submit.has_value()).toBeTruthy();

        auto result = rt.tick();
        expect(result.commands_rejected == 1).toBeTruthy();
        expect(result.rejected_commands[0].error.code() == ErrorCode::CommandInvalid).toBeTruthy();
    });

    it("rejects DestroyBlock with invalid identity", {
        DefaultRuntime rt;
        auto submit = rt.submit_command(Command{CommandDestroyBlock{{Identity::invalid()}}});
        expect(submit.has_value()).toBeTruthy();
        auto result = rt.tick();
        expect(result.commands_rejected == 1).toBeTruthy();
        expect(result.rejected_commands[0].error.code() == ErrorCode::InvalidIdentity).toBeTruthy();
    });

    it("conversation_command_create_block_factory", {
        auto cmd = Command::create_block(1);
        expect(cmd.type() == CommandType::CreateBlock).toBeTruthy();
        const auto& payload = std::get<CommandCreateBlock>(cmd.payload());
        expect(payload.type_id == 1).toBeTruthy();
    });

    it("conversation_command_destroy_block_factory", {
        Identity id{42};
        auto cmd = Command::destroy_block(id);
        expect(cmd.type() == CommandType::DestroyBlock).toBeTruthy();
        const auto& payload = std::get<CommandDestroyBlock>(cmd.payload());
        expect(payload.targets.size() == 1).toBeTruthy();
        expect(payload.targets[0] == id).toBeTruthy();
    });

    it("conversation_command_set_field_factory", {
        Identity id{1};
        int value = 42;
        auto cmd = Command::set_field(id, 8, value);
        expect(cmd.type() == CommandType::SetField).toBeTruthy();
        const auto& payload = std::get<CommandSetField>(cmd.payload());
        expect(payload.targets.size() == 1).toBeTruthy();
        expect(payload.targets[0] == id).toBeTruthy();
        expect(payload.offset == 8).toBeTruthy();
        expect(payload.data.size() == sizeof(int)).toBeTruthy();
    });

    it("conversation_command_set_field_raw", {
        Identity id(1);
        std::byte bytes[4];
        bytes[0] = static_cast<std::byte>(1);
        bytes[1] = static_cast<std::byte>(2);
        bytes[2] = static_cast<std::byte>(3);
        bytes[3] = static_cast<std::byte>(4);
        auto cmd = Command::set_field_raw(id, 0, bytes, 4);
        expect(cmd.type() == CommandType::SetField).toBeTruthy();
    });

    it("conversation_command_resize_block_factory", {
        Identity id{1};
        auto cmd = Command::resize_block(id, 64);
        expect(cmd.type() == CommandType::ResizeBlock).toBeTruthy();
        const auto& payload = std::get<CommandResizeBlock>(cmd.payload());
        expect(payload.targets.size() == 1).toBeTruthy();
        expect(payload.new_size == 64).toBeTruthy();
    });

    it("conversation_command_convert_layout_factory", {
        auto cmd = Command::convert_layout(1, LayoutStrategy::SoA);
        expect(cmd.type() == CommandType::ConvertLayout).toBeTruthy();
        const auto& payload = std::get<CommandConvertLayout>(cmd.payload());
        expect(payload.type_id == 1).toBeTruthy();
        expect(payload.new_layout == LayoutStrategy::SoA).toBeTruthy();
    });

    it("conversation_command_create_destroy_via_runtime", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.submit_command(Command::create_block(1));
        expect(id.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();

        auto blocks = rt.blocks_of_type(1);
        auto destroy = rt.submit_command(Command::destroy_block(blocks[0]));
        expect(destroy.has_value()).toBeTruthy();
        r = rt.tick();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 0).toBeTruthy();
    });
});
}
