#pragma once

inline void run_dsl_callback_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("DSL - Callbacks", {
    it("before_tick_callback_fires", {
        DefaultRuntime rt;
        int setup_count = 0;
        rt.before_tick([&] { setup_count++; });
        rt.tick();
        expect(setup_count == 1).toBeTruthy();
    });

    it("after_tick_callback_receives_result", {
        DefaultRuntime rt;
        TickResult captured;
        rt.after_tick([&](const TickResult& result) { captured = result; });
        rt.tick();
        expect(captured.status == ExecutionStatus::Success).toBeTruthy();
    });

    it("when_block_created_callback_fires", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        int created_count = 0;
        rt.when_block_created([&](Identity, uint32_t) { created_count++; });

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();
        rt.tick();
        expect(created_count == 1).toBeTruthy();
    });

    it("when_block_destroyed_callback_fires", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        int destroyed_count = 0;
        rt.when_block_destroyed([&](Identity) { destroyed_count++; });

        auto id = rt.submit_command(Command{CommandDestroyBlock{{block.value()}}});
        expect(id.has_value()).toBeTruthy();
        rt.tick();
        expect(destroyed_count == 1).toBeTruthy();
    });

    it("before_tick_multiple_callbacks", {
        DefaultRuntime rt;
        int count_a = 0;
        int count_b = 0;
        rt.before_tick([&] { count_a++; });
        rt.before_tick([&] { count_b++; });
        rt.tick();
        expect(count_a == 1).toBeTruthy();
        expect(count_b == 1).toBeTruthy();
    });

    it("callback_unlisten", {
        DefaultRuntime rt;
        int call_count = 0;
        auto handler = [&] { call_count++; };
        auto id = rt.before_tick(std::move(handler));
        rt.tick();
        expect(call_count == 1).toBeTruthy();
        rt.unlisten(id);
        rt.tick();
        expect(call_count == 1).toBeTruthy();
    });
});
}
