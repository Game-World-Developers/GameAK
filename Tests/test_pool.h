#pragma once

namespace {
namespace pool_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_same_type_shares_pool() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = 64; desc.alignment = 64; desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id1 = rt.create_block(1);
        expect(id1.has_value()).toBeTruthy();
        auto id2 = rt.create_block(1);
        expect(id2.has_value()).toBeTruthy();
        expect(rt.has_block(id1.value())).toBeTruthy();
        expect(rt.has_block(id2.value())).toBeTruthy();
    }

    void test_pool_growth() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = 32; desc.alignment = 32; desc.name = "growth";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        for (int i = 0; i < 100; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
        }
        expect(rt.block_count(1) == 100).toBeTruthy();
    }

    void test_pool_slot_reuse() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = 16; desc.alignment = 16; desc.name = "reuse";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        gameak::core::flat_vector<gameak::core::Identity, 10> ids;
        for (int i = 0; i < 10; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            ids.push_back(id.value());
        }

        // Destroy slot 5
        expect(rt.destroy_block(ids[5]).has_value()).toBeTruthy();
        expect(rt.block_count(1) == 9).toBeTruthy();

        // Create new block — should reuse the freed slot
        auto new_id = rt.create_block(1);
        expect(new_id.has_value()).toBeTruthy();
        expect(rt.block_count(1) == 10).toBeTruthy();
    }

    void test_pool_per_type() {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor a;
            a.type_id = 1; a.size = 32; a.alignment = 32; a.name = "type_a";
            expect(rt.register_block_type(std::move(a)).has_value()).toBeTruthy();
        }
        {
            BlockTypeDescriptor b;
            b.type_id = 2; b.size = 128; b.alignment = 128; b.name = "type_b";
            expect(rt.register_block_type(std::move(b)).has_value()).toBeTruthy();
        }

        for (int i = 0; i < 10; ++i) {
            expect(rt.create_block(1).has_value()).toBeTruthy();
            expect(rt.create_block(2).has_value()).toBeTruthy();
        }
        expect(rt.block_count(1) == 10).toBeTruthy();
        expect(rt.block_count(2) == 10).toBeTruthy();
    }

    void test_pool_behavior_identical() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "int_block";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id_res = rt.create_block(1);
        expect(id_res.has_value()).toBeTruthy();
        auto id = id_res.value();

        // Write and read back — should work identically with pool
        CommandProducer producer{[&](Command cmd) -> Result<CommandId> {
            return rt.submit_command(std::move(cmd));
        }};
        int val = 42;
        expect(producer.set(id, 0, val).has_value()).toBeTruthy();
        rt.tick();

        auto* block = rt.get_block(id);
        expect(block != nullptr).toBeTruthy();
        expect(block->field<int>(0) == 42).toBeTruthy();
    }
}
}

inline void run_pool_tests() {
describe("Pooled Allocator", {
    it("same type shares pool",              { pool_helpers::test_same_type_shares_pool(); });
    it("pool grows on demand",               { pool_helpers::test_pool_growth(); });
    it("destroyed slots are reused",         { pool_helpers::test_pool_slot_reuse(); });
    it("pool is per-type",                   { pool_helpers::test_pool_per_type(); });
    it("pool behavior is identical",         { pool_helpers::test_pool_behavior_identical(); });
});
}
