#pragma once

namespace {
namespace archetype_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    void test_archetype_opt_in() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "arch";
        desc.archetype_config.chunk_size = 64;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.archetype_block_count(1) == 1).toBeTruthy();
    }

    struct TestComponent {
        int x;
        int y;
        int z;
    };

    void test_archetype_dense_iteration() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(TestComponent);
        desc.alignment = alignof(TestComponent);
        desc.name = "pos";
        desc.layout = LayoutStrategy::AoS;
        desc.archetype_config.chunk_size = 64;
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", sizeof(int), sizeof(int), alignof(int)});
        desc.fields.push_back({"z", sizeof(int) * 2, sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        constexpr int N = 10;
        Identity ids[N];
        for (int i = 0; i < N; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            ids[i] = id.value();
        }

        for (int i = 0; i < N; ++i) {
            expect(rt.submit_command(Command::set_field(ids[i], 0, i * 100)).has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(ids[i], 4, i * 100 + 1)).has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(ids[i], 8, i * 100 + 2)).has_value()).toBeTruthy();
        }
        rt.tick();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.archetype_block_count(1) == N).toBeTruthy();
        expect(rt.archetype_chunk_count(1) == 1).toBeTruthy();

        auto x_data = rt.archetype_field_data(1, 0, 0);
        expect(x_data.size() == N * sizeof(int)).toBeTruthy();
        auto x_span = std::span<const int>(
            reinterpret_cast<const int*>(x_data.data()), N);
        for (int i = 0; i < N; ++i) {
            expect(x_span[i] == i * 100).toBeTruthy();
        }
    }

    void test_archetype_identity_lookup() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        desc.archetype_config.chunk_size = 64;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();
        Identity id_val = id.value();

        rt.convert_to_archetype(1);
        expect(rt.has_block(id_val)).toBeFalsy();
        expect(rt.archetype_block_count(1) == 1).toBeTruthy();
    }

    void test_archetype_conversion_preserves_identity() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto b1 = rt.create_block(1);
        auto b2 = rt.create_block(1);
        auto id1 = b1.value();
        auto id2 = b2.value();
        expect(b1.has_value() && b2.has_value()).toBeTruthy();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.archetype_block_count(1) == 2).toBeTruthy();

        rt.convert_from_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.has_block(id1)).toBeTruthy();
        expect(rt.has_block(id2)).toBeTruthy();
    }

    void test_archetype_chunk_overflow() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        desc.archetype_config.chunk_size = 4;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        for (int i = 0; i < 6; ++i) {
            expect(rt.create_block(1).has_value()).toBeTruthy();
        }

        rt.convert_to_archetype(1);
        expect(rt.archetype_chunk_count(1) == 2).toBeTruthy();
        expect(rt.archetype_block_count(1) == 6).toBeTruthy();
        expect(rt.archetype_chunk_identities(1, 0).size() == 4).toBeTruthy();
        expect(rt.archetype_chunk_identities(1, 1).size() == 2).toBeTruthy();
    }

    void test_archetype_coexists_with_other_layouts() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc_arch;
        desc_arch.type_id = 1;
        desc_arch.size = sizeof(int);
        desc_arch.alignment = alignof(int);
        desc_arch.name = "arch_type";
        desc_arch.archetype_config.chunk_size = 64;
        expect(rt.register_block_type(std::move(desc_arch)).has_value()).toBeTruthy();

        BlockTypeDescriptor desc_aos;
        desc_aos.type_id = 2;
        desc_aos.size = sizeof(float);
        desc_aos.alignment = alignof(float);
        desc_aos.name = "aos_type";
        expect(rt.register_block_type(std::move(desc_aos)).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(1).has_value()).toBeTruthy();
        expect(rt.create_block(2).has_value()).toBeTruthy();

        rt.convert_to_archetype(1);

        expect(rt.blocks().count() == 1).toBeTruthy();
        expect(rt.archetype_block_count(1) == 2).toBeTruthy();
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.get_layout(2) == LayoutStrategy::AoS).toBeTruthy();
    }

    struct Vec2 { int x; int y; };

    void test_archetype_field_access() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(Vec2);
        desc.alignment = alignof(Vec2);
        desc.name = "vec2";
        desc.layout = LayoutStrategy::AoS;
        desc.archetype_config.chunk_size = 64;
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();
        Identity id_val = id.value();

        expect(rt.submit_command(Command::set_field(id_val, 0, 42)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id_val, 4, 99)).has_value()).toBeTruthy();
        rt.tick();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();

        bool read_ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* px = view.field<int>(id_val, "x");
            auto* py = view.field<int>(id_val, "y");
            if (px && py) {
                read_ok = (*px == 42 && *py == 99);
            }
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(read_ok).toBeTruthy();
    }

    void test_archetype_round_trip_conversion() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(Vec2);
        desc.alignment = alignof(Vec2);
        desc.name = "vec2";
        desc.archetype_config.chunk_size = 4;
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        auto id1 = rt.create_block(1).value();
        auto id2 = rt.create_block(1).value();
        expect(rt.submit_command(Command::set_field(id1, 0, 10)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id1, 4, 20)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id2, 0, 30)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id2, 4, 40)).has_value()).toBeTruthy();
        rt.tick();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.archetype_block_count(1) == 2).toBeTruthy();

        rt.convert_from_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
        expect(rt.has_block(id1)).toBeTruthy();
        expect(rt.has_block(id2)).toBeTruthy();
    }

    void test_archetype_no_side_effects() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "aos";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();
        rt.convert_to_archetype(2);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
    }

    void test_archetype_empty_type() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.archetype_block_count(1) == 0).toBeTruthy();
        expect(rt.archetype_chunk_count(1) == 0).toBeTruthy();
    }

    void test_archetype_from_aos() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        rt.convert_from_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoS).toBeTruthy();
    }

    void test_archetype_idempotent_convert() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "test";
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        expect(rt.create_block(1).has_value()).toBeTruthy();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.archetype_block_count(1) == 1).toBeTruthy();

        rt.convert_to_archetype(1);
        expect(rt.get_layout(1) == LayoutStrategy::Archetype).toBeTruthy();
        expect(rt.archetype_block_count(1) == 1).toBeTruthy();
    }
}
}

inline void run_archetype_tests() {
    using namespace gameak::runtime;

describe("Archetype Storage", {
    it("Archetype is opt-in per type",                          { archetype_helpers::test_archetype_opt_in(); });
    it("dense field iteration",                                  { archetype_helpers::test_archetype_dense_iteration(); });
    it("O(1) identity lookup via sparse set",                    { archetype_helpers::test_archetype_identity_lookup(); });
    it("identity preserved through conversion",                  { archetype_helpers::test_archetype_conversion_preserves_identity(); });
    it("chunk overflow creates new chunk",                       { archetype_helpers::test_archetype_chunk_overflow(); });
    it("coexists with AoS layout",                               { archetype_helpers::test_archetype_coexists_with_other_layouts(); });
    it("field<T> access works across archetype storage",        { archetype_helpers::test_archetype_field_access(); });
    it("round-trip conversion Archetype → SoA → AoSoA → AoS",   { archetype_helpers::test_archetype_round_trip_conversion(); });
    it("no side effects on non-archetype types",                 { archetype_helpers::test_archetype_no_side_effects(); });
    it("empty type conversion",                                  { archetype_helpers::test_archetype_empty_type(); });
    it("convert from AoS on non-archetype type is no-op",        { archetype_helpers::test_archetype_from_aos(); });
    it("idempotent convert",                                     { archetype_helpers::test_archetype_idempotent_convert(); });
});
}
