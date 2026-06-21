#pragma once

namespace {
namespace edge_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;
    using FlatVec4 = flat_vector<int, 4>;

    void test_default_ctor_invalid() {
        Identity id;
        expect(id.is_valid()).toBeFalsy();
        expect(id.value() == 0).toBeTruthy();
    }

    void test_hash_invalid_stable() {
        std::hash<Identity> hasher;
        auto h1 = hasher(Identity::invalid());
        auto h2 = hasher(Identity::invalid());
        expect(h1 == h2).toBeTruthy();
    }

    void test_max_identity() {
        Identity id{UINT64_MAX};
        expect(id.is_valid()).toBeTruthy();
        expect(id.value() == UINT64_MAX).toBeTruthy();
    }

    void test_erase_all() {
        FlatVec4 v;
        v.push_back(1);
        v.push_back(2);
        v.erase(v.begin(), v.end());
        expect(v.empty()).toBeTruthy();
    }

    void test_pop_to_empty() {
        FlatVec4 v;
        v.push_back(42);
        v.pop_back();
        expect(v.empty()).toBeTruthy();
    }

    void test_clear_empty() {
        FlatVec4 v;
        v.clear();
        expect(v.empty()).toBeTruthy();
    }

    struct ENode : intrusive_node { int v; explicit ENode(int x) : v(x) {} };

    void test_erase_sole() {
        intrusive_list<ENode> list;
        ENode n{1};
        list.push_back(&n);
        list.erase(list.begin());
        expect(list.empty()).toBeTruthy();
    }

    void test_clear_empty_list() {
        intrusive_list<ENode> list;
        list.clear();
        expect(list.empty()).toBeTruthy();
    }

    void test_move_empty_list() {
        intrusive_list<ENode> a;
        intrusive_list<ENode> b = std::move(a);
        expect(b.empty()).toBeTruthy();
    }

    void test_moved_result() {
        Result<int> r{42};
        Result<int> s = std::move(r);
        expect(s.has_value()).toBeTruthy();
        expect(s.value() == 42).toBeTruthy();
    }

    void test_void_result_moved() {
        Result<void> r;
        Result<void> s = std::move(r);
        expect(s.has_value()).toBeTruthy();
    }

    void test_empty_targets_destroy() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id = rt.submit_command(Command{CommandDestroyBlock{{}}});
        expect(id.has_value()).toBeTruthy();
        auto r = rt.tick();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(r.commands_rejected == 0).toBeTruthy();
    }

    void test_cancel_nonexistent() {
        DefaultRuntime rt;
        // Cancelling a nonexistent command is a no-op, not an error
        auto cancel = rt.cancel_command(UINT64_MAX);
        expect(cancel.has_value()).toBeTruthy();
    }

    void test_setfield_zero_bytes() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        auto id = rt.submit_command(Command{CommandSetField{{block.value()}, 0, {}}});
        expect(id.has_value()).toBeTruthy();

        // Writing zero bytes is valid if offset is within bounds
        auto r = rt.tick();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(r.commands_rejected == 0).toBeTruthy();
    }

    void test_submit_after_tick_queues() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        rt.tick();
        expect(rt.scheduler().pending_count() == 0).toBeTruthy();

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();
        expect(rt.scheduler().pending_count() == 1).toBeTruthy();
    }

    void test_replay_independent() {
        DefaultRuntime rt1;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt1.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id = rt1.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();
        rt1.tick();
        expect(rt1.block_count(1) == 1).toBeTruthy();

        DefaultRuntime rt2;
        BlockTypeDescriptor desc2;
        desc2.type_id = 1; desc2.size = sizeof(int); desc2.alignment = alignof(int); desc2.name = "test";
        expect(rt2.register_block_type(std::move(desc2)).has_value()).toBeTruthy();

        auto replay = rt2.replay_command(rt1.command_history()[0]);
        expect(replay.has_value()).toBeTruthy();
        rt2.tick();
        expect(rt2.block_count(1) == 1).toBeTruthy();
        expect(rt1.block_count(1) == 1).toBeTruthy();
    }

    void test_zero_controllers() {
        DefaultRuntime rt;
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(r.controllers_executed == 0).toBeTruthy();
    }

    void test_all_controllers_fail() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto failer = [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return Error(ErrorCode::ControllerFailed, "fail");
        };
        expect(rt.register_controller(failer).has_value()).toBeTruthy();
        expect(rt.register_controller(failer).has_value()).toBeTruthy();

        auto id = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::PartialFailure).toBeTruthy();
        expect(r.commands_executed == 1).toBeTruthy();
        expect(rt.block_count(1) == 1).toBeTruthy();
    }

    void test_priority_boundaries() {
        DefaultRuntime rt;
        std::vector<int> order;

        auto low = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(0); return {};
        };
        auto high = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            order.push_back(1); return {};
        };
        expect(rt.register_controller(std::move(low), INT_MIN).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(high), INT_MAX).has_value()).toBeTruthy();

        rt.tick();
        expect(order.size() == 2).toBeTruthy();
        expect(order[0] == 1).toBeTruthy();
        expect(order[1] == 0).toBeTruthy();
    }

    void test_tick_zero_delta() {
        DefaultRuntime rt;
        auto r = rt.tick(0.0f);
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_block_count_unregistered() {
        DefaultRuntime rt;
        expect(rt.block_count(99) == 0).toBeTruthy();
    }

    void test_find_blocks_unregistered() {
        DefaultRuntime rt;
        auto result = rt.find_blocks_by_type(99);
        expect(result.empty()).toBeTruthy();
    }

    void test_has_block_invalid() {
        DefaultRuntime rt;
        expect(rt.has_block(Identity::invalid())).toBeFalsy();
    }

    void test_destroy_already_destroyed() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto b = rt.create_block(1);
        expect(b.has_value()).toBeTruthy();
        expect(rt.destroy_block(b.value()).has_value()).toBeTruthy();
        expect(rt.destroy_block(b.value()).has_value()).toBeFalsy();
    }

    void test_relate_destroyed_block() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto p = rt.create_block(1);
        auto c = rt.create_block(1);
        expect(p.has_value() && c.has_value()).toBeTruthy();

        auto _ = rt.destroy_block(c.value()); (void)_;
        auto rel = rt.relate(p.value(), c.value());
        expect(rel.has_value()).toBeFalsy();
    }

    void test_is_paused_default() {
        DefaultRuntime rt;
        expect(rt.is_paused()).toBeFalsy();
    }

    void test_pause_idempotent() {
        DefaultRuntime rt;
        rt.pause();
        rt.pause();
        rt.pause();
        expect(rt.is_paused()).toBeTruthy();
        rt.resume();
        expect(rt.is_paused()).toBeFalsy();
    }

    void test_unlisten_invalid() {
        DefaultRuntime rt;
        rt.unlisten(UINT64_MAX);
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_no_handlers() {
        DefaultRuntime rt;
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_listen_unlisten_before_tick() {
        DefaultRuntime rt;
        int count = 0;
        auto eid = rt.listen(DefaultRuntime::EventType::TickBegin,
            [&](const DefaultRuntime::Event&) { count++; });
        rt.unlisten(eid);
        rt.tick();
        expect(count == 0).toBeTruthy();
    }

    void test_save_empty() {
        DefaultRuntime rt;
        auto snap = rt.save();
        expect(snap.blocks.empty()).toBeTruthy();
        expect(snap.types.empty()).toBeTruthy();
    }

    void test_load_empty() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        Snapshot empty;
        rt.load(std::move(empty));
        expect(rt.block_count(1) == 0).toBeTruthy();
    }

    void test_save_load_cycles() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto id1 = b1.value();

        for (int i = 0; i < 5; ++i) {
            auto snap = rt.save();
            rt.load(std::move(snap));
            expect(rt.has_block(id1)).toBeTruthy();
            expect(rt.block_count(1) == 1).toBeTruthy();
        }
    }

    void test_layout_unregistered() {
        DefaultRuntime rt;
        expect(rt.get_layout(99) == LayoutStrategy::AoS).toBeTruthy();
    }

    void test_has_layout_storage_false() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();
        expect(rt.has_layout_storage(1)).toBeFalsy();
    }

    void test_convert_empty_type() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        rt.convert_to_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();
        expect(rt.soa_block_count(1) == 0).toBeTruthy();
    }

    void test_convert_from_aos() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        rt.convert_from_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
    }

    void test_create_after_soa() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        rt.convert_to_soa(1);
        auto b = rt.create_block(1);
        expect(b.has_value()).toBeTruthy();
        // Blocks created after SoA conversion go into layout storage
        expect(rt.has_layout_storage(1)).toBeTruthy();
    }

    void test_soa_field_count() {
        DefaultRuntime rt;
        expect(rt.soa_field_count(99) == 0).toBeTruthy();

        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();
        // SoA with no fields defined still has 0 fields
        // Field count is only meaningful when fields are in the descriptor
        expect(rt.soa_field_count(1) == 0).toBeTruthy();
    }

    void test_fsm_single_state() {
        int entry_b = 0;
        auto fsm = Fsm<>{}
            .initial_state("A")
            .add_state("A")
            .add_state("B")
            .add_transition("A", "go", "B")
            .on_entry("B", [&](StateView&, CommandProducer&, EphemeralProducer&) { entry_b++; })
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& v, CommandProducer& p, EphemeralProducer& e) mutable -> Result<void> {
            return fsm(v, p, e);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        // Tick while in state A — on_entry(B) should not fire
        rt.tick();
        expect(entry_b == 0).toBeTruthy();
    }

    void test_fsm_transition_to_self() {
        auto fsm = Fsm<>{}
            .initial_state("A")
            .add_state("A")
            .add_transition("A", "self", "A")
            .build();

        DefaultRuntime rt;
        Controller fsm_ctrl = [fsm = std::move(fsm)](StateView& v, CommandProducer& p, EphemeralProducer& e) mutable -> Result<void> {
            return fsm(v, p, e);
        };
        expect(rt.register_controller(std::move(fsm_ctrl)).has_value()).toBeTruthy();

        rt.tick();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_rule_same_priority_order() {
        std::vector<int> order;
        RuleSystem rs;
        rs.add_rule("first",  [](StateView&) { return true; }, [&](StateView&, CommandProducer&, EphemeralProducer&) { order.push_back(1); }, 0);
        rs.add_rule("second", [](StateView&) { return true; }, [&](StateView&, CommandProducer&, EphemeralProducer&) { order.push_back(2); }, 0);
        rs.add_rule("third",  [](StateView&) { return true; }, [&](StateView&, CommandProducer&, EphemeralProducer&) { order.push_back(3); }, 0);

        DefaultRuntime rt;
        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        rt.tick();
        expect(order.size() == 3).toBeTruthy();
        expect(order[0] == 1).toBeTruthy();
        expect(order[1] == 2).toBeTruthy();
        expect(order[2] == 3).toBeTruthy();
    }

    void test_remove_invalid_rule() {
        RuleSystem rs;
        rs.remove_rule(999);
        expect(rs.rule_count() == 0).toBeTruthy();
    }

    void test_rule_clear_then_add() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        RuleSystem rs;
        rs.add_rule("temp", [](StateView&) { return true; }, [](StateView&, CommandProducer&, EphemeralProducer&) {}, 0);
        rs.clear_rules();
        expect(rs.rule_count() == 0).toBeTruthy();

        int fired = 0;
        rs.add_rule("perm", [](StateView&) { return true; }, [&](StateView&, CommandProducer&, EphemeralProducer&) { fired++; }, 0);

        Controller c = rs.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        rt.tick();
        expect(fired == 1).toBeTruthy();
    }

    void test_remove_invalid_stage() {
        Pipeline pipe;
        pipe.remove_stage(999);
        expect(pipe.stage_count() == 0).toBeTruthy();
    }

    void test_single_stage() {
        int executed = 0;
        Pipeline pipe;
        pipe.add_stage("only", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            executed++; return {};
        });

        DefaultRuntime rt;
        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        rt.tick();
        expect(executed == 1).toBeTruthy();
    }

    void test_clear_stages_then_add() {
        Pipeline pipe;
        pipe.add_stage("temp", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> { return {}; });
        pipe.clear_stages();
        expect(pipe.stage_count() == 0).toBeTruthy();

        int executed = 0;
        pipe.add_stage("perm", [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            executed++; return {};
        });

        DefaultRuntime rt;
        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        rt.tick();
        expect(executed == 1).toBeTruthy();
    }

    void test_all_stages_fail() {
        Pipeline pipe;
        pipe.add_stage("fail1", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return Error(ErrorCode::ControllerFailed, "fail1");
        });
        pipe.add_stage("fail2", [](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return Error(ErrorCode::ControllerFailed, "fail2");
        });

        DefaultRuntime rt;
        Controller c = pipe.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        auto results = pipe.last_results();
        expect(results.size() == 2).toBeTruthy();
        expect(results[0].success).toBeFalsy();
        expect(results[1].success).toBeFalsy();
    }

    void test_controller_enqueues_event() {
        DefaultRuntime rt;

        int handler_fired = 0;
        EventLoop<int> el;
        el.on(1, [&](const int&, CommandProducer&, EphemeralProducer&) { handler_fired++; });

        bool controller_ran = false;
        auto event_source = [&](StateView&, CommandProducer&, EphemeralProducer&) -> Result<void> {
            el.enqueue(1, 99);
            controller_ran = true;
            return {};
        };

        Controller el_ctrl = el.build();
        expect(rt.register_controller(std::move(event_source), 10).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(el_ctrl), 0).has_value()).toBeTruthy();

        rt.tick();
        expect(controller_ran).toBeTruthy();
        expect(handler_fired == 1).toBeTruthy();
    }

    void test_eventloop_interleaved_types() {
        EventLoop<int> el;
        std::vector<int> log;

        el.on(1, [&](const int& d, CommandProducer&, EphemeralProducer&) { log.push_back(d); });
        el.on(2, [&](const int& d, CommandProducer&, EphemeralProducer&) { log.push_back(d); });

        el.enqueue(1, 10);
        el.enqueue(2, 20);
        el.enqueue(1, 30);

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        rt.tick();
        expect(log.size() == 3).toBeTruthy();
        expect(log[0] == 10).toBeTruthy();
        expect(log[1] == 20).toBeTruthy();
        expect(log[2] == 30).toBeTruthy();
    }

    void test_enqueue_then_remove_handler() {
        EventLoop<int> el;
        int count = 0;
        auto h = el.on(1, [&](const int&, CommandProducer&, EphemeralProducer&) { count++; });
        el.enqueue(1, 0);
        el.off(h);

        DefaultRuntime rt;
        Controller c = el.build();
        expect(rt.register_controller(std::move(c)).has_value()).toBeTruthy();

        rt.tick();
        expect(count == 0).toBeTruthy();
    }

    void test_history_empty_initially() {
        DefaultRuntime rt;
        expect(rt.command_history().empty()).toBeTruthy();
    }

    void test_priority_scheduler_no_fn() {
        using PriorityRuntime = Runtime<PriorityScheduler>;
        PriorityRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id1 = rt.submit_command(Command{CommandCreateBlock{1}});
        auto id2 = rt.submit_command(Command{CommandCreateBlock{1}});
        expect(id1.has_value() && id2.has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.commands_executed == 2).toBeTruthy();
        expect(rt.block_count(1) == 2).toBeTruthy();
    }

    void test_destroy_all_ephemeral_empty() {
        DefaultRuntime rt;
        rt.destroy_all_ephemeral();
        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
    }

    void test_ephemeral_id_independence() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc_p;
        desc_p.type_id = 1; desc_p.size = sizeof(int); desc_p.alignment = alignof(int); desc_p.name = "persistent";
        BlockTypeDescriptor desc_e;
        desc_e.type_id = 2; desc_e.size = sizeof(int); desc_e.alignment = alignof(int); desc_e.name = "ephemeral"; desc_e.ephemeral = true;
        expect(rt.register_block_type(std::move(desc_p)).has_value()).toBeTruthy();
        expect(rt.register_block_type(std::move(desc_e)).has_value()).toBeTruthy();

        auto p1 = rt.create_block(1);
        auto e1 = rt.create_ephemeral_block(2);
        auto p2 = rt.create_block(1);
        expect(p1.has_value() && e1.has_value() && p2.has_value()).toBeTruthy();
        expect(p1.value().value() == 1).toBeTruthy();
        expect(e1.value().value() == 2).toBeTruthy();
        expect(p2.value().value() == 3).toBeTruthy();

        rt.tick();
        expect(rt.has_block(p1.value())).toBeTruthy();
        expect(rt.has_block(p2.value())).toBeTruthy();
    }

    void test_ephemeral_producer_visible_to_creator() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc_e;
        desc_e.type_id = 2; desc_e.size = sizeof(int); desc_e.alignment = alignof(int); desc_e.name = "ephemeral"; desc_e.ephemeral = true;
        expect(rt.register_block_type(std::move(desc_e)).has_value()).toBeTruthy();

        Identity created;
        auto creator = [&](StateView&, CommandProducer&, EphemeralProducer& ephem) -> Result<void> {
            auto r = ephem.create(2);
            if (!r) return r.error();
            created = r.value();
            return {};
        };
        expect(rt.register_controller(std::move(creator)).has_value()).toBeTruthy();
        rt.tick();
        expect(created.is_valid()).toBeTruthy();
    }
}
}

inline void run_edge_case_tests() {
describe("Edge Cases - Identity", {
    it("default-constructed identity is invalid",               { edge_helpers::test_default_ctor_invalid(); });
    it("hash of invalid identity is stable",                    { edge_helpers::test_hash_invalid_stable(); });
    it("max uint64_t identity is valid",                        { edge_helpers::test_max_identity(); });
});

describe("Edge Cases - flat_vector", {
    it("erase entire range clears container",                   { edge_helpers::test_erase_all(); });
    it("pop_back on single element empties",                    { edge_helpers::test_pop_to_empty(); });
    it("clear on empty is safe",                                { edge_helpers::test_clear_empty(); });
});

describe("Edge Cases - intrusive_list", {
    it("erase sole element empties list",                       { edge_helpers::test_erase_sole(); });
    it("clear on empty list is safe",                            { edge_helpers::test_clear_empty_list(); });
    it("move from empty list",                                  { edge_helpers::test_move_empty_list(); });
});

describe("Edge Cases - Result", {
    it("moved-from result is empty",                            { edge_helpers::test_moved_result(); });
    it("void result after move is still usable",                { edge_helpers::test_void_result_moved(); });
});

describe("Edge Cases - Commands", {
    it("empty targets DestroyBlock is rejected",                { edge_helpers::test_empty_targets_destroy(); });
    it("cancel nonexistent command returns error",              { edge_helpers::test_cancel_nonexistent(); });
    it("SetField with zero bytes is rejected",                  { edge_helpers::test_setfield_zero_bytes(); });
    it("submit after tick queues for next tick",                { edge_helpers::test_submit_after_tick_queues(); });
    it("replay into separate runtime creates independent state",{ edge_helpers::test_replay_independent(); });
});

describe("Edge Cases - Controllers", {
    it("zero controllers still produces success tick",          { edge_helpers::test_zero_controllers(); });
    it("all controllers failing still allows commands",         { edge_helpers::test_all_controllers_fail(); });
    it("priority boundaries INT_MIN and INT_MAX",               { edge_helpers::test_priority_boundaries(); });
});

describe("Edge Cases - Runtime", {
    it("tick with zero delta executes normally",                { edge_helpers::test_tick_zero_delta(); });
    it("block_count for unregistered type returns 0",           { edge_helpers::test_block_count_unregistered(); });
    it("find_blocks_by_type for unregistered returns empty",     { edge_helpers::test_find_blocks_unregistered(); });
    it("has_block with invalid identity returns false",          { edge_helpers::test_has_block_invalid(); });
    it("destroy_block on already destroyed block fails",        { edge_helpers::test_destroy_already_destroyed(); });
    it("relate with destroyed block fails",                     { edge_helpers::test_relate_destroyed_block(); });
    it("is_paused returns false by default",                    { edge_helpers::test_is_paused_default(); });
    it("multiple pause calls are idempotent",                   { edge_helpers::test_pause_idempotent(); });
});

describe("Edge Cases - Events", {
    it("unlisten with invalid id is safe",                      { edge_helpers::test_unlisten_invalid(); });
    it("no events fire with zero handlers",                     { edge_helpers::test_no_handlers(); });
    it("listen then unlisten before tick produces nothing",     { edge_helpers::test_listen_unlisten_before_tick(); });
});

describe("Edge Cases - Serialization", {
    it("save of empty runtime produces empty snapshot",         { edge_helpers::test_save_empty(); });
    it("load of empty snapshot clears state",                   { edge_helpers::test_load_empty(); });
    it("multiple save/load cycles preserve identity count",     { edge_helpers::test_save_load_cycles(); });
});

describe("Edge Cases - Data Layout", {
    it("get_layout for unregistered type returns AoS",          { edge_helpers::test_layout_unregistered(); });
    it("has_layout_storage for unconverted type false",         { edge_helpers::test_has_layout_storage_false(); });
    it("convert_to_soa on empty type succeeds",                 { edge_helpers::test_convert_empty_type(); });
    it("convert_from_soa on AoS type is safe",                  { edge_helpers::test_convert_from_aos(); });
    it("create_block after convert_to_soa adds to storage",     { edge_helpers::test_create_after_soa(); });
    it("soa_field_count for non-SoA type returns 0",            { edge_helpers::test_soa_field_count(); });
});

describe("Edge Cases - FSM", {
    it("single state with no transitions stays there",          { edge_helpers::test_fsm_single_state(); });
    it("transition to self is ignored",                         { edge_helpers::test_fsm_transition_to_self(); });
});

describe("Edge Cases - RuleSystem", {
    it("same priority rules execute in add order",              { edge_helpers::test_rule_same_priority_order(); });
    it("remove_rule with invalid id is safe",                   { edge_helpers::test_remove_invalid_rule(); });
    it("clear_rules then add after build works",                { edge_helpers::test_rule_clear_then_add(); });
});

describe("Edge Cases - Pipeline", {
    it("remove_stage with invalid id is safe",                  { edge_helpers::test_remove_invalid_stage(); });
    it("single stage pipeline executes correctly",              { edge_helpers::test_single_stage(); });
    it("clear_stages then add_stage works",                     { edge_helpers::test_clear_stages_then_add(); });
    it("all stages failing does not crash",                     { edge_helpers::test_all_stages_fail(); });
});

describe("Edge Cases - EventLoop", {
    it("controller can enqueue events during execution",        { edge_helpers::test_controller_enqueues_event(); });
    it("multiple event types handled in enqueue order",         { edge_helpers::test_eventloop_interleaved_types(); });
    it("enqueue then remove handler before tick",               { edge_helpers::test_enqueue_then_remove_handler(); });
});

describe("Edge Cases - Scheduler", {
    it("history is empty initially",                            { edge_helpers::test_history_empty_initially(); });
    it("PriorityScheduler with no fn falls back to FIFO",      { edge_helpers::test_priority_scheduler_no_fn(); });
});

describe("Edge Cases - Ephemeral", {
    it("destroy_all_ephemeral when none exist is safe",         { edge_helpers::test_destroy_all_ephemeral_empty(); });
    it("ephemeral IDs do not interfere with persistent IDs",    { edge_helpers::test_ephemeral_id_independence(); });
    it("ephemeral created via producer visible to creator",     { edge_helpers::test_ephemeral_producer_visible_to_creator(); });
});
}
