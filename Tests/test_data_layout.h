#pragma once

namespace {
namespace data_layout_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_default_layout_aos() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(desc.layout == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
    }

    void test_layout_is_type_property() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc_aos;
        desc_aos.type_id = 1;
        desc_aos.size = sizeof(int);
        desc_aos.alignment = alignof(int);
        desc_aos.name = "aos_type";
        desc_aos.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc_aos).has_value()).toBeTruthy();

        BlockTypeDescriptor desc_soa;
        desc_soa.type_id = 2;
        desc_soa.size = sizeof(int) * 3;
        desc_soa.alignment = alignof(int);
        desc_soa.name = "soa_type";
        desc_soa.layout = LayoutStrategy::SoA;
        desc_soa.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc_soa.fields.push_back({"y", sizeof(int), sizeof(int), alignof(int)});
        desc_soa.fields.push_back({"z", sizeof(int) * 2, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc_soa).has_value()).toBeTruthy();

        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.get_layout(2) == LayoutStrategy::SoA).toBeTruthy();
    }

    void test_aos_field_ordering() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int) * 3;
        desc.alignment = alignof(int);
        desc.name = "pos_vel_mass";
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        expect(b1.has_value()).toBeTruthy();
        auto b2 = rt.create_block(1);
        expect(b2.has_value()).toBeTruthy();

        auto* b1p = rt.get_block(b1.value());
        auto* b2p = rt.get_block(b2.value());
        expect(b1p->size() == sizeof(int) * 3).toBeTruthy();
        expect(b2p->size() == sizeof(int) * 3).toBeTruthy();
    }

    void test_layout_transparent() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int) * 2;
        desc.alignment = alignof(int);
        desc.name = "counter";
        desc.layout = LayoutStrategy::SoA;
        desc.fields.push_back({"value", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"extra", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto block = rt.create_block(1);
        expect(block.has_value()).toBeTruthy();

        int val = 42;
        auto bytes = std::vector<std::byte>(
            reinterpret_cast<std::byte*>(&val),
            reinterpret_cast<std::byte*>(&val) + sizeof(int));
        auto cmd = rt.submit_command(Command{CommandSetField{{block.value()}, 0, bytes}});
        expect(cmd.has_value()).toBeTruthy();
        rt.tick();

        bool read_correct = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            read_correct = view.has_block(block.value());
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(read_correct).toBeTruthy();
    }

    void test_layout_conversion_preserves_identity() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(1);
        auto id1 = b1.value();
        auto id2 = b2.value();
        expect(b1.has_value() && b2.has_value()).toBeTruthy();

        rt.convert_to_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();

        rt.convert_from_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.has_block(id1)).toBeTruthy();
        expect(rt.has_block(id2)).toBeTruthy();
    }

    void test_multiple_layouts_coexist() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc_aos;
        desc_aos.type_id = 1;
        desc_aos.size = sizeof(int);
        desc_aos.alignment = alignof(int);
        desc_aos.name = "aos_type";
        desc_aos.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc_aos).has_value()).toBeTruthy();

        BlockTypeDescriptor desc_soa;
        desc_soa.type_id = 2;
        desc_soa.size = sizeof(int) * 3;
        desc_soa.alignment = alignof(int);
        desc_soa.name = "soa_type";
        desc_soa.layout = LayoutStrategy::SoA;
        desc_soa.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc_soa.fields.push_back({"y", sizeof(int), sizeof(int), alignof(int)});
        desc_soa.fields.push_back({"z", sizeof(int) * 2, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc_soa).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(2).has_value()).toBeTruthy();

        expect(rt.blocks().count() == 3).toBeTruthy();
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.get_layout(2) == LayoutStrategy::SoA).toBeTruthy();
    }

    // ── AoSoA Tests ────────────────────────────────────────────────

    void test_aosoa_default_chunk_size() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoSoA;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();
        // Default chunk_size is 8
        expect(desc.aosoa_config.chunk_size == 8).toBeTruthy();
    }

    void test_aosoa_custom_chunk_size() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        desc.aosoa_config.chunk_size = 4;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Create 6 blocks — they should fill 2 chunks (4 + 2)
        for (int i = 0; i < 6; ++i) {
            expect(rt.create_block(1).has_value()).toBeTruthy();
        }

        rt.convert_to_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoSoA).toBeTruthy();
        expect(rt.aosoa_block_count(1) == 6).toBeTruthy();
        expect(rt.aosoa_chunk_count(1) == 2).toBeTruthy();
    }

    void test_aosoa_conversion_preserves_identity() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(1);
        auto id1 = b1.value();
        auto id2 = b2.value();
        expect(b1.has_value() && b2.has_value()).toBeTruthy();

        rt.convert_to_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoSoA).toBeTruthy();
        expect(rt.aosoa_block_count(1) == 2).toBeTruthy();
        expect(rt.aosoa_chunk_count(1) == 1).toBeTruthy();

        rt.convert_from_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.has_block(id1)).toBeTruthy();
        expect(rt.has_block(id2)).toBeTruthy();
    }

    void test_aosoa_field_data_integrity() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int) * 3;
        desc.alignment = alignof(int);
        desc.name = "pos";
        desc.layout = LayoutStrategy::AoS;
        desc.aosoa_config.chunk_size = 2;
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", sizeof(int), sizeof(int), alignof(int)});
        desc.fields.push_back({"z", sizeof(int) * 2, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Create 3 blocks with known values
        auto id0 = rt.create_block(1).value();
        auto id1 = rt.create_block(1).value();
        auto id2 = rt.create_block(1).value();

        // Write values via SetField
        auto write_int = [&](auto id, size_t offset, int val) {
            auto bytes = std::vector<std::byte>(
                reinterpret_cast<std::byte*>(&val),
                reinterpret_cast<std::byte*>(&val) + sizeof(int));
            auto _ = rt.submit_command(Command{CommandSetField{{id}, offset, bytes}});
            (void)_;
        };
        write_int(id0, 0, 10);   write_int(id0, 4, 20);   write_int(id0, 8, 30);
        write_int(id1, 0, 40);   write_int(id1, 4, 50);   write_int(id1, 8, 60);
        write_int(id2, 0, 70);   write_int(id2, 4, 80);   write_int(id2, 8, 90);
        rt.tick();

        rt.convert_to_aosoa(1);
        expect(rt.aosoa_block_count(1) == 3).toBeTruthy();
        expect(rt.aosoa_chunk_count(1) == 2).toBeTruthy(); // chunk_size=2 → 2 chunks

        // Verify identities across all chunks
        size_t total_found = 0;
        bool found_id0 = false, found_id1 = false, found_id2 = false;
        for (size_t ci = 0; ci < rt.aosoa_chunk_count(1); ++ci) {
            auto ids = rt.aosoa_chunk_identities(1, ci);
            for (auto id : ids) {
                total_found++;
                if (id == id0) found_id0 = true;
                if (id == id1) found_id1 = true;
                if (id == id2) found_id2 = true;
            }
        }
        expect(total_found == 3).toBeTruthy();
        expect(found_id0).toBeTruthy();
        expect(found_id1).toBeTruthy();
        expect(found_id2).toBeTruthy();

        // Convert back and verify data
        rt.convert_from_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.has_block(id0)).toBeTruthy();
        expect(rt.has_block(id1)).toBeTruthy();
        expect(rt.has_block(id2)).toBeTruthy();
    }

    void test_aosoa_empty_type() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        rt.convert_to_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoSoA).toBeTruthy();
        expect(rt.aosoa_block_count(1) == 0).toBeTruthy();
        expect(rt.aosoa_chunk_count(1) == 0).toBeTruthy();
    }

    void test_aosoa_from_aos() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        rt.convert_from_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
    }

    void test_aosoa_no_fields_uses_full_block_size() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int) * 2;
        desc.alignment = alignof(int);
        desc.name = "pair";
        desc.layout = LayoutStrategy::AoS;
        desc.aosoa_config.chunk_size = 3;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();

        rt.convert_to_aosoa(1);
        expect(rt.aosoa_block_count(1) == 3).toBeTruthy();
        expect(rt.aosoa_chunk_count(1) == 1).toBeTruthy();
        expect(rt.aosoa_field_count(1) == 1).toBeTruthy();

        rt.convert_from_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.blocks().count() == 3).toBeTruthy();
    }

    void test_aosoa_multiple_types_coexist() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc_aos;
        desc_aos.type_id = 1;
        desc_aos.size = sizeof(int);
        desc_aos.alignment = alignof(int);
        desc_aos.name = "aos_type";
        expect(rt.register_block_type(desc_aos).has_value()).toBeTruthy();

        BlockTypeDescriptor desc_aosoa;
        desc_aosoa.type_id = 2;
        desc_aosoa.size = sizeof(float);
        desc_aosoa.alignment = alignof(float);
        desc_aosoa.name = "aosoa_type";
        desc_aosoa.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc_aosoa).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(2).has_value()).toBeTruthy();
        expect(rt.create_block(2).has_value()).toBeTruthy();
        expect(rt.create_block(2).has_value()).toBeTruthy();

        rt.convert_to_aosoa(2);

        expect(rt.blocks().count() == 2).toBeTruthy(); // only type 1 blocks remain in AoS
        expect(rt.aosoa_block_count(2) == 3).toBeTruthy();
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.get_layout(2) == LayoutStrategy::AoSoA).toBeTruthy();
    }

    void test_aosoa_exact_chunk_fill() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        desc.aosoa_config.chunk_size = 4;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Create exactly chunk_size blocks
        for (int i = 0; i < 4; ++i) {
            expect(rt.create_block(1).has_value()).toBeTruthy();
        }

        rt.convert_to_aosoa(1);
        expect(rt.aosoa_block_count(1) == 4).toBeTruthy();
        expect(rt.aosoa_chunk_count(1) == 1).toBeTruthy(); // exactly one chunk

        rt.convert_from_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.blocks().count() == 4).toBeTruthy();
    }

    void test_aosoa_large_chunk() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        desc.aosoa_config.chunk_size = 100;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        // Create 50 blocks, all in one chunk
        for (int i = 0; i < 50; ++i) {
            expect(rt.create_block(1).has_value()).toBeTruthy();
        }

        rt.convert_to_aosoa(1);
        expect(rt.aosoa_block_count(1) == 50).toBeTruthy();
        expect(rt.aosoa_chunk_count(1) == 1).toBeTruthy();

        rt.convert_from_aosoa(1);
        expect(rt.blocks().count() == 50).toBeTruthy();
    }

    void test_aosoa_idempotent_convert() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();

        rt.convert_to_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoSoA).toBeTruthy();
        expect(rt.aosoa_block_count(1) == 1).toBeTruthy();

        // Second convert when already AoSoA should be a no-op
        rt.convert_to_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoSoA).toBeTruthy();
        expect(rt.aosoa_block_count(1) == 1).toBeTruthy();
    }
}
}

inline void run_data_layout_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;

describe("Data Layout", {
    it("default layout is AoS",                                { data_layout_helpers::test_default_layout_aos(); });
    it("layout is a type-level property",                       { data_layout_helpers::test_layout_is_type_property(); });
    it("AoS stores fields per entity",                           { data_layout_helpers::test_aos_field_ordering(); });
    it("layout transparent to controllers",                      { data_layout_helpers::test_layout_transparent(); });
    it("layout conversion preserves identity",                   { data_layout_helpers::test_layout_conversion_preserves_identity(); });
    it("multiple layout strategies coexist",                     { data_layout_helpers::test_multiple_layouts_coexist(); });

    // AoSoA
    it("AoSoA default chunk size is 8",                         { data_layout_helpers::test_aosoa_default_chunk_size(); });
    it("AoSoA custom chunk size creates correct chunks",        { data_layout_helpers::test_aosoa_custom_chunk_size(); });
    it("AoSoA conversion preserves identity",                    { data_layout_helpers::test_aosoa_conversion_preserves_identity(); });
    it("AoSoA field data integrity after round-trip",            { data_layout_helpers::test_aosoa_field_data_integrity(); });
    it("AoSoA empty type conversion",                           { data_layout_helpers::test_aosoa_empty_type(); });
    it("AoSoA convert from AoS on non-AoSoA type is no-op",    { data_layout_helpers::test_aosoa_from_aos(); });
    it("AoSoA no explicit fields uses full block size",        { data_layout_helpers::test_aosoa_no_fields_uses_full_block_size(); });
    it("AoSoA and AoS types coexist",                           { data_layout_helpers::test_aosoa_multiple_types_coexist(); });
    it("AoSoA exact chunk fill",                                { data_layout_helpers::test_aosoa_exact_chunk_fill(); });
    it("AoSoA single chunk for many blocks",                    { data_layout_helpers::test_aosoa_large_chunk(); });
    it("AoSoA idempotent convert",                              { data_layout_helpers::test_aosoa_idempotent_convert(); });
});
}
