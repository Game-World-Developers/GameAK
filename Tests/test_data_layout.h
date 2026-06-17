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

        auto& blocks = rt.blocks();
        expect(blocks.at(b1.value()).size() == sizeof(int) * 3).toBeTruthy();
        expect(blocks.at(b2.value()).size() == sizeof(int) * 3).toBeTruthy();
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

        expect(rt.blocks().size() == 3).toBeTruthy();
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.get_layout(2) == LayoutStrategy::SoA).toBeTruthy();
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
});
}
