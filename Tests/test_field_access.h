#pragma once

inline void run_field_access_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;

    struct TestVec { int x; int y; int z; };

describe("FieldAccess", {

    it("field<T> by name works in AoS layout", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(TestVec);
        desc.alignment = alignof(TestVec);
        desc.name = "TestVec";
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", 4, sizeof(int), alignof(int)});
        desc.fields.push_back({"z", 8, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        int val_x = 10;
        int val_y = 20;
        int val_z = 30;
        expect(rt.submit_command(Command::set_field(id.value(), 0, val_x)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 4, val_y)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 8, val_z)).has_value()).toBeTruthy();
        rt.tick();

        int read_x = 0;
        int read_y = 0;
        int read_z = 0;
        auto checker = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* fx = view.field<int>(id.value(), "x");
            auto* fy = view.field<int>(id.value(), "y");
            auto* fz = view.field<int>(id.value(), "z");
            expect(fx != nullptr).toBeTruthy();
            expect(fy != nullptr).toBeTruthy();
            expect(fz != nullptr).toBeTruthy();
            if (fx) read_x = *fx;
            if (fy) read_y = *fy;
            if (fz) read_z = *fz;
            return {};
        };
        expect(rt.register_controller(std::move(checker)).has_value()).toBeTruthy();
        rt.tick();
        expect(read_x == 10).toBeTruthy();
        expect(read_y == 20).toBeTruthy();
        expect(read_z == 30).toBeTruthy();
    });

    it("field<T> by name works after SoA conversion", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(TestVec);
        desc.alignment = alignof(TestVec);
        desc.name = "TestVec";
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", 4, sizeof(int), alignof(int)});
        desc.fields.push_back({"z", 8, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        int val_x = 100;
        int val_y = 200;
        int val_z = 300;
        expect(rt.submit_command(Command::set_field(id.value(), 0, val_x)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 4, val_y)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 8, val_z)).has_value()).toBeTruthy();
        rt.tick();

        rt.convert_to_soa(1);
        expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();

        int read_x = 0;
        int read_y = 0;
        int read_z = 0;
        auto checker = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* fx = view.field<int>(id.value(), "x");
            auto* fy = view.field<int>(id.value(), "y");
            auto* fz = view.field<int>(id.value(), "z");
            expect(fx != nullptr).toBeTruthy();
            expect(fy != nullptr).toBeTruthy();
            expect(fz != nullptr).toBeTruthy();
            if (fx) read_x = *fx;
            if (fy) read_y = *fy;
            if (fz) read_z = *fz;
            return {};
        };
        expect(rt.register_controller(std::move(checker)).has_value()).toBeTruthy();
        rt.tick();
        expect(read_x == 100).toBeTruthy();
        expect(read_y == 200).toBeTruthy();
        expect(read_z == 300).toBeTruthy();
        expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();
    });

    it("field<T> by name works after AoSoA conversion", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(TestVec);
        desc.alignment = alignof(TestVec);
        desc.name = "TestVec";
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", 4, sizeof(int), alignof(int)});
        desc.fields.push_back({"z", 8, sizeof(int), alignof(int)});
        desc.layout = LayoutStrategy::AoS;
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        int val_x = 42;
        int val_y = 84;
        int val_z = 168;
        expect(rt.submit_command(Command::set_field(id.value(), 0, val_x)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 4, val_y)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 8, val_z)).has_value()).toBeTruthy();
        rt.tick();

        rt.convert_to_aosoa(1);
        expect(rt.get_layout(1) == LayoutStrategy::AoSoA).toBeTruthy();

        int read_x = 0;
        int read_y = 0;
        int read_z = 0;
        auto checker = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* fx = view.field<int>(id.value(), "x");
            auto* fy = view.field<int>(id.value(), "y");
            auto* fz = view.field<int>(id.value(), "z");
            expect(fx != nullptr).toBeTruthy();
            expect(fy != nullptr).toBeTruthy();
            expect(fz != nullptr).toBeTruthy();
            if (fx) read_x = *fx;
            if (fy) read_y = *fy;
            if (fz) read_z = *fz;
            return {};
        };
        expect(rt.register_controller(std::move(checker)).has_value()).toBeTruthy();
        rt.tick();
        expect(read_x == 42).toBeTruthy();
        expect(read_y == 84).toBeTruthy();
        expect(read_z == 168).toBeTruthy();
    });

    it("field<T> returns nullptr for invalid identity", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "Test";
        desc.fields.push_back({"val", 0, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        Identity invalid;
        auto checker = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* f = view.field<int>(invalid, "val");
            expect(f == nullptr).toBeTruthy();
            return {};
        };
        expect(rt.register_controller(std::move(checker)).has_value()).toBeTruthy();
        rt.tick();
    });

    it("field<T> returns nullptr for unknown field name", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(int);
        desc.alignment = alignof(int);
        desc.name = "Test";
        desc.fields.push_back({"val", 0, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        auto checker = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* f = view.field<int>(id.value(), "nonexistent");
            expect(f == nullptr).toBeTruthy();
            return {};
        };
        expect(rt.register_controller(std::move(checker)).has_value()).toBeTruthy();
        rt.tick();
    });

    it("field<T> by member pointer works across layouts", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(TestVec);
        desc.alignment = alignof(TestVec);
        desc.name = "TestVec";
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", 4, sizeof(int), alignof(int)});
        desc.fields.push_back({"z", 8, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        int val_x = 77;
        int val_y = 88;
        int val_z = 99;
        expect(rt.submit_command(Command::set_field(id.value(), 0, val_x)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 4, val_y)).has_value()).toBeTruthy();
        expect(rt.submit_command(Command::set_field(id.value(), 8, val_z)).has_value()).toBeTruthy();
        rt.tick();

        int read_x = 0;
        int read_y = 0;
        int read_z = 0;

        auto checker_aos = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            read_x = view.field(id.value(), &TestVec::x);
            read_y = view.field(id.value(), &TestVec::y);
            read_z = view.field(id.value(), &TestVec::z);
            return {};
        };
        expect(rt.register_controller(std::move(checker_aos)).has_value()).toBeTruthy();
        rt.tick();
        expect(read_x == 77).toBeTruthy();
        expect(read_y == 88).toBeTruthy();
        expect(read_z == 99).toBeTruthy();

        rt.convert_to_soa(1);
        int read_x2 = 0;
        int read_y2 = 0;
        int read_z2 = 0;
        auto checker_soa = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            read_x2 = view.field(id.value(), &TestVec::x);
            read_y2 = view.field(id.value(), &TestVec::y);
            read_z2 = view.field(id.value(), &TestVec::z);
            return {};
        };
        expect(rt.register_controller(std::move(checker_soa)).has_value()).toBeTruthy();
        rt.tick();
        expect(read_x2 == 77).toBeTruthy();
        expect(read_y2 == 88).toBeTruthy();
        expect(read_z2 == 99).toBeTruthy();
        expect(rt.get_layout(1) == LayoutStrategy::SoA).toBeTruthy();
    });

    it("field<T> survives round-trip AoS to SoA to AoS", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1;
        desc.size = sizeof(TestVec);
        desc.alignment = alignof(TestVec);
        desc.name = "TestVec";
        desc.fields.push_back({"x", 0, sizeof(int), alignof(int)});
        desc.fields.push_back({"y", 4, sizeof(int), alignof(int)});
        desc.fields.push_back({"z", 8, sizeof(int), alignof(int)});
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto id = rt.create_block(1);
        expect(id.has_value()).toBeTruthy();

        int val_x = 555;
        expect(rt.submit_command(Command::set_field(id.value(), 0, val_x)).has_value()).toBeTruthy();
        rt.tick();

        int v1 = 0;
        auto r1 = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* f = view.field<int>(id.value(), "x");
            if (f) v1 = *f;
            return {};
        };
        expect(rt.register_controller(std::move(r1)).has_value()).toBeTruthy();
        rt.tick();
        expect(v1 == 555).toBeTruthy();

        rt.convert_to_soa(1);
        int v2 = 0;
        auto r2 = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* f = view.field<int>(id.value(), "x");
            if (f) v2 = *f;
            return {};
        };
        expect(rt.register_controller(std::move(r2)).has_value()).toBeTruthy();
        rt.tick();
        expect(v2 == 555).toBeTruthy();

        rt.convert_from_soa(1);
        int v3 = 0;
        auto r3 = [&](StateView& view, CommandProducer&, EphemeralProducer&) -> Result<void> {
            auto* f = view.field<int>(id.value(), "x");
            if (f) v3 = *f;
            return {};
        };
        expect(rt.register_controller(std::move(r3)).has_value()).toBeTruthy();
        rt.tick();
        expect(v3 == 555).toBeTruthy();
    });
});
}
