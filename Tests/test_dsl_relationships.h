#pragma once

inline void run_dsl_relationship_tests() {
    using namespace gameak::core;
    using namespace gameak::runtime;
    using DefaultRuntime = Runtime<FifoScheduler>;
describe("DSL - Relationships", {
    it("relate_conversational", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto parent = rt.create_block(1);
        auto child  = rt.create_block(1);
        expect(parent.has_value() && child.has_value()).toBeTruthy();

        auto result = rt.relate(parent.value()).to(child.value());
        expect(result.has_value()).toBeTruthy();
    });

    it("unrelate_conversational", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto p = rt.create_block(1);
        auto c = rt.create_block(1);
        expect(p.has_value() && c.has_value()).toBeTruthy();

        expect(rt.relate(p.value(), c.value()).has_value()).toBeTruthy();
        auto result = rt.unrelate(p.value()).from(c.value());
        expect(result.has_value()).toBeTruthy();
    });

    it("relate_invalid_parent", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto child = rt.create_block(1);
        expect(child.has_value()).toBeTruthy();

        Identity invalid;
        auto result = rt.relate(invalid).to(child.value());
        expect(result.has_value()).toBeFalsy();
        expect(result.error().code() == ErrorCode::InvalidIdentity).toBeTruthy();
    });

    it("relate_invalid_child", {
        DefaultRuntime rt;
        BlockTypeDescriptor desc;
        desc.type_id = 1; desc.size = sizeof(int); desc.alignment = alignof(int); desc.name = "test";
        expect(rt.register_block_type(desc).has_value()).toBeTruthy();

        auto parent = rt.create_block(1);
        expect(parent.has_value()).toBeTruthy();

        Identity invalid;
        auto result = rt.relate(parent.value()).to(invalid);
        expect(result.has_value()).toBeFalsy();
        expect(result.error().code() == ErrorCode::InvalidIdentity).toBeTruthy();
    });
});
}
