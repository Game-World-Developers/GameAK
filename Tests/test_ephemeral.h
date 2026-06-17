#pragma once

namespace {
namespace ephemeral_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    constexpr uint32_t PERSISTENT_TYPE = 1;
    constexpr uint32_t EPHEMERAL_TYPE = 2;

    auto register_types(DefaultRuntime& rt) -> void {
        BlockTypeDescriptor persistent;
        persistent.type_id = PERSISTENT_TYPE;
        persistent.size = sizeof(int);
        persistent.alignment = alignof(int);
        persistent.name = "persistent";
        persistent.ephemeral = false;
        auto _a = rt.register_block_type(persistent);
        (void)_a;

        BlockTypeDescriptor ephemeral;
        ephemeral.type_id = EPHEMERAL_TYPE;
        ephemeral.size = sizeof(int);
        ephemeral.alignment = alignof(int);
        ephemeral.name = "ephemeral";
        ephemeral.ephemeral = true;
        auto _b = rt.register_block_type(ephemeral);
        (void)_b;
    }

    void test_create_ephemeral_without_command() {
        DefaultRuntime rt;
        register_types(rt);

        auto id = rt.create_ephemeral_block(EPHEMERAL_TYPE);
        expect(id.has_value()).toBeTruthy();
        expect(rt.has_block(id.value())).toBeTruthy();
    }

    void test_rejects_create_for_non_ephemeral_type() {
        DefaultRuntime rt;
        register_types(rt);

        auto id = rt.create_ephemeral_block(PERSISTENT_TYPE);
        expect(id.has_value()).toBeFalsy();
    }

    void test_ephemeral_destroyed_at_tick_end() {
        DefaultRuntime rt;
        register_types(rt);

        auto id = rt.create_ephemeral_block(EPHEMERAL_TYPE);
        expect(id.has_value()).toBeTruthy();
        expect(rt.has_block(id.value())).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(rt.has_block(id.value())).toBeFalsy();
    }

    void test_ephemeral_visible_to_other_controllers_same_tick() {
        DefaultRuntime rt;
        register_types(rt);

        Identity created_id;
        bool second_saw = false;

        auto creator = [&](StateView&, CommandProducer&, EphemeralProducer& ephem) -> Result<void> {
            auto r = ephem.create(EPHEMERAL_TYPE);
            if (!r) return r.error();
            created_id = r.value();
            return {};
        };

        auto checker = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            if (created_id.is_valid()) {
                second_saw = view.has_block(created_id);
            }
            return {};
        };

        expect(rt.register_controller(std::move(creator), 10).has_value()).toBeTruthy();
        expect(rt.register_controller(std::move(checker), 0).has_value()).toBeTruthy();

        auto r = rt.tick();
        expect(r.status == ExecutionStatus::Success).toBeTruthy();
        expect(second_saw).toBeTruthy();
    }

    void test_ephemeral_not_in_snapshot() {
        DefaultRuntime rt;
        register_types(rt);

        auto id = rt.create_ephemeral_block(EPHEMERAL_TYPE);
        expect(id.has_value()).toBeTruthy();
        auto pid = rt.create_block(PERSISTENT_TYPE);
        expect(pid.has_value()).toBeTruthy();
        rt.tick();

        auto snap = rt.save();
        // After tick, ephemeral is gone. check that it's not in save
        for (const auto& [sid, block] : snap.blocks) {
            (void)sid;
            if (block.type_id() == EPHEMERAL_TYPE) {
                expect(false).toBeTruthy(); // should not be reached
            }
        }
        // Serialization test: persistent blocks survive tick
        expect(snap.blocks.size() == 1).toBeTruthy();
    }

    void test_persistent_not_affected_by_ephemeral_destruction() {
        DefaultRuntime rt;
        register_types(rt);

        auto pid = rt.create_block(PERSISTENT_TYPE);
        expect(pid.has_value()).toBeTruthy();

        auto eid = rt.create_ephemeral_block(EPHEMERAL_TYPE);
        expect(eid.has_value()).toBeTruthy();

        rt.tick();
        expect(rt.has_block(pid.value())).toBeTruthy();
        expect(rt.has_block(eid.value())).toBeFalsy();
    }
}
}

inline void run_ephemeral_tests() {
describe("Ephemeral Data Blocks", {
    it("create ephemeral without command",        { ephemeral_helpers::test_create_ephemeral_without_command(); });
    it("rejects create for non-ephemeral type",   { ephemeral_helpers::test_rejects_create_for_non_ephemeral_type(); });
    it("ephemeral blocks are destroyed at tick end", { ephemeral_helpers::test_ephemeral_destroyed_at_tick_end(); });
    it("ephemeral visible to other controllers same tick", { ephemeral_helpers::test_ephemeral_visible_to_other_controllers_same_tick(); });
    it("ephemeral not included in snapshot",      { ephemeral_helpers::test_ephemeral_not_in_snapshot(); });
    it("persistent not affected by ephemeral destruction", { ephemeral_helpers::test_persistent_not_affected_by_ephemeral_destruction(); });
});
}
