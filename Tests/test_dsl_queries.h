#pragma once

inline void run_dsl_query_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;
describe("DSL - Fluent Queries", {
    it("blocks_query_count_by_name", {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor desc;
            desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "Player";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        }
        for (int i = 0; i < 5; i++)
            expect(rt.create_block(1).has_value()).toBeTruthy();

        size_t n = rt.blocks().of_type("Player").count();
        expect(n == 5).toBeTruthy();
    });

    it("blocks_query_count_by_type_id", {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor desc;
            desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "t1";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        }
        for (int i = 0; i < 3; i++)
            expect(rt.create_block(1).has_value()).toBeTruthy();

        size_t n = rt.blocks().of_type(1).count();
        expect(n == 3).toBeTruthy();
    });

    it("blocks_query_filter_then_map", {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor desc;
            desc.type_id = 1; desc.size = 16; desc.alignment = alignof(int); desc.name = "Player";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        }
        // Create blocks with varying sizes by size_of field simulation
        auto id1 = rt.create_block(1); expect(id1.has_value()).toBeTruthy();
        auto id2 = rt.create_block(1); expect(id2.has_value()).toBeTruthy();
        auto id3 = rt.create_block(1); expect(id3.has_value()).toBeTruthy();
        (void)id1; (void)id2; (void)id3;

        auto ids = rt.blocks()
            .of_type("Player")
            .where([](const DataBlock&) { return true; })
            .map<Identity>([](const DataBlock& b) { return b.identity(); });
        expect(ids.size() == 3).toBeTruthy();
    });

    it("blocks_query_each", {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor desc;
            desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "Player";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        }
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        int total = 0;
        rt.blocks().of_type("Player").each([&](const DataBlock&) {
            total++;
        });
        expect(total == 2).toBeTruthy();
    });

    it("blocks_query_any_all", {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor desc;
            desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "Player";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        }
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        auto pred = [](const DataBlock&) { return true; };
        bool any = rt.blocks().of_type("Player").any(pred);
        bool all = rt.blocks().of_type("Player").all(pred);
        expect(any).toBeTruthy();
        expect(all).toBeTruthy();

        auto false_pred = [](const DataBlock&) { return false; };
        expect(rt.blocks().of_type("Player").any(false_pred)).toBeFalsy();
        expect(rt.blocks().of_type("Player").all(false_pred)).toBeFalsy();
    });

    it("blocks_query_first", {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor desc;
            desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "Player";
            expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        }
        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        const DataBlock* b = rt.blocks().of_type("Player").first([](const DataBlock&) { return true; });
        expect(b != nullptr).toBeTruthy();
        expect(b->identity() == id.value()).toBeTruthy();
    });

    it("blocks_query_count_all", {
        DefaultRuntime rt;
        {
            BlockTypeDescriptor desc1;
            desc1.type_id = 1; desc1.size = sizeof(int); desc1.alignment = alignof(int); desc1.name = "t1";
            BlockTypeDescriptor desc2;
            desc2.type_id = 2; desc2.size = sizeof(double); desc2.alignment = alignof(double); desc2.name = "t2";
            expect(rt.register_block_type(desc1).has_value()).toBeTruthy();
            expect(rt.register_block_type(desc2).has_value()).toBeTruthy();
        }
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(2).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        auto n = rt.blocks().count();
        expect(n == 3).toBeTruthy();
    });

    it("blocks_query_empty", {
        DefaultRuntime rt;
        auto pred = [](const DataBlock&) { return true; };
        expect(rt.blocks().of_type("NonExistent").count() == 0).toBeTruthy();
        expect(rt.blocks().of_type("NonExistent").any(pred)).toBeFalsy();
        expect(rt.blocks().of_type("NonExistent").first(pred) == nullptr).toBeTruthy();
    });
});
}
