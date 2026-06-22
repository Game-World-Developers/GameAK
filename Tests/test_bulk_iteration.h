#pragma once

namespace {
namespace bulk_helpers {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    struct BulkComponent {
        int hp;
        int mp;
    };

    void test_bulk_archetype_field_span() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(BulkComponent);
        desc.alignment = alignof(BulkComponent);
        desc.name = "player";
        desc.archetype_config.chunk_size = 64;
        desc.fields.push_back({"hp", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"mp", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        constexpr int N = 60;
        Identity ids[N];
        for (int i = 0; i < N; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            ids[i] = id.value();
        }

        for (int i = 0; i < N; ++i) {
            expect(rt.submit_command(Command::set_field(ids[i], 0, i)).has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(ids[i], 4, i * 2)).has_value()).toBeTruthy();
        }
        rt.tick();

        rt.convert_to_archetype(1);
        expect(rt.archetype_chunk_count(1) == 1).toBeTruthy();

        bool ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto hp_span = view.field_span<int>(1, "hp");
            auto mp_span = view.field_span<int>(1, "mp");
            if (hp_span.size() != N || mp_span.size() != N) return {};
            for (int i = 0; i < N; ++i) {
                if (hp_span[i] != i || mp_span[i] != i * 2) return {};
            }
            ok = true;
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok).toBeTruthy();
    }

    void test_bulk_multi_field_single_chunk() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(BulkComponent);
        desc.alignment = alignof(BulkComponent);
        desc.name = "player";
        desc.archetype_config.chunk_size = 64;
        desc.fields.push_back({"hp", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"mp", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        constexpr int N = 10;
        for (int i = 0; i < N; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(id.value(), 0, i * 3)).has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(id.value(), 4, i * 7)).has_value()).toBeTruthy();
        }
        rt.tick();

        rt.convert_to_archetype(1);
        expect(rt.archetype_chunk_count(1) == 1).toBeTruthy();

        bool ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto hp_span = view.field_span<int>(1, "hp");
            auto mp_span = view.field_span<int>(1, "mp");
            if (hp_span.size() != N || mp_span.size() != N) return {};
            for (int i = 0; i < N; ++i) {
                if (hp_span[i] != i * 3 || mp_span[i] != i * 7) return {};
            }
            ok = true;
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok).toBeTruthy();
    }

    void test_bulk_soa_field_span() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(BulkComponent);
        desc.alignment = alignof(BulkComponent);
        desc.name = "enemy";
        desc.fields.push_back({"hp", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"mp", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        constexpr int N = 50;
        for (int i = 0; i < N; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(id.value(), 0, i * 10)).has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(id.value(), 4, i * 20)).has_value()).toBeTruthy();
        }
        rt.tick();

        rt.convert_to_soa(1);

        bool ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto hp_span = view.field_span<int>(1, "hp");
            auto mp_span = view.field_span<int>(1, "mp");
            if (hp_span.size() != N || mp_span.size() != N) return {};
            for (int i = 0; i < N; ++i) {
                if (hp_span[i] != i * 10 || mp_span[i] != i * 20) return {};
            }
            ok = true;
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok).toBeTruthy();
    }

    void test_bulk_soa_field_span_multi() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(BulkComponent);
        desc.alignment = alignof(BulkComponent);
        desc.name = "item";
        desc.fields.push_back({"val", 0, sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        constexpr int N = 20;
        for (int i = 0; i < N; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(id.value(), 0, i * 5)).has_value()).toBeTruthy();
        }
        rt.tick();

        rt.convert_to_soa(1);

        bool ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto span = view.field_span<int>(1, "val");
            if (span.size() != N) return {};
            for (int i = 0; i < N; ++i) {
                if (span[i] != i * 5) return {};
            }
            ok = true;
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok).toBeTruthy();
    }

    void test_bulk_empty_type() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "empty";
        desc.fields.push_back({"val", 0, sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        bool ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto span = view.field_span<int>(1, "val");
            ok = span.empty();
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok).toBeTruthy();
    }

    void test_bulk_unregistered_type() {
        DefaultRuntime rt;
        bool ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto span = view.field_span<int>(999, "nonexistent");
            ok = span.empty();
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok).toBeTruthy();
    }

    void test_bulk_member_pointer() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(BulkComponent);
        desc.alignment = alignof(BulkComponent);
        desc.name = "test";
        desc.fields.push_back({"hp", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"mp", sizeof(int), sizeof(int), alignof(int)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        constexpr int N = 10;
        for (int i = 0; i < N; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(id.value(), 0, i * 3)).has_value()).toBeTruthy();
        }
        rt.tick();

        rt.convert_to_archetype(1);

        bool ok = false;
        auto controller = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto span = view.field_span(1u, &BulkComponent::hp);
            if (span.size() != N) return {};
            for (int i = 0; i < N; ++i) {
                if (span[i] != i * 3) return {};
            }
            ok = true;
            return {};
        };
        expect(rt.register_controller(std::move(controller)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok).toBeTruthy();
    }

    struct ComplexType {
        float x;
        float y;
    };

    void test_bulk_layout_independent() {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(ComplexType);
        desc.alignment = alignof(ComplexType);
        desc.name = "pos";
        desc.archetype_config.chunk_size = 64;
        desc.fields.push_back({"x", 0, sizeof(float), alignof(float)});
        desc.fields.push_back({"y", sizeof(float), sizeof(float), alignof(float)});
        expect(rt.register_block_type(std::move(desc)).has_value()).toBeTruthy();

        constexpr int N = 30;
        for (int i = 0; i < N; ++i) {
            auto id = rt.create_block(1);
            expect(id.has_value()).toBeTruthy();
            float xv = static_cast<float>(i);
            float yv = static_cast<float>(i * 2);
            expect(rt.submit_command(Command::set_field(id.value(), 0, xv)).has_value()).toBeTruthy();
            expect(rt.submit_command(Command::set_field(id.value(), 4, yv)).has_value()).toBeTruthy();
        }
        rt.tick();

        auto verify = [&](StateView& view, bool& ok) -> Result<void> {
            auto xs = view.field_span<float>(1, "x");
            auto ys = view.field_span<float>(1, "y");
            if (xs.size() != N || ys.size() != N) {
                ok = false;
                return {};
            }
            for (int i = 0; i < N; ++i) {
                if (xs[i] != static_cast<float>(i) ||
                    ys[i] != static_cast<float>(i * 2)) {
                    ok = false;
                    return {};
                }
            }
            ok = true;
            return {};
        };

        rt.convert_to_soa(1);
        bool ok_soa = false;
        auto controller_soa = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return verify(view, ok_soa);
        };
        expect(rt.register_controller(std::move(controller_soa)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok_soa).toBeTruthy();

        rt.convert_to_archetype(1);
        bool ok_arch = false;
        auto controller_arch = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            return verify(view, ok_arch);
        };
        expect(rt.register_controller(std::move(controller_arch)).has_value()).toBeTruthy();
        rt.tick();
        expect(ok_arch).toBeTruthy();
    }
}
}

inline void run_bulk_iteration_tests() {
    using namespace gameak::runtime;

describe("Bulk Iteration", {
    it("bulk field span on Archetype storage",        { bulk_helpers::test_bulk_archetype_field_span(); });
    it("bulk multi-field from single Archetype chunk", { bulk_helpers::test_bulk_multi_field_single_chunk(); });
    it("bulk field span on SoA storage",               { bulk_helpers::test_bulk_soa_field_span(); });
    it("bulk field span on SoA multi-entity",          { bulk_helpers::test_bulk_soa_field_span_multi(); });
    it("bulk read of empty type",                      { bulk_helpers::test_bulk_empty_type(); });
    it("bulk read of unregistered type",               { bulk_helpers::test_bulk_unregistered_type(); });
    it("bulk field span with member pointer",           { bulk_helpers::test_bulk_member_pointer(); });
    it("bulk works identically across layouts",        { bulk_helpers::test_bulk_layout_independent(); });
});
}
