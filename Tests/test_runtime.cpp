// GameAK headers first (pull in standard library headers)
#include "GameAk/Core/identity.h"
#include "GameAk/Core/platform.h"
#include "GameAk/Core/simd.h"
#include "GameAk/Core/result.h"
#include "GameAk/Core/error.h"
#include "GameAk/Core/flat_vector.h"
#include "GameAk/Core/intrusive_list.h"
#include "GameAk/Core/avl_tree.h"
#include "GameAk/Core/rb_tree.h"
#include "GameAk/Core/bitset.h"
#include "GameAk/Core/bitvector.h"
#include "GameAk/Core/bitflags.h"
#include "GameAk/Core/bit_packing.h"
#include "GameAk/Runtime/fsm.h"
#include "GameAk/Runtime/rule_system.h"
#include "GameAk/Runtime/pipeline.h"
#include "GameAk/Runtime/event_loop.h"
#include "GameAk/Runtime/runtime.h"
#include "GameAk/Runtime/controller.h"
#include "GameAk/Runtime/command.h"
#include "GameAk/Runtime/priority_scheduler.h"

// Standard library headers
#include <cstring>
#include <memory>
#include <string>
#include <vector>

// Cest must be included AFTER all headers that use std::atomic
#include "cest.h"

// Test header files (use cest macros)
#include "test_identity.h"
#include "test_flat_vector.h"
#include "test_intrusive_list.h"
#include "test_error.h"
#include "test_command.h"
#include "test_controller.h"
#include "test_avl_tree.h"
#include "test_rb_tree.h"
#include "test_bit_representation.h"
#include "test_data_layout.h"
#include "test_fsm.h"
#include "test_rule_system.h"
#include "test_pipeline.h"
#include "test_event_loop.h"
#include "test_ephemeral.h"
#include "test_scheduler.h"
#include "test_integration.h"
#include "test_stress.h"
#include "test_fuzz.h"
#include "test_e2e.h"
#include "test_semantic.h"
#include "test_edge_cases.h"

using namespace gameak::core;
using namespace gameak::runtime;

using DefaultRuntime = Runtime<FifoScheduler>;

inline void run_runtime_tests() {
describe("Runtime", {
    it("creates and destroys a block", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        auto reg = rt.register_block_type(desc);
        expect(reg.has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();
        expect(rt.has_block(block.value())).toBeTruthy();

        auto destroy = rt.destroy_block(block.value());
        expect(destroy.has_value()).toBeTruthy();
        expect(rt.has_block(block.value())).toBeFalsy();
    });

    it("fails to create block with unregistered type", {
        DefaultRuntime rt;
        auto block = rt.create_block(99);
        expect(block.has_value()).toBeFalsy();
        expect(block.error().code() == ErrorCode::TypeNotRegistered).toBeTruthy();
    });

    it("rejects duplicate type registration", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        auto r1 = rt.register_block_type(desc);
        expect(r1.has_value()).toBeTruthy();
        auto r2 = rt.register_block_type(desc);
        expect(r2.has_value()).toBeFalsy();
        expect(r2.error().code() == ErrorCode::DuplicateRegistration).toBeTruthy();
    });

    it("creates with custom config", {
        RuntimeConfig cfg;
        cfg.log_level = LogLevel::Error;
        Runtime<FifoScheduler> rt{cfg};
        expect(rt.config().log_level == LogLevel::Error).toBeTruthy();
    });

    it("CriticalFailure enum value exists", {
        TickResult tr;
        tr.status = ExecutionStatus::CriticalFailure;
        expect(tr.status == ExecutionStatus::CriticalFailure).toBeTruthy();
    });

    it("identity destroyed and not reused on new blocks", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Create blocks, note the IDs
        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(1);
        auto b3 = rt.create_block(1);
        expect(b1.has_value()).toBeTruthy();
        expect(b2.has_value()).toBeTruthy();
        expect(b3.has_value()).toBeTruthy();
        auto id1 = b1.value();
        auto id2 = b2.value();

        // Destroy them
        expect(rt.destroy_block(id1).has_value()).toBeTruthy();
        expect(rt.destroy_block(id2).has_value()).toBeTruthy();

        // Create new blocks - IDs must be fresh, not reused
        auto b4 = rt.create_block(1);
        auto b5 = rt.create_block(1);
        expect(b4.has_value()).toBeTruthy();
        expect(b5.has_value()).toBeTruthy();

        // New IDs must be greater than the destroyed ones
        expect(b4.value().value() > id2.value()).toBeTruthy();
        expect(b5.value().value() > b4.value().value()).toBeTruthy();

        // Destroyed IDs must not appear
        expect(b4.value() != id1).toBeTruthy();
        expect(b4.value() != id2).toBeTruthy();
        expect(b5.value() != id1).toBeTruthy();
        expect(b5.value() != id2).toBeTruthy();
    });

    it("identities are monotonically increasing", {
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
        expect(b1.has_value()).toBeTruthy();
        expect(b2.has_value()).toBeTruthy();
        expect(b3.has_value()).toBeTruthy();

        // IDs start at 1 and increase monotonically
        expect(b1.value().value() == 1).toBeTruthy();
        expect(b2.value().value() == 2).toBeTruthy();
        expect(b3.value().value() == 3).toBeTruthy();
    });

    it("commands submitted between ticks are queued for next tick", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Submit command without ticking
        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();

        // Block should not exist yet (command hasn't been processed)
        // We can't easily check this directly since blocks aren't created yet,
        // but after first tick, it should be processed
        auto r1 = rt.tick();
        expect(r1.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    });

    it("diagnostics reports internal state", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto diag = rt.collect_diagnostics();
        expect(diag.pending_commands == 0).toBeTruthy();
        expect(diag.blocks_count == 0).toBeTruthy();
        expect(diag.controllers_count == 0).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        auto controller = [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();

        diag = rt.collect_diagnostics();
        expect(diag.blocks_count == 2).toBeTruthy();
        expect(diag.controllers_count == 1).toBeTruthy();
        expect(diag.pending_commands == 1).toBeTruthy();

        auto result = rt.tick();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();

        diag = rt.collect_diagnostics();
        expect(diag.commands_executed == 1).toBeTruthy();
        expect(diag.pending_commands == 0).toBeTruthy();
        expect(diag.blocks_count == 3).toBeTruthy();
    });

    it("deterministic execution produces identical state", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Create a block, run tick
        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        auto r1 = rt.tick();
        expect(r1.status == ExecutionStatus::Success).toBeTruthy();
        auto count1 = rt.block_count(1);
        auto has1 = rt.has_block(block.value());

        // Second tick with same state
        auto r2 = rt.tick();
        expect(r2.status == ExecutionStatus::Success).toBeTruthy();
        auto count2 = rt.block_count(1);
        auto has2 = rt.has_block(block.value());

        // State should be identical
        expect(count1 == count2).toBeTruthy();
        expect(has1 == has2).toBeTruthy();
    });

    it("pause skips tick execution", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        rt.pause();
        expect(rt.is_paused()).toBeTruthy();

        auto r1 = rt.tick(0.016f);
        expect(r1.commands_executed == 0).toBeTruthy();
        expect(r1.controllers_executed == 0).toBeTruthy();
        expect(r1.status == ExecutionStatus::Success).toBeTruthy();
    });

    it("resume allows tick execution after pause", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        rt.pause();
        auto r1 = rt.tick();
        expect(r1.controllers_executed == 0).toBeTruthy();

        rt.resume();
        expect(rt.is_paused()).toBeFalsy();

        // Add a controller to verify execution
        auto controller = [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        auto r2 = rt.tick();
        expect(r2.controllers_executed == 1).toBeTruthy();
    });

    it("fixed timestep accumulates and runs multiple sub-ticks", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Set fixed timestep to 10ms
        rt.set_fixed_timestep(0.01f);
        expect(rt.fixed_timestep() == 0.01f).toBeTruthy();

        // Submit one command per tick via a controller
        int controller_invocations = 0;
        auto controller = [&](StateView&, CommandProducer& producer, EphemeralProducer&) -> Result<void> {
            controller_invocations++;
            auto r = producer.produce(Command{CommandCreateBlock{1}});
            if (!r) return r.error();
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        // Call tick with 25ms → should produce 2 sub-ticks (10ms each)
        auto result = rt.tick(0.025f);
        expect(result.controllers_executed == 2).toBeTruthy();
        expect(result.commands_executed == 2).toBeTruthy();
        expect(result.status == ExecutionStatus::Success).toBeTruthy();
        expect(controller_invocations == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    });

    it("fixed timestep sub-ticks see the fixed delta, not accumulated delta", {
        DefaultRuntime rt;

        float captured_delta = 0.0f;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            captured_delta = view.time_delta();
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();

        const float fixed_dt = 0.005f;
        rt.set_fixed_timestep(fixed_dt);
        rt.tick(0.012f); // 2 sub-ticks

        // The time delta seen by controllers should be the fixed dt, not the accumulated amount
        expect(captured_delta == fixed_dt).toBeTruthy();
    });

    it("clear_fixed_timestep reverts to variable timestep", {
        DefaultRuntime rt;

        float captured = 0.0f;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            captured = view.time_delta();
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.set_fixed_timestep(0.01f);
        rt.clear_fixed_timestep();
        expect(rt.fixed_timestep() == 0.0f).toBeTruthy();

        rt.tick(0.033f);
        expect(captured == 0.033f).toBeTruthy();
    });

    it("paused runtime still accepts commands but does not process them", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        rt.pause();
        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();

        auto r1 = rt.tick();
        expect(r1.commands_executed == 0).toBeTruthy();
        expect(rt.block_count(1) == 0).toBeTruthy();

        // Commands queued while paused should process after resume
        rt.resume();
        auto r2 = rt.tick();
        expect(r2.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    });

    it("find_blocks_by_type returns blocks of matching type", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc1;
        desc1.type_id = 1; desc1.size = sizeof(int); desc1.alignment = alignof(int); desc1.name = "type1";
        BlockTypeDescriptor desc2;
        desc2.type_id = 2; desc2.size = sizeof(double); desc2.alignment = alignof(double); desc2.name = "type2";
        expect(rt.register_block_type(desc1).has_value()).toBeTruthy();
        expect(rt.register_block_type(desc2).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(2);
        auto b3 = rt.create_block(1);
        auto b4 = rt.create_block(2);
        expect(b1.has_value() && b2.has_value() && b3.has_value() && b4.has_value()).toBeTruthy();

        auto type1_blocks = rt.find_blocks_by_type(1);
        auto type2_blocks = rt.find_blocks_by_type(2);
        auto type3_blocks = rt.find_blocks_by_type(99);

        expect(type1_blocks.size() == 2).toBeTruthy();
        expect(type2_blocks.size() == 2).toBeTruthy();
        expect(type3_blocks.size() == 0).toBeTruthy();

        // Verify identities are correct
        expect(type1_blocks[0] == b1.value() || type1_blocks[0] == b3.value()).toBeTruthy();
        expect(type2_blocks[0] == b2.value() || type2_blocks[0] == b4.value()).toBeTruthy();
    });

    it("find_blocks with predicate filters correctly", {
        DefaultRuntime rt;
        // Register two types with different sizes
        BlockTypeDescriptor desc1;
        desc1.type_id = 1; desc1.size = 16; desc1.alignment = alignof(int); desc1.name = "small";
        BlockTypeDescriptor desc2;
        desc2.type_id = 2; desc2.size = 64; desc2.alignment = alignof(double); desc2.name = "large";
        expect(rt.register_block_type(desc1).has_value()).toBeTruthy();
        expect(rt.register_block_type(desc2).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(2);
        auto b3 = rt.create_block(1);
        expect(b1.has_value() && b2.has_value() && b3.has_value()).toBeTruthy();

        // Find blocks with size > 32 bytes (should only return type 2 blocks)
        auto large_blocks = rt.find_blocks([](const DataBlock& block) {
            return block.size() > 32;
        });

        expect(large_blocks.size() == 1).toBeTruthy();
        expect(large_blocks[0] == b2.value()).toBeTruthy();
    });

    it("find_blocks returns empty when nothing matches", {
        DefaultRuntime rt;
        auto result = rt.find_blocks([](const DataBlock&) { return false; });
        expect(result.size() == 0).toBeTruthy();
    });

    it("find_blocks returns all blocks when predicate always true", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        auto all = rt.find_blocks([](const DataBlock&) { return true; });
        expect(all.size() == 3).toBeTruthy();
    });

    it("conversation_blocks_of_type", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc1;
        desc1.type_id = 1; desc1.size = sizeof(int); desc1.alignment = alignof(int); desc1.name = "type1";
        BlockTypeDescriptor desc2;
        desc2.type_id = 2; desc2.size = sizeof(double); desc2.alignment = alignof(double); desc2.name = "type2";
        expect(rt.register_block_type(desc1).has_value()).toBeTruthy();
        expect(rt.register_block_type(desc2).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(2);
        auto b3 = rt.create_block(1);
        expect(b1.has_value() && b2.has_value() && b3.has_value()).toBeTruthy();

        auto type1 = rt.blocks_of_type(1);
        auto type2 = rt.blocks_of_type(2);
        expect(type1.size() == 2).toBeTruthy();
        expect(type2.size() == 1).toBeTruthy();
    });

    it("conversation_blocks_where", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = 16; desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        auto all = rt.blocks_where([](const DataBlock&) { return true; });
        expect(all.size() == 2).toBeTruthy();

        auto none = rt.blocks_where([](const DataBlock&) { return false; });
        expect(none.size() == 0).toBeTruthy();
    });
});
}

inline void run_event_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Events", {
    it("TickBegin and TickEnd fire during tick", {
        DefaultRuntime rt;
        int tick_begin_count = 0;
        int tick_end_count = 0;

        rt.listen(Runtime<FifoScheduler>::EventType::TickBegin,
            [&](const Runtime<FifoScheduler>::Event&) { tick_begin_count++; });
        rt.listen(Runtime<FifoScheduler>::EventType::TickEnd,
            [&](const Runtime<FifoScheduler>::Event&) { tick_end_count++; });

        rt.tick(0.016f);
        expect(tick_begin_count == 1).toBeTruthy();
        expect(tick_end_count == 1).toBeTruthy();
    });

    it("BlockCreated fires when a block is created via command", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        int created_count = 0;
        Identity created_id;
        uint32_t created_type = 0;

        rt.listen(Runtime<FifoScheduler>::EventType::BlockCreated,
            [&](const Runtime<FifoScheduler>::Event& ev) {
                created_count++;
                created_id = ev.identity;
                created_type = ev.block_type_id;
            });

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();
        rt.tick();

        expect(created_count == 1).toBeTruthy();
        expect(created_id.is_valid()).toBeTruthy();
        expect(created_type == 1).toBeTruthy();
    });

    it("BlockDestroyed fires when a block is destroyed via command", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        int destroyed_count = 0;
        Identity destroyed_id;

        rt.listen(Runtime<FifoScheduler>::EventType::BlockDestroyed,
            [&](const Runtime<FifoScheduler>::Event& ev) {
                destroyed_count++;
                destroyed_id = ev.identity;
            });

        auto id = rt.submit_command(Command{CommandDestroyBlock{{block.value()}}});
        expect(id.has_value()).toBeTruthy();
        rt.tick();

        expect(destroyed_count == 1).toBeTruthy();
        expect(destroyed_id == block.value()).toBeTruthy();
    });

    it("unlisten removes event handler", {
        DefaultRuntime rt;
        int call_count = 0;

        auto eid = rt.listen(Runtime<FifoScheduler>::EventType::TickBegin,
            [&](const Runtime<FifoScheduler>::Event&) { call_count++; });

        rt.tick();
        expect(call_count == 1).toBeTruthy();

        rt.unlisten(eid);
        rt.tick();
        // Should still be 1 since handler was removed
        expect(call_count == 1).toBeTruthy();
    });

    it("fixed timestep fires events for each sub-tick", {
        DefaultRuntime rt;
        int tick_begin_count = 0;

        rt.listen(Runtime<FifoScheduler>::EventType::TickBegin,
            [&](const Runtime<FifoScheduler>::Event&) { tick_begin_count++; });

        rt.set_fixed_timestep(0.005f);
        rt.tick(0.012f); // 2 sub-ticks
        expect(tick_begin_count == 2).toBeTruthy();
    });

    it("multiple handlers on same event type", {
        DefaultRuntime rt;
        int count_a = 0;
        int count_b = 0;

        rt.listen(Runtime<FifoScheduler>::EventType::TickBegin,
            [&](const Runtime<FifoScheduler>::Event&) { count_a++; });
        rt.listen(Runtime<FifoScheduler>::EventType::TickBegin,
            [&](const Runtime<FifoScheduler>::Event&) { count_b++; });

        rt.tick();
        expect(count_a == 1).toBeTruthy();
        expect(count_b == 1).toBeTruthy();
    });

    it("conversation_on_tick_begin_end", {
        DefaultRuntime rt;
        int begin_count = 0;
        int end_count = 0;

        rt.on_tick_begin([&](const Runtime<FifoScheduler>::Event&) { begin_count++; });
        rt.on_tick_end([&](const Runtime<FifoScheduler>::Event&) { end_count++; });

        rt.tick();
        expect(begin_count == 1).toBeTruthy();
        expect(end_count == 1).toBeTruthy();
    });

    it("conversation_on_block_created_destroyed", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        int created = 0;
        int destroyed = 0;

        rt.on_block_created([&](const Runtime<FifoScheduler>::Event&) { created++; });
        rt.on_block_destroyed([&](const Runtime<FifoScheduler>::Event&) { destroyed++; });

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        auto _ = rt.submit_command(Command{CommandDestroyBlock{{block.value()}}});
        (void)_;
        rt.tick();

        expect(created == 0).toBeTruthy();  // no command created it
        expect(destroyed == 1).toBeTruthy();
    });
});
}

inline void run_serialization_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Serialization", {
    it("save captures current state", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        expect(b1.has_value()).toBeTruthy();
        auto b2 = rt.create_block(1);
        expect(b2.has_value()).toBeTruthy();

        auto snap = rt.save();
        expect(snap.blocks.size() == 2).toBeTruthy();
        expect(snap.types.size() == 1).toBeTruthy();
        expect(snap.next_identity >= 2).toBeTruthy();
    });

    it("load restores previously saved state", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(1);
        auto id1 = b1.value();
        expect(rt.has_block(id1)).toBeTruthy();

        auto snap = rt.save();

        // Destroy both blocks
        expect(rt.destroy_block(id1).has_value()).toBeTruthy();
        expect(rt.destroy_block(b2.value()).has_value()).toBeTruthy();
        expect(rt.has_block(id1)).toBeFalsy();

        // Restore
        rt.load(snap);
        expect(rt.has_block(id1)).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    });

    it("load rebuilds type_counts", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc1;
        desc1.type_id = 1; desc1.size = sizeof(int); desc1.alignment = alignof(int); desc1.name = "t1";
        BlockTypeDescriptor desc2;
        desc2.type_id = 2; desc2.size = sizeof(double); desc2.alignment = alignof(double); desc2.name = "t2";
        expect(rt.register_block_type(desc1).has_value()).toBeTruthy();
        expect(rt.register_block_type(desc2).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(2).has_value()).toBeTruthy();

        auto snap = rt.save();
        expect(rt.block_count(1) == 2).toBeTruthy();
        expect(rt.block_count(2) == 1).toBeTruthy();

        // Mess up type_counts_ by poking at internal block_count
        // (just recreate snap and verify load works)
        DefaultRuntime rt2;
        rt2.load(snap);
        expect(rt2.block_count(1) == 2).toBeTruthy();
        expect(rt2.block_count(2) == 1).toBeTruthy();
    });
});
}

inline void run_relationship_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

describe("Relationships", {
    it("relate creates parent-child edge", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto parent = rt.create_block(1);
        auto child  = rt.create_block(1);
        expect(parent.has_value() && child.has_value()).toBeTruthy();

        auto rel = rt.relate(parent.value(), child.value());
        expect(rel.has_value()).toBeTruthy();

        auto children = rt.children_of(parent.value());
        expect(children.size() == 1).toBeTruthy();
        expect(children[0] == child.value()).toBeTruthy();

        auto parents = rt.parents_of(child.value());
        expect(parents.size() == 1).toBeTruthy();
        expect(parents[0] == parent.value()).toBeTruthy();
    });

    it("unrelate removes parent-child edge", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto p = rt.create_block(1);
        auto c = rt.create_block(1);
        expect(p.has_value() && c.has_value()).toBeTruthy();

        expect(rt.relate(p.value(), c.value()).has_value()).toBeTruthy();
        expect(rt.unrelate(p.value(), c.value()).has_value()).toBeTruthy();

        expect(rt.children_of(p.value()).size() == 0).toBeTruthy();
        expect(rt.parents_of(c.value()).size() == 0).toBeTruthy();
    });

    it("multiple children per parent", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto p = rt.create_block(1);
        auto c1 = rt.create_block(1);
        auto c2 = rt.create_block(1);
        auto c3 = rt.create_block(1);
        expect(p.has_value() && c1.has_value() && c2.has_value() && c3.has_value()).toBeTruthy();

        expect(rt.relate(p.value(), c1.value()).has_value()).toBeTruthy();
        expect(rt.relate(p.value(), c2.value()).has_value()).toBeTruthy();
        expect(rt.relate(p.value(), c3.value()).has_value()).toBeTruthy();

        auto children = rt.children_of(p.value());
        expect(children.size() == 3).toBeTruthy();
    });

    it("multiple parents per child", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto p1 = rt.create_block(1);
        auto p2 = rt.create_block(1);
        auto c  = rt.create_block(1);

        expect(rt.relate(p1.value(), c.value()).has_value()).toBeTruthy();
        expect(rt.relate(p2.value(), c.value()).has_value()).toBeTruthy();

        auto parents = rt.parents_of(c.value());
        expect(parents.size() == 2).toBeTruthy();
    });

    it("relating self fails", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b = rt.create_block(1);
        expect(b.has_value()).toBeTruthy();

        auto rel = rt.relate(b.value(), b.value());
        expect(rel.has_value()).toBeFalsy();
        expect(rel.error().code() == ErrorCode::InvalidOperation).toBeTruthy();
    });

    it("relating nonexistent block fails", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b = rt.create_block(1);
        expect(b.has_value()).toBeTruthy();

        Identity fake{999};
        auto rel = rt.relate(b.value(), fake);
        expect(rel.has_value()).toBeFalsy();
    });
});
}


inline void run_platform_tests() {
describe("Platform", {
    it("generic memcopy produces correct result", {
        char src[64];
        char dst[64] = {};
        for (int i = 0; i < 64; ++i) src[i] = static_cast<char>(i);
        simd::memcopy(dst, src, 64);
        for (int i = 0; i < 64; ++i) expect(dst[i] == src[i]).toBeTruthy();
    });

    it("generic memzero produces correct result", {
        char buf[64];
        for (int i = 0; i < 64; ++i) buf[i] = static_cast<char>(0xFF);
        simd::memzero(buf, 64);
        for (int i = 0; i < 64; ++i) expect(buf[i] == 0).toBeTruthy();
    });

    it("platform selection uses detect_platform", {
        auto info = detect_platform();
        // Generic fallback is always available
        char src[16] = {1};
        char dst[16] = {};
        simd::memcopy(dst, src, 16);
        expect(dst[0] == 1).toBeTruthy();

        // Architecture-specific optimizations would be selected here
        // based on info.has_sse2, info.has_avx2, info.has_neon, etc.
        (void)info;
    });

    it("detects environment", {
        auto info = detect_platform();
        expect(info.compiler != Compiler::Unknown).toBeTruthy();
        expect(info.arch != Architecture::Unknown).toBeTruthy();
        expect(info.os != OperatingSystem::Unknown).toBeTruthy();
        auto s = info.to_string();
        expect(s.find("Clang") != std::string::npos || s.find("GCC") != std::string::npos).toBeTruthy();
    });
});
}

inline void run_error_handling_tests() {
describe("Error Handling", {
    it("fails to destroy invalid identity", {
        DefaultRuntime rt;
        auto destroy = rt.destroy_block(Identity::invalid());
        expect(destroy.has_value()).toBeFalsy();
        expect(destroy.error().code() == ErrorCode::InvalidIdentity).toBeTruthy();
    });

    it("fails to destroy nonexistent block", {
        DefaultRuntime rt;
        auto destroy = rt.destroy_block(Identity{999});
        expect(destroy.has_value()).toBeFalsy();
        expect(destroy.error().code() == ErrorCode::BlockNotFound).toBeTruthy();
    });
});
}

int main(int argc, char* argv[]) {
    cest_init(argc, argv);
    run_identity_tests();
    run_flat_vector_tests();
    run_intrusive_list_tests();
    run_error_tests();
    run_command_tests();
    run_controller_tests();
    run_runtime_tests();
    run_platform_tests();
    run_error_handling_tests();
    run_event_tests();
    run_serialization_tests();
    run_relationship_tests();
    run_avl_tree_tests();
    run_rb_tree_tests();
    run_bit_representation_tests();
    run_data_layout_tests();
    run_fsm_tests();
    run_rule_system_tests();
    run_pipeline_tests();
    run_event_loop_tests();
    run_ephemeral_tests();
    run_scheduler_tests();
    run_integration_tests();
    run_stress_tests();
    run_fuzz_tests();
    run_e2e_tests();
    run_edge_case_tests();
    run_semantic_tests();
    return cest_result();
}
